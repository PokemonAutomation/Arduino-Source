/*  Dictionary OCR
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_OCR_DictionaryOCR_H
#define PokemonAutomation_CommonTools_OCR_DictionaryOCR_H

#include <vector>
#include <set>
#include <map>
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "OCR_StringMatchResult.h"

namespace PokemonAutomation{
    class JsonObject;
namespace OCR{


class DictionaryOCR{
public:
    DictionaryOCR(
        const JsonObject& json,
        const std::set<std::string>* subset,
        double random_match_chance,
        bool first_only
    );
    DictionaryOCR(
        const std::string& json_path,
        const std::set<std::string>* subset,
        double random_match_chance,
        bool first_only
    );

    JsonObject to_json() const;
    void save_json(const std::string& json_path) const;

    StringMatchResult match_substring(const std::string& text, double log10p_spread = 0.50) const;


public:
    //  This function is thread-safe with itself, but not with any other
    //  function in this class.

    void add_candidate(std::string token, const std::u32string& candidate);


private:
    SpinLock m_lock;
    double m_random_match_chance;
    std::map<std::string, std::vector<std::string>> m_database;
    std::map<std::u32string, std::set<std::string>> m_candidate_to_token;
};




}
}
#endif
