/*  Integrations API
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
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
        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
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
            str += duration_to_string(std::chrono::duration_cast<std::chrono::milliseconds>(now - item.second.start_time));
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
        str += "* **Consoles:** ";
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
    QImage image;
    std::string err = ProgramTracker::instance().grab_screenshot(console_id, image);
    if (!err.empty()){
        error = err;
        return;
    }
    if (image.save(QString::fromStdString(path))){
        error = DllSafeString();
    }else{
        error = std::string("Failed to save image to: ") + path;
    }
}

void pai_start_program(DllSafeString& error, uint64_t program_id){
    error = ProgramTracker::instance().start_program(program_id);
}
void pai_stop_program(DllSafeString& error, uint64_t program_id){
    error = ProgramTracker::instance().stop_program(program_id);
}

void pai_nsw_press_button(DllSafeString& error, uint64_t console_id, uint16_t button, uint16_t ticks){
    error = ProgramTracker::instance().nsw_press_button(console_id, button, ticks);
}
void pai_nsw_press_dpad(DllSafeString& error, uint64_t console_id, uint8_t position, uint16_t ticks){
    error = ProgramTracker::instance().nsw_press_dpad(console_id, position, ticks);
}
void pai_nsw_press_left_joystick(DllSafeString& error, uint64_t console_id, uint8_t x, uint8_t y, uint16_t ticks){
    error = ProgramTracker::instance().nsw_press_left_joystick(console_id, x, y, ticks);
}
void pai_nsw_press_right_joystick(DllSafeString& error, uint64_t console_id, uint8_t x, uint8_t y, uint16_t ticks){
    error = ProgramTracker::instance().nsw_press_right_joystick(console_id, x, y, ticks);
}





}
}
