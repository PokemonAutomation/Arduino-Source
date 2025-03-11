/*  Multi-Video Test
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_SwitchViewer_H
#define PokemonAutomation_NintendoSwitch_SwitchViewer_H

#include "CommonFramework/Panels/PanelInstance.h"
#include "CommonFramework/Panels/UI/PanelWidget.h"
#include "NintendoSwitch/Framework/NintendoSwitch_MultiSwitchSystemOption.h"
#include "NintendoSwitch/Framework/NintendoSwitch_MultiSwitchSystemSession.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class SwitchViewer_Descriptor : public PanelDescriptor{
public:
    SwitchViewer_Descriptor();
};



class SwitchViewer : public PanelInstance{
public:
    SwitchViewer(const SwitchViewer_Descriptor& descriptor);
    virtual QWidget* make_widget(QWidget& parent, PanelHolder& holder) override;

public:
    //  Serialization
    virtual void from_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

private:
    friend class SwitchViewer_Widget;

    MultiSwitchSystemOption m_switches;
};



class SwitchViewer_Widget : public PanelWidget{
public:
    static SwitchViewer_Widget* make(
        QWidget& parent,
        SwitchViewer& instance,
        PanelHolder& holder
    );

private:
    ~SwitchViewer_Widget();
    SwitchViewer_Widget(
        QWidget& parent,
        SwitchViewer& instance,
        PanelHolder& holder
    );
    void construct();

private:
    MultiSwitchSystemSession m_session;
    MultiSwitchSystemWidget* m_switches;
};





}
}
#endif
