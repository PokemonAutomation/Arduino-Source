/*  Multi-Video Test
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_SwitchViewer_H
#define PokemonAutomation_SwitchViewer_H

#include <QJsonObject>
#include "CommonFramework/Panels/RightPanel.h"
#include "CommonFramework/Windows/MainWindow.h"
#include "NintendoSwitch/Framework/MultiSwitchSystem.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class SwitchViewer : public RightPanel{
public:
    SwitchViewer()
        : RightPanel(
            QColor(),
            "Switch Viewer",
            "SerialPrograms/SwitchViewer.md",
            "View status information from one or more running programs."
        )
        , m_switches(
            PABotBaseLevel::NOT_PABOTBASE, FeedbackType::NONE,
            1, 4, 1
        )
    {}
    SwitchViewer(const QJsonValue& json)
        : SwitchViewer()
    {
        m_switches.load_json(json.toObject().value(m_name));
    }
    virtual QJsonValue to_json() const override{
        return m_switches.to_json();
    }

    virtual QWidget* make_ui(MainWindow& window) override;

private:
    friend class SwitchViewerUI;
    MultiSwitchSystemFactory m_switches;
};


class SwitchViewerUI final: public RightPanelUI{
    friend class SwitchViewer;

private:
    SwitchViewerUI(SwitchViewer& factory, MainWindow& window)
        : RightPanelUI(factory)
        , m_window(window)
    {}
    virtual void make_body(QWidget& parent, QVBoxLayout& layout) override{
        SwitchViewer& factory = static_cast<SwitchViewer&>(m_factory);
        m_switches = (MultiSwitchSystem*)factory.m_switches.make_ui(parent, m_window.output_window());
        layout.addWidget(m_switches);
    }

private:
    MainWindow& m_window;
    MultiSwitchSystem* m_switches;
};



QWidget* SwitchViewer::make_ui(MainWindow& window){
    SwitchViewerUI* widget = new SwitchViewerUI(*this, window);
    widget->construct();
    return widget;
}


}
}
#endif
