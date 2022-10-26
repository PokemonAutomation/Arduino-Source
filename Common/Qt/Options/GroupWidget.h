/*  Group Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Qt/Options/BatchWidget.h"
#include "Common/Cpp/Options/GroupOption.h"

#ifndef PokemonAutomation_Options_GroupWidget_H
#define PokemonAutomation_Options_GroupWidget_H

class QVBoxLayout;
class QGroupBox;

namespace PokemonAutomation{



class GroupWidget : public QWidget, public ConfigWidget{
public:
    ~GroupWidget();
    GroupWidget(QWidget& parent, GroupOption& value);

    void set_options_enabled(bool enabled);

    virtual void update_value() override;
    virtual void update_visibility(bool program_is_running) override;
    virtual void value_changed() override;

private:
    virtual void mouseDoubleClickEvent(QMouseEvent* event) override;

protected:
    GroupOption& m_value;
    QGroupBox* m_group_box;
    QWidget* m_expand_text;
    QWidget* m_options_holder;
    std::vector<ConfigWidget*> m_options;
    bool m_expanded = true;
    QVBoxLayout* m_options_layout;
};



}
#endif
