/*  Multi-Video Test
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_SwitchViewer_H
#define PokemonAutomation_NintendoSwitch_SwitchViewer_H

#include "CommonFramework/Panels/PanelSession.h"
#include "CommonFramework/Panels/UI/PanelWidget.h"
#include "NintendoSwitch/Framework/NintendoSwitch_MultiSwitchSystemOption.h"
#include "NintendoSwitch/Framework/NintendoSwitch_MultiSwitchSystemSession.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class SwitchViewer_Descriptor : public PanelDescriptor{
public:
    SwitchViewer_Descriptor();
};



class SwitchViewer : public UiState<SwitchViewer, PanelSession>{
public:
    SwitchViewer(const SwitchViewer_Descriptor& descriptor);

public:
    //  Serialization
    virtual JsonValue to_json() const override;
    virtual void load_json(const JsonValue& json) override;

private:
    friend class SwitchViewer_Widget;

    MultiSwitchSystemOption m_switches;
};



class SwitchViewer_Widget final : public PanelWidget{
public:
    using ParentState = SwitchViewer;

public:
    ~SwitchViewer_Widget();
    SwitchViewer_Widget(QWidget& parent, SwitchViewer& session);

    virtual QWidget& widget() override{
        return *this;
    }

private:
    MultiSwitchSystemSession m_session;
    MultiSwitchSystemWidget* m_switches;
};





}
}
#endif
