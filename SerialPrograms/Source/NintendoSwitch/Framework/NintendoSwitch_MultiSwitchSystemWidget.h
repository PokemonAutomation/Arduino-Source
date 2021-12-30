/*  Switch System (4 Switches)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomationn_NintendoSwitch_SwitchSystem4Widget_H
#define PokemonAutomationn_NintendoSwitch_SwitchSystem4Widget_H

#include <QComboBox>
#include "NintendoSwitch_SwitchSetupWidget.h"
#include "NintendoSwitch_MultiSwitchSystem.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class MultiSwitchSystemWidget : public SwitchSetupWidget{
    Q_OBJECT

public:
    MultiSwitchSystemWidget(
        QWidget& parent,
        MultiSwitchSystemFactory& factory,
        Logger& logger,
        uint64_t program_id
    );
    void redraw_videos(size_t count);

    size_t switch_count() const{ return m_switches.size(); }
    SwitchSystemWidget& operator[](size_t index){ return *m_switches[index]; }

    virtual bool serial_ok() const override;
    virtual void wait_for_all_requests() override;
    virtual void stop_serial() override;
    virtual void reset_serial() override;

public:
    virtual void update_ui(ProgramState state) override;

private:
//    void change_serial(size_t old_index, size_t new_index, SwitchSystem& system);

private:
    uint64_t m_program_id;
    MultiSwitchSystemFactory& m_factory;
    Logger& m_logger;
    QComboBox* m_console_count_box;
    std::vector<SwitchSystemWidget*> m_switches;
    QWidget* m_videos;
//    std::map<size_t, SwitchSystem*> m_active_ports;
};


}
}
#endif
