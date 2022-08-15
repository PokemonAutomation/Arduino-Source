/*  Fixed Code Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Options_FixedCodeWidget_H
#define PokemonAutomation_Options_FixedCodeWidget_H

#include <QWidget>
#include "Common/Cpp/Options/FixedCodeOption.h"
#include "ConfigWidget.h"

class QLineEdit;

namespace PokemonAutomation{


class FixedCodeWidget : public QWidget, public ConfigWidget, private ConfigOption::Listener{
public:
    ~FixedCodeWidget();
    FixedCodeWidget(QWidget& parent, FixedCodeOption& value);

    virtual void update() override;
    virtual void value_changed() override;

private:
    std::string sanitized_code(const std::string& text) const;

private:
    FixedCodeOption& m_value;
    QLineEdit* m_box;
};




}
#endif
