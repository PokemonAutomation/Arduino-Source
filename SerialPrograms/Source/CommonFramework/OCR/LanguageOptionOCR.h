/*  Language OCR Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_LanguageOCR_H
#define PokemonAutomation_LanguageOCR_H

#include <set>
#include <QLabel>
#include <QComboBox>
#include "CommonFramework/Language.h"
#include "CommonFramework/Options/ConfigOption.h"

namespace PokemonAutomation{
namespace OCR{


class LanguageOCR : public ConfigOption{
public:
    LanguageOCR(QString label, const LanguageSet& languages, bool required = true);

    operator bool() const{ return  m_case_list[m_current].first != Language::None && m_case_list[m_current].second; }
    operator size_t() const{ return m_current; }
    operator Language() const{ return m_case_list[m_current].first; }

    virtual void load_json(const QJsonValue& json) override;
    virtual QJsonValue to_json() const override;

    virtual bool is_valid() const override;
    virtual void restore_defaults() override;

    virtual ConfigOptionUI* make_ui(QWidget& parent) override;

private:
    friend class LanguageOCRUI;

    QString m_label;

    std::vector<std::pair<Language, bool>> m_case_list;
    std::map<Language, size_t> m_case_map;
    size_t m_default;
    size_t m_current;
};



class LanguageOCRUI : public ConfigOptionUI, public QWidget{
public:
    LanguageOCRUI(QWidget& parent, LanguageOCR& value);
    virtual QWidget* widget() override{ return this; }
    virtual void restore_defaults() override;

private:
    void update_status();

private:
    LanguageOCR& m_value;
    QComboBox* m_box;
    QLabel* m_status;
    bool m_updating = false;
};




}
}
#endif
