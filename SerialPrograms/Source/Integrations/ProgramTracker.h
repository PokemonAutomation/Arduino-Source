/*  Program Tracker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *      A singleton class that keeps track of all live programs and handles.
 *  This allows Discord integration to safely interface with programs.
 *
 */

#ifndef PokemonAutomation_ProgramTracker_H
#define PokemonAutomation_ProgramTracker_H

#include <vector>
#include <map>
#include <memory>
#include "Common/Cpp/Concurrency/Mutex.h"
#include "CommonFramework/Globals.h"
#include "Controllers/Joystick.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ControllerButtons.h"
#include "IntegrationsAPI.h"
#include "ProgramTrackerInterfaces.h"

namespace PokemonAutomation{

class ImageRGB32;
class VideoFeed;
class AudioFeed;



struct ProgramTrackingState{
    std::string program_name;
    std::vector<uint64_t> console_ids;
    WallClock last_state_change;
    ProgramState state;
    std::string stats;
};



class ProgramTracker{
public:
    static ProgramTracker& instance();

    std::map<uint64_t, ProgramTrackingState> all_programs();

    std::string grab_screenshot     (uint64_t console_id, std::shared_ptr<const ImageRGB32>& image);
    std::string reset_camera        (uint64_t console_id);
    std::string reset_controller    (uint64_t console_id, uint64_t controller_index);
//    void change_program (uint64_t program_id, std::string program_identifier);
    std::string start_program       (uint64_t program_id);
    std::string stop_program        (uint64_t program_id);


public:
    //  Nintendo Switch
    std::string nsw_press_button(
        uint64_t console_id, uint64_t controller_index,
        Milliseconds duration,
        NintendoSwitch::Button button
    );
    std::string nsw_press_dpad(
        uint64_t console_id, uint64_t controller_index,
        Milliseconds duration,
        NintendoSwitch::DpadPosition position
    );
    std::string nsw_press_joystick(
        uint64_t console_id, uint64_t controller_index,
        Milliseconds duration,
        Integration::JoystickSide side,
        JoystickPosition position
    );


private:
    ProgramTracker() = default;
    ProgramTracker(const ProgramTracker&) = delete;
    void operator=(const ProgramTracker&) = delete;


public:
    uint64_t add_program(TrackableProgram& program);
    void remove_program(uint64_t program_id);

    uint64_t add_console(uint64_t program_id, TrackableConsole& console);
    void remove_console(uint64_t console_id);
    std::optional<uint64_t> add_console(std::optional<uint64_t> program_id, TrackableConsole& console);
    void remove_console(std::optional<uint64_t> console_id);


private:
    static std::string make_header(const std::string& function_name, uint64_t id);
    TrackableProgram* get_program(
        std::string& error,
        const std::string& header,
        uint64_t program_id
    );
    TrackableConsole* get_console(
        std::string& error,
        const std::string& header,
        uint64_t console_id
    );
    ControllerSession* get_controller(
        std::string& error,
        const std::string& header,
        uint64_t console_id,
        uint64_t controller_index
    );


private:
    struct ProgramData;

    Mutex m_lock;
    uint64_t m_program_instance_counter = 0;
    uint64_t m_console_instance_counter = 0;
    std::map<uint64_t, std::unique_ptr<ProgramData>> m_programs;
    std::map<uint64_t, std::pair<TrackableConsole*, uint64_t>> m_consoles;
};









}
#endif
