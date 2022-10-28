/*  Language OCR Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Options_LanguageOCRWidget_H
#define PokemonAutomation_Options_LanguageOCRWidget_H

#include <QWidget>
#include "Common/Qt/Options/ConfigWidget.h"
#include "CommonFramework/Options/LanguageOCROption.h"

class QLabel;
class QComboBox;

namespace PokemonAutomation{
namespace OCR{


class LanguageOCRWidget : public QWidget, public ConfigWidget{
public:
    LanguageOCRWidget(QWidget& parent, LanguageOCR& value);

    virtual void update_value() override;
    virtual void value_changed() override;

private:
    LanguageOCR& m_value;
    QComboBox* m_box;
    QLabel* m_status;
    std::vector<Language> m_index_to_enum;
//    std::map<Language, int> m_enum_to_index;
};



}
}
#endif
