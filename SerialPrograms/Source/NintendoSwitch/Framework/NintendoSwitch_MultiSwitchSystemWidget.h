/*  Switch System (4 Switches)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomationn_NintendoSwitch_SwitchSystem4Widget_H
#define PokemonAutomationn_NintendoSwitch_SwitchSystem4Widget_H

#include <QComboBox>
#include "NintendoSwitch_SwitchSetupWidget.h"
#include "NintendoSwitch_MultiSwitchSystemOption.h"
#include "NintendoSwitch_MultiSwitchSystemSession.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class MultiSwitchSystemWidget final : public SwitchSetupWidget, private MultiSwitchSystemSession::Listener{
    Q_OBJECT

public:
    ~MultiSwitchSystemWidget();
    MultiSwitchSystemWidget(
        QWidget& parent,
        MultiSwitchSystemOption& option,
        Logger& logger,
        uint64_t program_id
    );

    size_t switch_count() const{ return m_switches.size(); }
    SwitchSystemWidget& operator[](size_t index){ return *m_switches[index]; }

    virtual bool serial_ok() const override;
    virtual void wait_for_all_requests() override;
    virtual void stop_serial() override;
    virtual void reset_serial() override;

public:
    virtual void update_ui(ProgramState state) override;

private:
    virtual void shutdown() override;
    virtual void startup(size_t switch_count) override;

    void redraw_videos(size_t count);

private:
    uint64_t m_program_id;
    MultiSwitchSystemOption& m_option;
    Logger& m_logger;

    std::mutex m_lock;
    bool m_shutting_down = false;

    QComboBox* m_console_count_box;
    MultiSwitchSystemSession m_session;
    std::vector<SwitchSystemWidget*> m_switches;
    QWidget* m_videos;
//    std::map<size_t, SwitchSystem*> m_active_ports;
};


}
}
#endif
