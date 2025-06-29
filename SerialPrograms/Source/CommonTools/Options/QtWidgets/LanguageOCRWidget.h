/*  Language OCR Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_Options_LanguageOCRWidget_H
#define PokemonAutomation_CommonTools_Options_LanguageOCRWidget_H

#include <QWidget>
#include "Common/Qt/Options/ConfigWidget.h"
#include "CommonTools/Options/LanguageOCROption.h"

class QLabel;
class QComboBox;

namespace PokemonAutomation{
namespace OCR{


class LanguageOCRCellWidget : public QWidget, public ConfigWidget{
public:
    LanguageOCRCellWidget(QWidget& parent, LanguageOCRCell& value);

    virtual void update_value() override;
    virtual void on_config_value_changed(void* object) override;

private:
    LanguageOCRCell& m_value;
    QComboBox* m_box;
    QLabel* m_status;
    std::vector<Language> m_index_to_enum;
//    std::map<Language, int> m_enum_to_index;
};



class LanguageOCROptionWidget : public QWidget, public ConfigWidget{
public:
    ~LanguageOCROptionWidget();
    LanguageOCROptionWidget(QWidget& parent, LanguageOCROption& value);

    virtual void update_value() override;
    virtual void on_config_value_changed(void* object) override;

private:
    LanguageOCRCellWidget* m_cell;
};



}
}
#endif
