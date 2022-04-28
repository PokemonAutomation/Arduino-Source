/*  Dictionary OCR
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_OCR_DictionaryOCR_H
#define PokemonAutomation_OCR_DictionaryOCR_H

#include <vector>
#include <set>
#include <map>
#include "Common/Cpp/SpinLock.h"
#include "CommonFramework/Logging/LoggerQt.h"
#include "OCR_StringMatchResult.h"

class QJsonObject;

namespace PokemonAutomation{
namespace OCR{


class DictionaryOCR{
public:
    DictionaryOCR(
        const QJsonObject& json,
        const std::set<std::string>* subset,
        double random_match_chance,
        bool first_only
    );
    DictionaryOCR(
        const QString& json_path,
        const std::set<std::string>* subset,
        double random_match_chance,
        bool first_only
    );

    QJsonObject to_json() const;
    void save_json(const QString& json_path) const;

    StringMatchResult match_substring(const QString& text, double log10p_spread = 0.50) const;


public:
    //  This function is thread-safe with itself, but not with any other
    //  function in this class.

    void add_candidate(std::string token, const QString& candidate);


private:
    SpinLock m_lock;
    double m_random_match_chance;
    std::map<std::string, std::vector<QString>> m_database;
    std::map<QString, std::set<std::string>> m_candidate_to_token;
};




}
}
#endif
