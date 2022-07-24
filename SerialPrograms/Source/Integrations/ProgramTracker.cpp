/*  Panel Tracker
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/Panels/RunnablePanelWidget.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Messages_PushButtons.h"
#include "ProgramTracker.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


ProgramTracker& ProgramTracker::instance(){
    static ProgramTracker obj;
    return obj;
}



struct ProgramTracker::ProgramData{
    size_t system_count;
    RunnablePanelWidget& program;
    std::vector<uint64_t> console_ids;

    ProgramData(size_t p_system_count, RunnablePanelWidget& p_program)
        : system_count(p_system_count)
        , program(p_program)
    {}
};


std::map<uint64_t, ProgramTrackingState> ProgramTracker::all_programs(){
    std::lock_guard<std::mutex> lg(m_lock);
    std::map<uint64_t, ProgramTrackingState> info;
    for (const auto& item : m_programs){
        info[item.first] = ProgramTrackingState{
            item.second->program.instance().descriptor().identifier(),
            item.second->console_ids,
            item.second->program.timestamp(),
            item.second->program.state(),
            item.second->program.stats()
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
    image = iter->second.first->video().snapshot();
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
    iter->second.first->video().async_reset_video();
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
    const char* msg = iter->second.first->sender().try_reset();
    return msg == nullptr ? "Serial connection was reset." : msg;
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
std::string ProgramTracker::nsw_press_button(uint64_t console_id, Button button, uint16_t ticks){
    std::string header = "press_button(ID = " + std::to_string(console_id) + ")";
    std::lock_guard<std::mutex> lg(m_lock);
    auto iter = m_consoles.find(console_id);
    if (iter == m_consoles.end()){
        std::string error = header + ": ID not found.";
        global_logger_tagged().log("SwitchProgramTracker::" + error, COLOR_RED);
        return error;
    }
    const char* err = iter->second.first->sender().try_send_request(
        NintendoSwitch::DeviceRequest_pbf_press_button(button, ticks, 0)
    );
    if (err){
        global_logger_tagged().log("SwitchProgramTracker::" + header + ": " + err, COLOR_RED);
        return err;
    }else{
        global_logger_tagged().log("SwitchProgramTracker::" + header, COLOR_BLUE);
        return "";
    }
}
std::string ProgramTracker::nsw_press_dpad(uint64_t console_id, DpadPosition position, uint16_t ticks){
    std::string header = "press_dpad(ID = " + std::to_string(console_id) + ")";
    std::lock_guard<std::mutex> lg(m_lock);
    auto iter = m_consoles.find(console_id);
    if (iter == m_consoles.end()){
        std::string error = header + ": ID not found.";
        global_logger_tagged().log("SwitchProgramTracker::" + error, COLOR_RED);
        return error;
    }
    const char* err = iter->second.first->sender().try_send_request(
        NintendoSwitch::DeviceRequest_pbf_press_dpad(position, ticks, 0)
    );
    if (err){
        global_logger_tagged().log("SwitchProgramTracker::" + header + ": " + err, COLOR_RED);
        return err;
    }else{
        global_logger_tagged().log("SwitchProgramTracker::" + header, COLOR_BLUE);
        return "";
    }
}
std::string ProgramTracker::nsw_press_left_joystick(uint64_t console_id, uint8_t x, uint8_t y, uint16_t ticks){
    std::string header = "press_left_joystick(ID = " + std::to_string(console_id) + ")";
    std::lock_guard<std::mutex> lg(m_lock);
    auto iter = m_consoles.find(console_id);
    if (iter == m_consoles.end()){
        std::string error = header + ": ID not found.";
        global_logger_tagged().log("SwitchProgramTracker::" + error, COLOR_RED);
        return error;
    }
    const char* err = iter->second.first->sender().try_send_request(
        NintendoSwitch::DeviceRequest_pbf_move_left_joystick(x, y, ticks, 0)
    );
    if (err){
        global_logger_tagged().log("SwitchProgramTracker::" + header + ": " + err, COLOR_RED);
        return err;
    }else{
        global_logger_tagged().log("SwitchProgramTracker::" + header, COLOR_BLUE);
        return "";
    }
}
std::string ProgramTracker::nsw_press_right_joystick(uint64_t console_id, uint8_t x, uint8_t y, uint16_t ticks){
    std::string header = "press_right_joystick(ID = " + std::to_string(console_id) + ")";
    std::lock_guard<std::mutex> lg(m_lock);
    auto iter = m_consoles.find(console_id);
    if (iter == m_consoles.end()){
        std::string error = header + ": ID not found.";
        global_logger_tagged().log("SwitchProgramTracker::" + error, COLOR_RED);
        return error;
    }
    const char* err = iter->second.first->sender().try_send_request(
        NintendoSwitch::DeviceRequest_pbf_move_right_joystick(x, y, ticks, 0)
    );
    if (err){
        global_logger_tagged().log("SwitchProgramTracker::" + header + ": " + err, COLOR_RED);
        return err;
    }else{
        global_logger_tagged().log("SwitchProgramTracker::" + header, COLOR_BLUE);
        return "";
    }
}





uint64_t ProgramTracker::add_program(RunnablePanelWidget& program){
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
uint64_t ProgramTracker::add_console(uint64_t program_id, ConsoleSystem& console){
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
