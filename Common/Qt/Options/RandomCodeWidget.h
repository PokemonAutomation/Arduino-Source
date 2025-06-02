/*  Random Code Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Options_RandomCodeWidget_H
#define PokemonAutomation_Options_RandomCodeWidget_H

#include <QWidget>
#include "Common/Cpp/Options/RandomCodeOption.h"
#include "ConfigWidget.h"

class QLabel;
class QLineEdit;

namespace PokemonAutomation{



class RandomCodeWidget : public QWidget, public ConfigWidget{
public:
    ~RandomCodeWidget();
    RandomCodeWidget(QWidget& parent, RandomCodeOption& value);

    virtual void update_value() override;
    virtual void on_config_value_changed(void* object) override;

private:
    std::string sanitized_code(const std::string& text) const;
    std::string random_code_string() const;
    void update_labels();

private:
    RandomCodeOption& m_value;
    QLabel* m_label_code;
    QLabel* m_under_text;
    QLineEdit* m_box_random;
    QLineEdit* m_box_code;
};




}
#endif
