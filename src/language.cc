#include "language.h"
#include "assert.h"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/string_generator.hpp>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string.h>
#include <regex>

namespace fasttext {

    Language::Language(const std::string& language) : lang_() {
        init(language);
    };

    void Language::init(const std::string& language) {
        assert(LONGEST_WORDS.find(language) != LONGEST_WORDS.end());
        lang_.MAXLEN = LONGEST_WORDS[language];
        lang_.PROFANITY = load(std::getenv("PROFANITY_PATH") + language + ".txt");
        lang_.STOPWORDS = load(std::getenv("STOPWORDS_PATH") + language + ".txt");
    }

    std::unordered_set<std::string> Language::load(const std::string& filename) {
        std::unordered_set<std::string> set;
        std::string line;
        std::ifstream ifs(filename);
        if(ifs.is_open()) {
            while(getline(ifs, line)) {
                set.insert(line);
            }
        } else {
            throw std::invalid_argument("Cannot load " + filename + "!");
        }
        return set;
    }

    void Language::addWord(const entry e) {
        dict_.insert(e.word);
        words.push_back(e);
    }

    bool Language::isWord(const std::string word) {
        for(int8_t i = 0; i < word.size(); i++) {
            if(std::isalpha(word[i])){return true;}
        }
        return false;
    }

    bool Language::isProfanity(const std::string word) {
        return lang_.PROFANITY.find(word) != lang_.PROFANITY.end();
    }

    bool Language::isStopword(const std::string word) {
        return lang_.STOPWORDS.find(word) != lang_.STOPWORDS.end();
    }

    bool Language::isDuplicate(std::string word) {
        std::string original = word;
        std::transform(word.begin(), word.end(), word.begin(), [](unsigned char c){return std::tolower(c);});
        // Check that splitting the words on full stops doesn't return already known tokens
        // VERY EXPENSIVE! Not implemented yet.
        // std::string delimiter = ".";
        // size_t pos = 0;
        // std::string token;
        // while ((pos = word.find(delimiter)) != std::string::npos) {
        //     token = word.substr(0, pos);
        //     word.erase(0, pos + delimiter.length());
        // }
        for (uint8_t i = 0; i < PUNCT.size(); i++) {
            word.erase(std::remove(word.begin(), word.end(), *PUNCT[i]), word.end());
        }
        if (STRICT_PUNCT.find(std::string(1, word.front())) != STRICT_PUNCT.end()) {
            word.erase(word.begin());
        }
        if (STRICT_PUNCT.find(std::string(1, word.back())) != STRICT_PUNCT.end()) {
            word.erase(word.end());
        }
        if (word == original) return false;
        if (dict_.find(word) != dict_.end()) return true;
        return false;
    }

    bool Language::isWeb(const std::string word) {
        if (std::regex_match(word, std::regex("^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?"))) {
            return true;
        } else {
            return false;
        }
    }

    bool Language::isUUID(const std::string word) {
        using namespace boost::uuids;
        try {
            auto result = string_generator()(word); 
            return result.version() != uuid::version_unknown;
        } catch(...) {
            return false;
        }
    }

}