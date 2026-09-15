/*  Integrations API
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <memory>
#include "Common/Compiler.h"
#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "Integrations/ProgramTracker.h"
#include "IntegrationsAPI.h"

namespace PokemonAutomation{
namespace Integration{


void pai_run_command(DllSafeString& error, const char* commands){
    error = "Not supported yet.";

#if 0
    //  Split string into tokens.
    std::vector<std::string> tokens;
    std::istringstream stream(commands);
    std::string str;
    while (std::getline(stream, str, ' ')){
        if (!str.empty()){
            tokens.emplace_back(std::move(str));
        }
    }
#endif
}

void pai_status(DllSafeString& description){
    std::string str;
    for (const auto& item : ProgramTracker::instance().all_programs()){
        str += "__**Program " + std::to_string(item.first) + "**__\n";
        str += "* **Name:** " + item.second.program_name + "\n";
        str += "* **State:** ";
        WallClock now = current_time();
        switch (item.second.state){
        case ProgramState::NOT_READY:
            str += "Not Ready";
            break;
        case ProgramState::STOPPED:
            str += "Stopped";
            break;
        case ProgramState::RUNNING:
            str += "Running";
            str += " (";
            str += duration_to_string(
                std::chrono::duration_cast<std::chrono::milliseconds>(now - item.second.last_state_change)
            );
            str += ")";
            break;
        case ProgramState::STOPPING:
            str += "Stopping";
            break;
        }
        str += "\n";
        if (!item.second.stats.empty()){
            str += "* **Stats:** " + item.second.stats + "\n";
        }
        str += "* **Console ID(s):** ";
        bool first = true;
        if (item.second.console_ids.empty()){
            str += "No consoles enabled.";
        }
        for (uint64_t console_id : item.second.console_ids){
            if (!first){
                str += ", ";
            }
            first = false;
            str += std::to_string(console_id);
        }
        str += "\n\n";
    }
    if (str.empty()){
        str = "No programs running.";
    }
    description = str;
}
void pai_screenshot(DllSafeString& error, uint64_t console_id, const char* path){
    std::shared_ptr<const ImageRGB32> image;
    std::string err = ProgramTracker::instance().grab_screenshot(console_id, image);
    if (!err.empty()){
        error = err;
        return;
    }
    if (image->save(path)){
        error = DllSafeString();
    }else{
        error = std::string("Failed to save image to: ") + path;
    }
}

void pai_reset_camera(DllSafeString& error, uint64_t console_id){
    error = ProgramTracker::instance().reset_camera(console_id);
}
void pai_reset_controller(DllSafeString& error, uint64_t console_id, uint64_t controller_index){
    error = ProgramTracker::instance().reset_controller(console_id, controller_index);
}

void pai_start_program(DllSafeString& error, uint64_t program_id){
    error = ProgramTracker::instance().start_program(program_id);
}
void pai_stop_program(DllSafeString& error, uint64_t program_id){
    error = ProgramTracker::instance().stop_program(program_id);
}

void pai_nsw_press_button(
    DllSafeString& error,
    uint64_t console_id, uint64_t controller_index,
    uint32_t milliseconds,
    uint32_t button
){
    error = ProgramTracker::instance().nsw_press_button(
        console_id, controller_index,
        Milliseconds(milliseconds),
        (NintendoSwitch::Button)button
    );
}
void pai_nsw_press_dpad(
    DllSafeString& error,
    uint64_t console_id, uint64_t controller_index,
    uint32_t milliseconds,
    uint8_t position
){
    error = ProgramTracker::instance().nsw_press_dpad(
        console_id, controller_index,
        Milliseconds(milliseconds),
        (NintendoSwitch::DpadPosition)position
    );
}
void pai_nsw_press_left_joystick(
    DllSafeString& error,
    uint64_t console_id, uint64_t controller_index,
    uint32_t milliseconds,
    uint8_t x, uint8_t y
){
    error = ProgramTracker::instance().nsw_press_left_joystick(
        console_id, controller_index,
        Milliseconds(milliseconds),
        x, y
    );
}
void pai_nsw_press_right_joystick(
    DllSafeString& error,
    uint64_t console_id, uint64_t controller_index,
    uint32_t milliseconds,
    uint8_t x, uint8_t y
){
    error = ProgramTracker::instance().nsw_press_right_joystick(
        console_id, controller_index,
        Milliseconds(milliseconds),
        x, y
    );
}





}
}
