/*  Virtual Game Console
 *
 *  From: https://github.com/PokemonAutomation/
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


// Descriptor for the program "Virtual Console".
// It defines basic info such as title name and color of the program on the program list panel.
// It inherits abstract base class PanelDescriptor but is still an abstract class as it does
// not define `make_panel()`, which functionality is simply to instantiate the PanelInstance, the
// program panel.
// Call CommonFramework/Panels/PanelTools.h:make_panel<VirtualConsole_Descriptor, VirtualConsole>()
// to create a wrapper class that implements `make_panel()` to instantiate the descriptor.
class VirtualConsole_Descriptor : public PanelDescriptor{
public:
    VirtualConsole_Descriptor();
};


// The program panel of Virtual Console.
// It calls make_widget() to create a VirtualConsole_Widget that holds the UI wideget.
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
    // switch control options like what micro-controller 
    // and what video source to use
    SwitchSystemOption m_switch_control_option;
};


// The UI of the prgoram Virtual Console
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
    SwitchSystemWidget* m_switch_widget;
};





}
}
#endif

