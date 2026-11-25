/*  Test Utils
 *
 *  From: https://github.com/PokemonAutomation/
 *  
 *  
 */

#include <iostream>

#include <sstream>
#include <fstream>
#include "Common/Cpp/Exceptions.h"
#include "TestUtils.h"

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

bool load_slug_list_with_2d_coordinates(const std::string& filepath, std::vector<SlugWith2DCoordinate>& slugs){
    std::ifstream fin(filepath.c_str());
    if (!fin){
        return false;
    }

    std::string line;
    size_t line_number = 0;
    while (std::getline(fin, line)){
        line_number++;
        std::istringstream is(line);
        std::vector<std::string> words;
        std::string word;
        while (is >> word){
            words.emplace_back(std::move(word));
        }
        if (words.empty()){
            continue; // empty line, skip
        }

        const std::string where = filepath + ", line " + std::to_string(line_number) + ": \"" + line + "\"";
        SlugWith2DCoordinate entry;
        entry.slug = words[0];
        switch (words.size()){
        case 1:
            entry.x = entry.y = -1.0;
            break;
        case 3:{
            //  `std::stod()` would accept "12abc", so check that the whole word is used.
            size_t x_length = 0;
            size_t y_length = 0;
            try{
                entry.x = std::stod(words[1], &x_length);
                entry.y = std::stod(words[2], &y_length);
            }catch (std::exception&){}
            if (x_length != words[1].size() || y_length != words[2].size()){
                throw ParseException("The point must be two numbers \"<x> <y>\" in " + where);
            }
            //  This also catches a point in pixels, e.g. "1205 478".
            //  `!(... >= ...)` is also true for NaN.
            if (!(entry.x >= 0 && entry.x <= 1 && entry.y >= 0 && entry.y <= 1)){
                throw ParseException("The point must be relative to the image size, from 0.0 to 1.0, in " + where);
            }
            break;
        }
        default:
            throw ParseException("Each line must be \"<slug>\" or \"<slug> <x> <y>\" in " + where);
        }
        slugs.emplace_back(std::move(entry));
    }

    return true;
}


}
