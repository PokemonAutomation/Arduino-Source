/*  Dictionary OCR
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Qt/StringToolsQt.h"
#include "CommonFramework/Logging/Logger.h"
#include "OCR_StringNormalization.h"
#include "OCR_TextMatcher.h"
#include "OCR_DictionaryOCR.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace OCR{



DictionaryOCR::DictionaryOCR(
    const JsonObject& json,
    const std::set<std::string>* subset,
    double random_match_chance,
    bool first_only
)
    : m_random_match_chance(random_match_chance)
{
    for (const auto& item0 : json){
        const std::string& token = item0.first;
        if (subset != nullptr && subset->find(token) == subset->end()){
            continue;
        }
        std::vector<std::string>& candidates = m_database[token];
        for (const auto& item1 : item0.second.to_array_throw()){
            const std::string& candidate = item1.to_string_throw();
            std::u32string normalized = normalize_utf32(candidate);
            std::set<std::string>& set = m_candidate_to_token[normalized];
            if (!set.empty()){
                global_logger_tagged().log("DictionaryOCR - Duplicate Candidate: " + token + " (" + to_utf8(normalized) + ")");
//                cout << "Duplicate Candidate: " << candidate << endl;
            }
            set.insert(token);
            candidates.emplace_back(candidate);
            if (first_only){
                break;
            }
        }
    }
    global_logger_tagged().log(
        "DictionaryOCR - Tokens: " + std::to_string(m_database.size()) +
        ", Match Candidates: " + std::to_string(m_candidate_to_token.size())
    );
//    cout << "Tokens: " << m_database.size() << ", Match Candidates: " << m_candidate_to_token.size() << endl;
}
DictionaryOCR::DictionaryOCR(
    const std::string& json_path,
    const std::set<std::string>* subset,
    double random_match_chance,
    bool first_only
)
    : DictionaryOCR(
        load_json_file(json_path).to_object_throw(json_path),
        subset,
        random_match_chance,
        first_only
    )
{}

JsonObject DictionaryOCR::to_json() const{
    JsonObject obj;
    for (const auto& item : m_database){
        JsonArray list;
        for (const std::string& candidate : item.second){
            list.push_back(candidate);
        }
        obj[item.first] = std::move(list);
    }
    return obj;
}
void DictionaryOCR::save_json(const std::string& json_path) const{
    to_json().dump(json_path);
}



StringMatchResult DictionaryOCR::match_substring(
    const std::string& text,
    double log10p_spread
) const{
    return OCR::match_substring(
        m_candidate_to_token, m_random_match_chance,
        text, log10p_spread
    );
}
void DictionaryOCR::add_candidate(std::string token, const std::u32string& candidate){
    if (candidate.size() < 2){
        return;
    }

    WriteSpinLock lg(m_lock, "DictionaryOCR::add_candidate()");

    auto iter = m_candidate_to_token.find(candidate);
    if (iter == m_candidate_to_token.end()){
        //  New candidate. Add it to both maps.
        m_database[token].emplace_back(to_utf8(candidate));
        m_candidate_to_token[candidate].insert(std::move(token));
        return;
    }

    //  Candidate already exists in table.
    std::set<std::string>& tokens = iter->second;
    if (tokens.find(token) == tokens.end()){
        //  Add to database only if it isn't already there.
        m_database[token].emplace_back(to_utf8(candidate));
    }

    tokens.insert(std::move(token));
}






}
}
