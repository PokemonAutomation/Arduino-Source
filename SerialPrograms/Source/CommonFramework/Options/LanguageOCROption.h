/*  Language OCR Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Options_LanguageOCROption_H
#define PokemonAutomation_Options_LanguageOCROption_H

#include <atomic>
#include <map>
#include "Common/Cpp/Options/ConfigOption.h"
#include "CommonFramework/Language.h"

namespace PokemonAutomation{
namespace OCR{


class LanguageOCR : public ConfigOption{
public:
    LanguageOCR(std::string label, const LanguageSet& languages, bool required = true);

//    explicit operator bool() const{ return m_case_list[m_current].first != Language::None && m_case_list[m_current].second; }

    operator size_t() const{ return m_current; }
    operator Language() const{ return m_case_list[m_current.load(std::memory_order_relaxed)].first; }
    void set(Language language);

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual std::string check_validity() const override;
    virtual void restore_defaults() override;

    virtual ConfigWidget* make_QtWidget(QWidget& parent) override;

private:
    friend class LanguageOCRWidget;

    const std::string m_label;
    std::vector<std::pair<Language, bool>> m_case_list;
    std::map<Language, size_t> m_case_map;
    const size_t m_default;

    std::atomic<size_t> m_current;
};




}
}
#endif
