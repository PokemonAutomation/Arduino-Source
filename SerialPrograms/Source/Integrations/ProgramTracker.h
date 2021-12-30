/*  Program Tracker
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      A singleton class that keeps track of all live programs and handles.
 *  This allows Discord integration to safely interface with programs.
 *
 */

#ifndef PokemonAutomation_ProgramTracker_H
#define PokemonAutomation_ProgramTracker_H

#include <vector>
#include <map>
#include <mutex>
#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/Tools/BotBaseHandle.h"
#include "CommonFramework/Tools/VideoFeed.h"

namespace PokemonAutomation{

class RunnablePanelWidget;


class ConsoleSystem{
public:
    virtual VideoFeed& video() = 0;
    virtual BotBaseHandle& sender() = 0;
};



struct ProgramTrackingState{
    std::string program_name;
    std::vector<uint64_t> console_ids;
    std::chrono::system_clock::time_point start_time;
    ProgramState state;
    std::string stats;
};



class ProgramTracker{
public:
    static ProgramTracker& instance();

    std::map<uint64_t, ProgramTrackingState> all_programs();

    std::string grab_screenshot     (uint64_t console_id, QImage& image);
    std::string reset_camera        (uint64_t console_id);
    std::string reset_serial        (uint64_t console_id);
//    void change_program (uint64_t program_id, std::string program_identifier);
    std::string start_program       (uint64_t program_id);
    std::string stop_program        (uint64_t program_id);


public:
    //  Nintendo Switch
    std::string nsw_press_button        (uint64_t console_id, Button button, uint16_t ticks);
    std::string nsw_press_dpad          (uint64_t console_id, DpadPosition position, uint16_t ticks);
    std::string nsw_press_left_joystick (uint64_t console_id, uint8_t x, uint8_t y, uint16_t ticks);
    std::string nsw_press_right_joystick(uint64_t console_id, uint8_t x, uint8_t y, uint16_t ticks);


private:
    ProgramTracker() = default;
    ProgramTracker(const ProgramTracker&) = delete;
    void operator=(const ProgramTracker&) = delete;

public:
    uint64_t add_program(RunnablePanelWidget& program);
    void remove_program(uint64_t program_id);

    uint64_t add_console(uint64_t program_id, ConsoleSystem& console);
    void remove_console(uint64_t console_id);


private:
    struct ProgramData;

    std::mutex m_lock;
    uint64_t m_program_instance_counter = 0;
    uint64_t m_console_instance_counter = 0;
    std::map<uint64_t, std::unique_ptr<ProgramData>> m_programs;
    std::map<uint64_t, std::pair<ConsoleSystem*, uint64_t>> m_consoles;
};









}
#endif
