/*  Virtual Game Console
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_VirtualConsole_H
#define PokemonAutomation_NintendoSwitch_VirtualConsole_H

#include "CommonFramework/Panels/Panel.h"
#include "CommonFramework/Panels/PanelWidget.h"
#include "CommonFramework/Windows/MainWindow.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SwitchSystem.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class VirtualConsole_Descriptor : public PanelDescriptor{
public:
    VirtualConsole_Descriptor();
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

    SwitchSystemFactory m_switch;
};



class VirtualConsole_Widget : public PanelWidget{
public:
    static VirtualConsole_Widget* make(
        QWidget& parent,
        VirtualConsole& instance,
        PanelHolder& holder
    );

private:
    VirtualConsole_Widget(
        QWidget& parent,
        VirtualConsole& instance,
        PanelHolder& holder
    );
    void construct();

private:
    SwitchSystemWidget* m_switch;
};





}
}
#endif

