/*  Virtual Game Console
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_VirtualConsole_H
#define PokemonAutomation_VirtualConsole_H

#include <QJsonObject>
#include "Common/Qt/QtJsonTools.h"
#include "CommonFramework/Panels/RightPanel.h"
#include "CommonFramework/Windows/MainWindow.h"
#include "NintendoSwitch/Framework/SwitchSystem.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class VirtualConsoleUI;

class VirtualConsole : public RightPanel{
public:
    VirtualConsole()
        : RightPanel(
            QColor(),
            "Virtual Console",
            "SerialPrograms/VirtualConsole.md",
            "Play your Switch from your computer. Device logging is logged to the output window."
        )
        , m_switch(
            "Switch Settings", "Switch 0",
            PABotBaseLevel::NOT_PABOTBASE, FeedbackType::NONE
        )
    {}
    VirtualConsole(const QJsonValue& json)
        : VirtualConsole()
    {
        m_switch.load_json(json.toObject().value(m_name));
    }
    virtual QJsonValue to_json() const override{
        return m_switch.to_json();
    }

    virtual QWidget* make_ui(MainWindow& window) override;

private:
    friend class VirtualConsoleUI;
    SwitchSystemFactory m_switch;
};


class VirtualConsoleUI final : public RightPanelUI{
    friend class VirtualConsole;

private:
    VirtualConsoleUI(VirtualConsole& factory, MainWindow& window)
        : RightPanelUI(factory)
        , m_window(window)
    {}
    virtual void make_body(QWidget& parent, QVBoxLayout& layout) override{
        VirtualConsole& factory = static_cast<VirtualConsole&>(m_factory);
        m_switch = (SwitchSystem*)factory.m_switch.make_ui(parent, m_window.output_window());
        layout.addWidget(m_switch);

//        QLabel* controls = new QLabel("Controls", this);
//        layout.addWidget(controls);
    }

private:
    MainWindow& m_window;
    SwitchSystem* m_switch;
};



inline QWidget* VirtualConsole::make_ui(MainWindow& window){
    VirtualConsoleUI* widget = new VirtualConsoleUI(*this, window);
    widget->construct();
    return widget;
}


}
}
#endif

