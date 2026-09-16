/*  Panel Tracker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "Controllers/JoystickTools.h"
#include "Controllers/ControllerSession.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"
#include "NintendoSwitch/Controllers/Joycon/NintendoSwitch_Joycon.h"
#include "ProgramTracker.h"

//  REMOVE
#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{

using namespace std::chrono_literals;


ProgramTracker& ProgramTracker::instance(){
    static ProgramTracker obj;
    return obj;
}



struct ProgramTracker::ProgramData{
    size_t system_count;
    TrackableProgram& program;
    std::vector<uint64_t> console_ids;

    ProgramData(size_t p_system_count, TrackableProgram& p_program)
        : system_count(p_system_count)
        , program(p_program)
    {}
};


std::map<uint64_t, ProgramTrackingState> ProgramTracker::all_programs(){
    std::lock_guard<Mutex> lg(m_lock);
    std::map<uint64_t, ProgramTrackingState> info;
    for (const auto& item : m_programs){
        info[item.first] = ProgramTrackingState{
            item.second->program.identifier(),
            item.second->console_ids,
            item.second->program.last_state_change(),
            item.second->program.current_state(),
            item.second->program.current_stats()
        };
    }
    return info;
}

std::string ProgramTracker::make_header(const std::string& function_name, uint64_t id){
    if (id == (uint64_t)-1){
        return function_name + "()";
    }
    return function_name + "(ID = " + std::to_string(id) + ")";
}
TrackableProgram* ProgramTracker::get_program(
    std::string& error,
    const std::string& header,
    uint64_t program_id
){
    auto iter = m_programs.end();
    if (m_programs.empty()){
        error = header + ": No programs found.";
        global_logger_tagged().log("ProgramTracker::" + error, COLOR_RED);
        return nullptr;
    }else if (program_id != (uint64_t)-1){
        iter = m_programs.find(program_id);
    }else if (m_programs.size() == 1){
        iter = m_programs.begin();
    }else{
        error = header + ": Multiple programs found. Please specify an ID.";
        global_logger_tagged().log("ProgramTracker::" + error, COLOR_RED);
        return nullptr;
    }
    if (iter == m_programs.end()){
        error = header + ": ID not found.";
        global_logger_tagged().log("ProgramTracker::" + error, COLOR_RED);
        return nullptr;
    }

    return &iter->second->program;
}
TrackableConsole* ProgramTracker::get_console(
    std::string& error,
    const std::string& header,
    uint64_t console_id
){
    auto iter = m_consoles.end();
    if (m_consoles.empty()){
        error = header + ": No consoles found.";
        global_logger_tagged().log("ProgramTracker::" + error, COLOR_RED);
        return nullptr;
    }else if (console_id != (uint64_t)-1){
        iter = m_consoles.find(console_id);
    }else if (m_consoles.size() == 1){
        iter = m_consoles.begin();
    }else{
        error = header + ": Multiple consoles found. Please specify an ID.";
        global_logger_tagged().log("ProgramTracker::" + error, COLOR_RED);
        return nullptr;
    }
    if (iter == m_consoles.end()){
        error = header + ": ID not found.";
        global_logger_tagged().log("ProgramTracker::" + error, COLOR_RED);
        return nullptr;
    }

    return iter->second.first;
}
ControllerSession* ProgramTracker::get_controller(
    std::string& error,
    const std::string& header,
    uint64_t console_id,
    uint64_t controller_index
){
    TrackableConsole* console = get_console(error, header, console_id);
    if (console == nullptr){
        return nullptr;
    }
    if (controller_index >= console->controllers()){
        error = "reset_serial(" + std::to_string(console_id) + ") - Index out of bounds.";
        global_logger_tagged().log("ProgramTracker::" + error, COLOR_RED);
        return nullptr;
    }
    return &console->controller(controller_index);
}

std::string ProgramTracker::grab_screenshot(uint64_t console_id, std::shared_ptr<const ImageRGB32>& image){
    std::string header = make_header("grab_screenshot", console_id);
    std::lock_guard<Mutex> lg(m_lock);
    std::string error;
    TrackableConsole* console = get_console(error, header, console_id);
    if (console == nullptr){
        return error;
    }
    VideoSnapshot snapshot = console->video_feed().snapshot();
    image = std::move(snapshot.frame);
    return "";
}
std::string ProgramTracker::reset_camera(uint64_t console_id){
    std::string header = make_header("reset_camera", console_id);
    std::lock_guard<Mutex> lg(m_lock);
    std::string error;
    TrackableConsole* console = get_console(error, header, console_id);
    if (console == nullptr){
        return error;
    }
    console->video_feed().reset();
    return "";
}
std::string ProgramTracker::reset_controller(uint64_t console_id, uint64_t controller_index){
    std::string header = make_header("reset_controller", console_id);
    std::lock_guard<Mutex> lg(m_lock);
    std::string error;
    ControllerSession* controller = get_controller(error, header, console_id, controller_index);
    if (controller == nullptr){
        return error;
    }
    error = controller->reset(false);
    return error.empty() ? "Controller was reset." : error;
}
std::string ProgramTracker::start_program(uint64_t program_id){
    std::string header = make_header("start_program", program_id);
    std::lock_guard<Mutex> lg(m_lock);
    std::string error;
    TrackableProgram* program = get_program(error, header, program_id);
    if (program == nullptr){
        return error;
    }
    program->async_start();
    return "";
}
std::string ProgramTracker::stop_program(uint64_t program_id){
    std::string header = make_header("stop_program", program_id);
    std::lock_guard<Mutex> lg(m_lock);
    std::string error;
    TrackableProgram* program = get_program(error, header, program_id);
    if (program == nullptr){
        return error;
    }
    program->async_stop();
    return "";
}
std::string ProgramTracker::nsw_press_button(
    uint64_t console_id, uint64_t controller_index,
    Milliseconds duration,
    NintendoSwitch::Button button
){
    using namespace NintendoSwitch;
    std::string header = make_header("press_button", console_id);

    std::lock_guard<Mutex> lg(m_lock);

    std::string error;
    ControllerSession* controller = get_controller(error, header, console_id, controller_index);
    if (controller == nullptr){
        return error;
    }

    try{
        switch (controller->controller_class()){
        case ControllerClass::NintendoSwitch_ProController:
            error = controller->try_run<ProController>(
                [=](ProController& controller){
                    controller.issue_buttons(nullptr, duration, duration, 0ms, button);
                }
            );
            break;
        case ControllerClass::NintendoSwitch_LeftJoycon:
            error = controller->try_run<LeftJoycon>(
                [=](LeftJoycon& controller){
                    controller.issue_buttons(nullptr, duration, duration, 0ms, button);
                }
            );
            break;
        case ControllerClass::NintendoSwitch_RightJoycon:
            error = controller->try_run<RightJoycon>(
                [=](RightJoycon& controller){
                    controller.issue_buttons(nullptr, duration, duration, 0ms, button);
                }
            );
            break;
        default:
            error = "Incompatible controller class.";
        }
    }catch (Exception& e){
        e.log(global_logger_tagged());
        error = e.to_str();
    }
    if (error.empty()){
        global_logger_tagged().log("ProgramTracker::" + header, COLOR_BLUE);
        return "";
    }else{
        global_logger_tagged().log("ProgramTracker::" + header + ": " + error, COLOR_RED);
        return error;
    }
}
std::string ProgramTracker::nsw_press_dpad(
    uint64_t console_id, uint64_t controller_index,
    Milliseconds duration,
    NintendoSwitch::DpadPosition position
){
    using namespace NintendoSwitch;
    std::string header = make_header("press_dpad", console_id);

    std::lock_guard<Mutex> lg(m_lock);

    std::string error;
    ControllerSession* controller = get_controller(error, header, console_id, controller_index);
    if (controller == nullptr){
        return error;
    }

    try{
        error = controller->try_run<ProController>(
            [=](ProController& controller){
                controller.issue_dpad(nullptr, duration, duration, 0ms, position);
            }
        );
    }catch (Exception& e){
        e.log(global_logger_tagged());
        error = e.to_str();
    }
    if (error.empty()){
        global_logger_tagged().log("ProgramTracker::" + header, COLOR_BLUE);
        return "";
    }else{
        global_logger_tagged().log("ProgramTracker::" + header + ": " + error, COLOR_RED);
        return error;
    }
}
std::string ProgramTracker::nsw_press_joystick(
    uint64_t console_id, uint64_t controller_index,
    Milliseconds duration,
    Integration::JoystickSide side,
    JoystickPosition position
){
    using namespace NintendoSwitch;
    std::string header = make_header("press_left_joystick", console_id);

    std::lock_guard<Mutex> lg(m_lock);

    std::string error;
    ControllerSession* controller = get_controller(error, header, console_id, controller_index);
    if (controller == nullptr){
        return error;
    }

    ControllerClass type = controller->controller_class();
    try{
        auto procon_left = [=](ProController& controller){
            controller.issue_left_joystick(nullptr, duration, duration, 0ms, position);
        };
        auto procon_right = [=](ProController& controller){
            controller.issue_right_joystick(nullptr, duration, duration, 0ms, position);
        };
        auto joycon = [=](JoyconController& controller){
            controller.issue_joystick(nullptr, duration, duration, 0ms, position);
        };

        switch (type){
        case ControllerClass::NintendoSwitch_ProController:
            switch (side){
            case PokemonAutomation::Integration::JoystickSide::LEFT:
                error = controller->try_run<ProController>(procon_left);
                break;
            case PokemonAutomation::Integration::JoystickSide::RIGHT:
                error = controller->try_run<ProController>(procon_right);
                break;
            default:
                error = "Incompatible controller class.";
            }
            break;
        case ControllerClass::NintendoSwitch_LeftJoycon:
            switch (side){
            case PokemonAutomation::Integration::JoystickSide::NEITHER:
            case PokemonAutomation::Integration::JoystickSide::LEFT:
                error = controller->try_run<LeftJoycon>(joycon);
                break;
            default:
                error = "Incompatible controller class.";
            }
            break;
        case ControllerClass::NintendoSwitch_RightJoycon:
            switch (side){
            case PokemonAutomation::Integration::JoystickSide::NEITHER:
            case PokemonAutomation::Integration::JoystickSide::RIGHT:
                error = controller->try_run<RightJoycon>(joycon);
                break;
            default:
                error = "Incompatible controller class.";
            }
            break;
        default:
            error = "Incompatible controller class.";
        }

    }catch (Exception& e){
        e.log(global_logger_tagged());
        error = e.to_str();
    }
    if (error.empty()){
        global_logger_tagged().log("ProgramTracker::" + header, COLOR_BLUE);
        return "";
    }else{
        global_logger_tagged().log("ProgramTracker::" + header + ": " + error, COLOR_RED);
        return error;
    }
}





uint64_t ProgramTracker::add_program(TrackableProgram& program){
    std::lock_guard<Mutex> lg(m_lock);
    m_program_instance_counter++;
    m_programs.emplace(
        m_program_instance_counter,
        std::unique_ptr<ProgramData>(new ProgramData(0, program))
    );
    return m_program_instance_counter;
}
void ProgramTracker::remove_program(uint64_t program_id){
    std::lock_guard<Mutex> lg(m_lock);
    m_programs.erase(program_id);
}
uint64_t ProgramTracker::add_console(uint64_t program_id, TrackableConsole& console){
    std::lock_guard<Mutex> lg(m_lock);
    m_console_instance_counter++;
    m_consoles.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(m_console_instance_counter),
        std::forward_as_tuple(&console, program_id)
    );
    auto iter = m_programs.find(program_id);
    if (iter != m_programs.end()){
        iter->second->console_ids.emplace_back(m_console_instance_counter);
    }
    return m_console_instance_counter;
}
void ProgramTracker::remove_console(uint64_t console_id){
    std::lock_guard<Mutex> lg(m_lock);
    auto iter0 = m_consoles.find(console_id);
    if (iter0 == m_consoles.end()){
        return;
    }
    uint64_t program_id = iter0->second.second;
    m_consoles.erase(iter0);
    auto iter1 = m_programs.find(program_id);
    if (iter1 == m_programs.end()){
        return;
    }
    std::vector<uint64_t>& consoles = iter1->second->console_ids;
    for (auto iter = consoles.begin(); iter != consoles.end(); ++iter){
        if (*iter == console_id){
            consoles.erase(iter);
            break;
        }
    }
}
std::optional<uint64_t> ProgramTracker::add_console(std::optional<uint64_t> program_id, TrackableConsole& console){
    if (!program_id.has_value()){
        return std::nullopt;
    }
    return add_console(program_id.value(), console);
}
void ProgramTracker::remove_console(std::optional<uint64_t> console_id){
    if (console_id.has_value()){
        remove_console(console_id.value());
    }
}







}
