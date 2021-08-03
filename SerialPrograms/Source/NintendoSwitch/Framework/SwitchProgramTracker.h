/*  Switch Panel Tracker
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      A singleton class that keeps track of all live programs and handles.
 *  This allows Discord integration to safely interface with programs.
 *
 */

#ifndef PokemonAutomation_SwitchPanelTracker_H
#define PokemonAutomation_SwitchPanelTracker_H

#include <map>
#include <mutex>
#include "Common/SwitchFramework/SwitchControllerDefs.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"
#include "NintendoSwitch/Framework/MultiSwitchProgram.h"
#include "SwitchSystem.h"
#include "MultiSwitchSystem.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


struct ProgramInfo{
    std::string program_name;
    std::vector<uint64_t> console_ids;
    ProgramState state;
//    std::chrono::system_clock::time_point start_time;
};
struct ConsoleInfo{
    std::string program_name;
};



class SwitchProgramTracker{
public:
    static SwitchProgramTracker& instance();

    std::map<uint64_t, ProgramInfo> all_programs();
    std::map<uint64_t, ConsoleInfo> all_consoles();

    QImage grab_screenshot  (uint64_t console_id);
//    void reset_serial       (uint64_t console_id);
//    void change_program (uint64_t program_id, std::string program_identifier);
    void start_program  (uint64_t program_id);
    void stop_program   (uint64_t program_id);

    void press_button           (uint64_t console_id, Button button, uint16_t ticks);
    void press_dpad             (uint64_t console_id, DpadPosition position, uint16_t ticks);
    void press_left_joystick    (uint64_t console_id, uint8_t x, uint8_t y, uint16_t ticks);
    void press_right_joystick   (uint64_t console_id, uint8_t x, uint8_t y, uint16_t ticks);


private:
    SwitchProgramTracker() = default;
//    SwitchPanelTracker();
    SwitchProgramTracker(const SwitchProgramTracker&) = delete;
    void operator=(const SwitchProgramTracker&) = delete;

    uint64_t add_program(SingleSwitchProgramWidget& program);
    void remove_program(SingleSwitchProgramWidget& program);

    uint64_t add_program(MultiSwitchProgramWidget& program);
    void remove_program(MultiSwitchProgramWidget& program);


private:
    friend class SingleSwitchProgramWidget;
    friend class MultiSwitchProgramWidget;

    struct ProgramData;
    struct ProgramDataS;
    struct ProgramDataM;

    std::mutex m_lock;
    uint64_t m_instance_counter = 0;
    std::map<RunnableSwitchProgramWidget*, uint64_t> m_program_lookup;
    std::map<uint64_t, std::unique_ptr<ProgramData>> m_all_programs;
    std::map<uint64_t, std::pair<SwitchSystem*, ProgramData*>> m_consoles;

//    std::thread m_thread;
};


#if 0
class SingleSwitchProgramScope{
public:
    SingleSwitchProgramScope(SingleSwitchProgramWidget& program)
        : m_tracker(SwitchPanelTracker::instance())
        , m_program(program)
    {
        m_tracker.add_single_program(program);
    }
    ~SingleSwitchProgramScope(){
        m_tracker.remove_single_program(m_program);
    }

private:
    SwitchPanelTracker& m_tracker;
    SingleSwitchProgramWidget& m_program;
};
class MultiSwitchProgramScope{
public:
    MultiSwitchProgramScope(MultiSwitchProgramWidget& program)
        : m_tracker(SwitchPanelTracker::instance())
        , m_program(program)
    {
        m_tracker.add_multi_program(program);
    }
    ~MultiSwitchProgramScope(){
        m_tracker.remove_multi_program(m_program);
    }

private:
    SwitchPanelTracker& m_tracker;
    MultiSwitchProgramWidget& m_program;
};
#endif







}
}
#endif
