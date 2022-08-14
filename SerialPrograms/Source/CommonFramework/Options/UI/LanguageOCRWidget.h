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


class LanguageOCRWidget : public QWidget, public ConfigWidget, private ConfigOption::Listener{
public:
    LanguageOCRWidget(QWidget& parent, LanguageOCR& value);

    virtual void restore_defaults() override;
    virtual void update_ui() override;
    virtual void value_changed() override;

private:
    void update_status();

private:
    LanguageOCR& m_value;
    QComboBox* m_box;
    QLabel* m_status;
};



}
}
#endif
