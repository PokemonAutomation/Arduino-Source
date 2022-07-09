/*  Language OCR Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_OCR_LanguageOCR_H
#define PokemonAutomation_OCR_LanguageOCR_H

#include <map>
#include "CommonFramework/Language.h"
#include "CommonFramework/Options/ConfigOption.h"

namespace PokemonAutomation{
namespace OCR{


class LanguageOCR : public ConfigOption{
public:
    LanguageOCR(QString label, const LanguageSet& languages, bool required = true);

//    explicit operator bool() const{ return m_case_list[m_current].first != Language::None && m_case_list[m_current].second; }
//    operator size_t() const{ return m_current; }
    operator Language() const{ return m_case_list[m_current].first; }

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual QString check_validity() const override;
    virtual void restore_defaults() override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    friend class LanguageOCRWidget;

    QString m_label;

    std::vector<std::pair<Language, bool>> m_case_list;
    std::map<Language, size_t> m_case_map;
    size_t m_default;
    size_t m_current;
};




}
}
#endif
