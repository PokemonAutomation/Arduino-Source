/*  Switch System (4 Switches)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomationn_NintendoSwitch_SwitchSystem4Widget_H
#define PokemonAutomationn_NintendoSwitch_SwitchSystem4Widget_H

#include <QWidget>
#include "NintendoSwitch/Framework/NintendoSwitch_MultiSwitchSystemOption.h"
#include "NintendoSwitch/Framework/NintendoSwitch_MultiSwitchSystemSession.h"

class QComboBox;

namespace PokemonAutomation{
namespace NintendoSwitch{

class SwitchSystemWidget;


class MultiSwitchSystemWidget final : public QWidget, private MultiSwitchSystemSession::Listener{
    Q_OBJECT

public:
    ~MultiSwitchSystemWidget();
    MultiSwitchSystemWidget(
        QWidget& parent,
        MultiSwitchSystemSession& session,
        uint64_t program_id
    );

    size_t switch_count() const{ return m_switches.size(); }
    SwitchSystemWidget& operator[](size_t index){ return *m_switches[index]; }

public:
    void update_ui(ProgramState state);

private:
    virtual void shutdown() override;
    virtual void startup(size_t switch_count) override;

    void redraw_videos(size_t count);

private:
    uint64_t m_program_id;
    MultiSwitchSystemSession& m_session;

    std::mutex m_lock;
    bool m_shutting_down = false;

    QComboBox* m_console_count_box;
    std::vector<SwitchSystemWidget*> m_switches;
    QWidget* m_videos;
//    std::map<size_t, SwitchSystem*> m_active_ports;
};


}
}
#endif
