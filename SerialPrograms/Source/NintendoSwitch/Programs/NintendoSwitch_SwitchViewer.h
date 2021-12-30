/*  Multi-Video Test
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_SwitchViewer_H
#define PokemonAutomation_NintendoSwitch_SwitchViewer_H

#include <QJsonObject>
#include "CommonFramework/Panels/Panel.h"
#include "CommonFramework/Panels/PanelWidget.h"
#include "NintendoSwitch/Framework/MultiSwitchSystem.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class SwitchViewer_Descriptor : public PanelDescriptor{
public:
    SwitchViewer_Descriptor();
};



class SwitchViewer : public PanelInstance{
public:
    SwitchViewer(const SwitchViewer_Descriptor& descriptor);
    virtual QWidget* make_widget(QWidget& parent, PanelListener& listener) override;

public:
    //  Serialization
    virtual void from_json(const QJsonValue& json) override;
    virtual QJsonValue to_json() const override;

private:
    friend class SwitchViewer_Widget;

    MultiSwitchSystemFactory m_switches;
};



class SwitchViewer_Widget : public PanelWidget{
public:
    static SwitchViewer_Widget* make(
        QWidget& parent,
        SwitchViewer& instance,
        PanelListener& listener
    );

private:
    SwitchViewer_Widget(
        QWidget& parent,
        SwitchViewer& instance,
        PanelListener& listener
    );
    void construct();

private:
    MultiSwitchSystem* m_switches;
};





}
}
#endif
