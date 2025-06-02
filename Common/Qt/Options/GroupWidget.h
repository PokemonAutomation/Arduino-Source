/*  Group Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QWidget>
#include "ConfigWidget.h"
#include "Common/Cpp/Options/GroupOption.h"

#ifndef PokemonAutomation_Options_GroupWidget_H
#define PokemonAutomation_Options_GroupWidget_H

class QVBoxLayout;
class QGroupBox;
class QPushButton;

namespace PokemonAutomation{



class GroupWidget : public QWidget, public ConfigWidget{
public:
    ~GroupWidget();
    GroupWidget(QWidget& parent, GroupOption& value);

//    void set_options_enabled(bool enabled);

    virtual void update_value() override;
    virtual void on_config_value_changed(void* object) override;

private:
    virtual void mouseDoubleClickEvent(QMouseEvent* event) override;

protected:
    GroupOption& m_value;
    QGroupBox* m_group_box;
    QWidget* m_expand_text;
    QWidget* m_options_holder;
    std::vector<ConfigWidget*> m_options;
    QPushButton* m_restore_defaults_button;
    bool m_expanded = true;
    QVBoxLayout* m_options_layout;
};



}
#endif
