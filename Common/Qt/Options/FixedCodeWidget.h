/*  Fixed Code Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Options_FixedCodeWidget_H
#define PokemonAutomation_Options_FixedCodeWidget_H

#include <QWidget>
#include "Common/Cpp/Options/FixedCodeOption.h"
#include "ConfigWidget.h"

class QLineEdit;

namespace PokemonAutomation{


class FixedCodeWidget : public QWidget, public ConfigWidget{
public:
    ~FixedCodeWidget();
    FixedCodeWidget(QWidget& parent, FixedCodeOption& value);

    virtual void update_value() override;
    virtual void on_config_value_changed(void* object) override;

private:
    std::string sanitized_code(const std::string& text) const;

private:
    FixedCodeOption& m_value;
    QLineEdit* m_box;
};




}
#endif
