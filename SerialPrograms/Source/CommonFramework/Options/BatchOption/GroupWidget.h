/*  Group Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "BatchWidget.h"
#include "GroupOption.h"

#ifndef PokemonAutomation_GroupWidget_H
#define PokemonAutomation_GroupWidget_H

class QVBoxLayout;
class QGroupBox;

namespace PokemonAutomation{



class GroupWidget : public QWidget, public ConfigWidget{
public:
    GroupWidget(QWidget& parent, GroupOption& value);

    void set_options_enabled(bool enabled);

    virtual void restore_defaults() override;
    virtual void update_visibility() override;


public:
    //  Callbacks
    virtual void on_set_enabled(bool enabled);

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
