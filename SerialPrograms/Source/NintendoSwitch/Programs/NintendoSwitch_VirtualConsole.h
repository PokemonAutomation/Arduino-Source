/*  Virtual Game Console
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_VirtualConsole_H
#define PokemonAutomation_NintendoSwitch_VirtualConsole_H

#include "CommonFramework/Panels/PanelInstance.h"
#include "CommonFramework/Panels/UI/PanelWidget.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SwitchSystemOption.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SwitchSystemSession.h"

namespace PokemonAutomation{
namespace NintendoSwitch{

class SwitchSystemWidget;



class VirtualConsole_Descriptor : public PanelDescriptor{
public:
    VirtualConsole_Descriptor();
    const ControllerRequirements& requirements() const{ return m_requirements; }

private:
    const ControllerRequirements m_requirements;
};



class VirtualConsole : public PanelInstance{
public:
    VirtualConsole(const VirtualConsole_Descriptor& descriptor);
    virtual QWidget* make_widget(QWidget& parent, PanelHolder& holder) override;

public:
    //  Serialization
    virtual void from_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

private:
    friend class VirtualConsole_Widget;

    SwitchSystemOption m_switch;
};



class VirtualConsole_Widget : public PanelWidget{
public:
    static VirtualConsole_Widget* make(
        QWidget& parent,
        VirtualConsole& instance,
        PanelHolder& holder
    );

private:
    ~VirtualConsole_Widget();
    VirtualConsole_Widget(
        QWidget& parent,
        VirtualConsole& instance,
        PanelHolder& holder
    );
    void construct();

private:
    SwitchSystemSession m_session;
    SwitchSystemWidget* m_switch;
};





}
}
#endif

