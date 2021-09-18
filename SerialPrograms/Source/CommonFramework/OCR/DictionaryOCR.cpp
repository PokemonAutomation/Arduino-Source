/*  Dictionary OCR
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonArray>
#include "Common/Cpp/Exception.h"
#include "Common/Qt/QtJsonTools.h"
#include "StringNormalization.h"
#include "TextMatcher.h"
#include "DictionaryOCR.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace OCR{



DictionaryOCR::DictionaryOCR(
    const QJsonObject& json,
    const std::set<std::string>* subset,
    double random_match_chance,
    bool first_only
)
    : m_random_match_chance(random_match_chance)
{
    for (auto it = json.begin(); it != json.end(); ++it){
        std::string token = it.key().toUtf8().data();
        if (subset != nullptr && subset->find(token) == subset->end()){
            continue;
        }
        std::vector<QString>& candidates = m_database[token];
        for (const auto& item : it.value().toArray()){
            QString candidate = item.toString();
            QString normalized = normalize(candidate);
            std::set<std::string>& set = m_candidate_to_token[normalized];
            if (!set.empty()){
                global_logger().log("[DictionaryOCR] Duplicate Candidate: " + it.key().toStdString());
//                cout << "Duplicate Candidate: " << it.key().toUtf8().data() << endl;
            }
            set.insert(token);
            candidates.emplace_back(std::move(candidate));
            if (first_only){
                break;
            }
        }
    }
    global_logger().log(
        "[DictionaryOCR] Tokens: " + std::to_string(m_database.size()) +
        ", Match Candidates: " + std::to_string(m_candidate_to_token.size())
    );
//    cout << "Tokens: " << m_database.size() << ", Match Candidates: " << m_candidate_to_token.size() << endl;
}
DictionaryOCR::DictionaryOCR(
    const QString& json_path,
    const std::set<std::string>* subset,
    double random_match_chance,
    bool first_only
)
    : DictionaryOCR(read_json_file(json_path).object(), subset, random_match_chance, first_only)
{}

QJsonObject DictionaryOCR::to_json() const{
    QJsonObject obj;
    for (const auto& item : m_database){
        QJsonArray list;
        for (const QString& candidate : item.second){
            list.append(candidate);
        }
        obj.insert(QString::fromStdString(item.first), list);
    }
    return obj;
}
void DictionaryOCR::save_json(const QString& json_path) const{
    write_json_file(json_path, QJsonDocument(to_json()));
}



MatchResult DictionaryOCR::match_substring(
    const QString& text,
    double min_alpha
) const{
    return OCR::match_substring(
        m_candidate_to_token,
        text,
        m_random_match_chance,
        min_alpha
    );
}
MatchResult DictionaryOCR::match_substring(
    const std::string& expected,
    const QString& text,
    double min_alpha
) const{
    MatchResult result = OCR::match_substring(
        m_candidate_to_token,
        text,
        m_random_match_chance,
        min_alpha
    );
    result.expected_token = expected;
    result.matched = result.matched && result.slugs.find(expected) != result.slugs.end();
    return result;
}
void DictionaryOCR::add_candidate(std::string token, const QString& candidate){
    if (candidate.isEmpty() || (candidate.size() == 1 && candidate[0] < 128)){
        return;
    }

    SpinLockGuard lg(m_lock, "DictionaryOCR::add_candidate()");

    auto iter = m_candidate_to_token.find(candidate);
    if (iter == m_candidate_to_token.end()){
        //  New candidate. Add it to both maps.
        m_database[token].emplace_back(candidate);
        m_candidate_to_token[candidate].insert(std::move(token));
        return;
    }

    //  Candidate already exists in table.
    std::set<std::string>& tokens = iter->second;
    if (tokens.find(token) == tokens.end()){
        //  Add to database only if it isn't already there.
        m_database[token].emplace_back(candidate);
    }

    tokens.insert(std::move(token));
}






}
}
