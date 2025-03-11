/*  Test Utils
 *
 *  From: https://github.com/PokemonAutomation/
 *  
 *  
 */

#include "TestUtils.h"

#include <iostream>
#include <sstream>
#include <fstream>
using std::cout;
using std::cerr;
using std::endl;

namespace PokemonAutomation{


std::vector<std::string> parse_words(const std::string& filename_base){
    std::vector<std::string> words;
    std::istringstream is(filename_base);
    std::string keyword;
    while (getline(is, keyword, '_')){
        words.push_back(keyword);
    }

    return words;
}

bool parse_size_t(const std::string& word, size_t& number){
    std::istringstream iss(word);
    iss >> number;
    return iss.eof() && !iss.fail();
}

bool parse_int(const std::string& word, int& number){
    std::istringstream iss(word);
    iss >> number;
    return iss.eof() && !iss.fail();
}

bool parse_float(const std::string& word, float& number){
    std::istringstream iss(word);
    iss >> number;
    return iss.eof() && !iss.fail();
}

bool parse_bool(const std::string& word, bool& value){
    if (word == "True"){
        value = true;
        return true;
    }else if (word == "False"){
        value = false;
        return true;
    }
    return false;
}

bool load_sprite_count(const std::string& txt_path, std::map<std::string, int>& sprites){
    std::ifstream fin(txt_path.c_str());
    std::string line;
    while(std::getline(fin, line)){
        std::istringstream is(line);
        std::string word;
        is >> word;
        if (is.fail()){
            continue; // empty line, skip
        }

        int count = 0;
        is >> count;
        if (is.fail()){
            count = 1;
        }else{
            if (count <= 0){
                cerr << "Error: in file " << txt_path << " sprite count must be larger than zero, but got " << count << endl;
                return false;
            }
        }
        sprites.emplace(std::move(word), count);
    }

    return true;
}

bool load_slug_list(const std::string& filepath, std::vector<std::string>& sprites){
    std::ifstream fin(filepath.c_str());
    std::string line;
    if (!fin){
        return false;
    }

    while(std::getline(fin, line)){
        std::istringstream is(line);
        std::string word;
        is >> word;
        if (is.fail()){
            continue; // empty line, skip
        }

        sprites.emplace_back(std::move(word));
    }

    return true;
}


}
