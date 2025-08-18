/*  Panel Tracker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "Controllers/ControllerSession.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "ProgramTracker.h"

//#include <iostream>
//using std::cout;
//using std::endl;

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
    std::lock_guard<std::mutex> lg(m_lock);
    std::map<uint64_t, ProgramTrackingState> info;
    for (const auto& item : m_programs){
        info[item.first] = ProgramTrackingState{
            item.second->program.identifier(),
            item.second->console_ids,
            item.second->program.timestamp(),
            item.second->program.current_state(),
            item.second->program.current_stats()
        };
    }
    return info;
}

std::string ProgramTracker::grab_screenshot(uint64_t console_id, std::shared_ptr<const ImageRGB32>& image){
    std::lock_guard<std::mutex> lg(m_lock);
    auto iter = m_consoles.find(console_id);
    if (iter == m_consoles.end()){
        std::string error = "grab_screenshot(" + std::to_string(console_id) + ") - ID not found.";
        global_logger_tagged().log("SwitchProgramTracker::" + error, COLOR_RED);
        return error;
    }
    VideoSnapshot snapshot = iter->second.first->video().snapshot();
    image = std::move(snapshot.frame);
    return "";
}
std::string ProgramTracker::reset_camera(uint64_t console_id){
    std::lock_guard<std::mutex> lg(m_lock);
    auto iter = m_consoles.find(console_id);
    if (iter == m_consoles.end()){
        std::string error = "reset_camera(" + std::to_string(console_id) + ") - ID not found.";
        global_logger_tagged().log("SwitchProgramTracker::" + error, COLOR_RED);
        return error;
    }
    iter->second.first->video().reset();
    return "";
}
std::string ProgramTracker::reset_serial(uint64_t console_id){
    std::lock_guard<std::mutex> lg(m_lock);
    auto iter = m_consoles.find(console_id);
    if (iter == m_consoles.end()){
        std::string error = "reset_serial(" + std::to_string(console_id) + ") - ID not found.";
        global_logger_tagged().log("SwitchProgramTracker::" + error, COLOR_RED);
        return error;
    }
    std::string error = iter->second.first->controller().reset(false);
    return error.empty() ? "Controller was reset." : error;
}
std::string ProgramTracker::start_program(uint64_t program_id){
    std::lock_guard<std::mutex> lg(m_lock);
    auto iter = m_programs.find(program_id);
    if (iter == m_programs.end()){
        std::string error = "start_program(ID = " + std::to_string(program_id) + ") - ID not found.";
        global_logger_tagged().log("SwitchProgramTracker::" + error, COLOR_RED);
        return error;
    }
    iter->second->program.async_start();
    return "";
}
std::string ProgramTracker::stop_program(uint64_t program_id){
    std::lock_guard<std::mutex> lg(m_lock);
    auto iter = m_programs.find(program_id);
    if (iter == m_programs.end()){
        std::string error = "stop_program(ID = " + std::to_string(program_id) + ") - ID not found.";
        global_logger_tagged().log("SwitchProgramTracker::" + error, COLOR_RED);
        return error;
    }
    iter->second->program.async_stop();
    return "";
}
std::string ProgramTracker::nsw_press_button(uint64_t console_id, NintendoSwitch::Button button, uint16_t ticks){
    using namespace NintendoSwitch;
    std::string header = "press_button(ID = " + std::to_string(console_id) + ")";
    std::lock_guard<std::mutex> lg(m_lock);
    auto iter = m_consoles.find(console_id);
    if (iter == m_consoles.end()){
        std::string error = header + ": ID not found.";
        global_logger_tagged().log("SwitchProgramTracker::" + error, COLOR_RED);
        return error;
    }
    Milliseconds duration = ticks * 8ms;
    std::string err;
    try{
        err = iter->second.first->controller().try_run<ProController>(
            [=](ProController& controller){
                controller.issue_buttons(nullptr, duration, duration, 0ms, button);
            }
        );
    }catch (Exception& e){
        e.log(global_logger_tagged());
        err = e.to_str();
    }
    if (err.empty()){
        global_logger_tagged().log("SwitchProgramTracker::" + header, COLOR_BLUE);
        return "";
    }else{
        global_logger_tagged().log("SwitchProgramTracker::" + header + ": " + err, COLOR_RED);
        return err;
    }
}
std::string ProgramTracker::nsw_press_dpad(uint64_t console_id, NintendoSwitch::DpadPosition position, uint16_t ticks){
    using namespace NintendoSwitch;
    std::string header = "press_dpad(ID = " + std::to_string(console_id) + ")";
    std::lock_guard<std::mutex> lg(m_lock);
    auto iter = m_consoles.find(console_id);
    if (iter == m_consoles.end()){
        std::string error = header + ": ID not found.";
        global_logger_tagged().log("SwitchProgramTracker::" + error, COLOR_RED);
        return error;
    }
    Milliseconds duration = ticks * 8ms;
    std::string err;
    try{
        err = iter->second.first->controller().try_run<ProController>(
            [=](ProController& controller){
                controller.issue_dpad(nullptr, duration, duration, 0ms, position);
            }
        );
    }catch (Exception& e){
        e.log(global_logger_tagged());
        err = e.to_str();
    }
    if (err.empty()){
        global_logger_tagged().log("SwitchProgramTracker::" + header, COLOR_BLUE);
        return "";
    }else{
        global_logger_tagged().log("SwitchProgramTracker::" + header + ": " + err, COLOR_RED);
        return err;
    }
}
std::string ProgramTracker::nsw_press_left_joystick(uint64_t console_id, uint8_t x, uint8_t y, uint16_t ticks){
    using namespace NintendoSwitch;
    std::string header = "press_left_joystick(ID = " + std::to_string(console_id) + ")";
    std::lock_guard<std::mutex> lg(m_lock);
    auto iter = m_consoles.find(console_id);
    if (iter == m_consoles.end()){
        std::string error = header + ": ID not found.";
        global_logger_tagged().log("SwitchProgramTracker::" + error, COLOR_RED);
        return error;
    }
    Milliseconds duration = ticks * 8ms;
    std::string err;
    try{
        err = iter->second.first->controller().try_run<ProController>(
            [=](ProController& controller){
                controller.issue_left_joystick(nullptr, duration, duration, 0ms, x, y);
            }
        );
    }catch (Exception& e){
        e.log(global_logger_tagged());
        err = e.to_str();
    }
    if (err.empty()){
        global_logger_tagged().log("SwitchProgramTracker::" + header, COLOR_BLUE);
        return "";
    }else{
        global_logger_tagged().log("SwitchProgramTracker::" + header + ": " + err, COLOR_RED);
        return err;
    }
}
std::string ProgramTracker::nsw_press_right_joystick(uint64_t console_id, uint8_t x, uint8_t y, uint16_t ticks){
    using namespace NintendoSwitch;
    std::string header = "press_right_joystick(ID = " + std::to_string(console_id) + ")";
    std::lock_guard<std::mutex> lg(m_lock);
    auto iter = m_consoles.find(console_id);
    if (iter == m_consoles.end()){
        std::string error = header + ": ID not found.";
        global_logger_tagged().log("SwitchProgramTracker::" + error, COLOR_RED);
        return error;
    }
    Milliseconds duration = ticks * 8ms;
    std::string err;
    try{
        err = iter->second.first->controller().try_run<ProController>(
            [=](ProController& controller){
                controller.issue_right_joystick(nullptr, duration, duration, 0ms, x, y);
            }
        );
    }catch (Exception& e){
        e.log(global_logger_tagged());
        err = e.to_str();
    }
    if (err.empty()){
        global_logger_tagged().log("SwitchProgramTracker::" + header, COLOR_BLUE);
        return "";
    }else{
        global_logger_tagged().log("SwitchProgramTracker::" + header + ": " + err, COLOR_RED);
        return err;
    }
}





uint64_t ProgramTracker::add_program(TrackableProgram& program){
    std::lock_guard<std::mutex> lg(m_lock);
    m_program_instance_counter++;
    m_programs.emplace(
        m_program_instance_counter,
        std::unique_ptr<ProgramData>(new ProgramData(0, program))
    );
    return m_program_instance_counter;
}
void ProgramTracker::remove_program(uint64_t program_id){
    std::lock_guard<std::mutex> lg(m_lock);
    m_programs.erase(program_id);
}
uint64_t ProgramTracker::add_console(uint64_t program_id, TrackableConsole& console){
    std::lock_guard<std::mutex> lg(m_lock);
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
    std::lock_guard<std::mutex> lg(m_lock);
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







}
