/*  Switch Panel Tracker
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/SwitchFramework/Switch_PushButtons.h"
//#include <Windows.h>
#include "SwitchProgramTracker.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{


SwitchProgramTracker& SwitchProgramTracker::instance(){
    static SwitchProgramTracker obj;
    return obj;
}


#if 0
SwitchPanelTracker::SwitchPanelTracker()
    : m_thread([=]{
        while (true){
            Sleep(1000);
            cout << "Program List:" << endl;
            for (const auto& item : all_programs()){
                cout << item.first << "\t: " << item.second.program_name << " - " << (int)item.second.state << endl;
            }
            cout << "Console List:" << endl;
            for (const auto& item : all_consoles()){
                cout << item.first << "\t: " << item.second.program_name << endl;
            }
        }
    })
{}
#endif




struct SwitchProgramTracker::ProgramData{
    size_t system_count;
    RunnableSwitchProgramWidget& program;

    ProgramData(size_t p_system_count, RunnableSwitchProgramWidget& p_program)
        : system_count(p_system_count)
        , program(p_program)
    {}
    virtual ~ProgramData() = default;
};


std::map<uint64_t, ProgramInfo> SwitchProgramTracker::all_programs(){
    std::lock_guard<std::mutex> lg(m_lock);
    std::map<uint64_t, ProgramInfo> info;
    for (const auto& item : m_all_programs){
        std::vector<uint64_t> ids;
        for (size_t c = 0; c < item.second->system_count; c++){
            ids.emplace_back(item.first * 10 + c);
        }
        info[item.first] = ProgramInfo{
            item.second->program.instance().descriptor().identifier(),
            std::move(ids),
            item.second->program.state()
        };
    }
    return info;
}
std::map<uint64_t, ConsoleInfo> SwitchProgramTracker::all_consoles(){
    std::lock_guard<std::mutex> lg(m_lock);
    std::map<uint64_t, ConsoleInfo> info;
    for (const auto& item : m_consoles){
        info[item.first] = ConsoleInfo{
            item.second.second->program.instance().descriptor().identifier()
        };
    }
    return info;
}

QImage SwitchProgramTracker::grab_screenshot(uint64_t console_id){
    std::lock_guard<std::mutex> lg(m_lock);
    auto iter = m_consoles.find(console_id);
    if (iter == m_consoles.end()){
        global_logger().log(
            "SwitchProgramTracker::grab_screenshot(" + std::to_string(console_id) + ") - ID not found.",
            "red"
        );
        return QImage();
    }
    return iter->second.first->camera().snapshot();
}
#if 0
void SwitchProgramTracker::reset_serial(uint64_t console_id){
    std::lock_guard<std::mutex> lg(m_lock);
    auto iter = m_consoles.find(console_id);
    if (iter == m_consoles.end()){
        return;
    }


}
#endif
void SwitchProgramTracker::start_program(uint64_t program_id){
    std::lock_guard<std::mutex> lg(m_lock);
    auto iter = m_all_programs.find(program_id);
    if (iter == m_all_programs.end()){
        global_logger().log(
            "SwitchProgramTracker::start_program(" + std::to_string(program_id) + ") - ID not found.",
            "red"
        );
        return;
    }
    iter->second->program.async_start();
}
void SwitchProgramTracker::stop_program(uint64_t program_id){
    std::lock_guard<std::mutex> lg(m_lock);
    auto iter = m_all_programs.find(program_id);
    if (iter == m_all_programs.end()){
        global_logger().log(
            "SwitchProgramTracker::stop_program(" + std::to_string(program_id) + ") - ID not found.",
            "red"
        );
        return;
    }
    iter->second->program.async_stop();
}
void SwitchProgramTracker::press_button(uint64_t console_id, Button button, uint16_t ticks){
    std::lock_guard<std::mutex> lg(m_lock);
    auto iter = m_consoles.find(console_id);
    if (iter == m_consoles.end()){
        global_logger().log(
            "SwitchProgramTracker::press_button(" + std::to_string(console_id) + ", ...) - ID not found.",
            "red"
        );
        return;
    }
    pabb_pbf_press_button params;
    params.button = button;
    params.hold_ticks = ticks;
    params.release_ticks = 0;
    iter->second.first->send_request<PABB_MSG_COMMAND_PBF_PRESS_BUTTON>(params);
}
void SwitchProgramTracker::press_dpad(uint64_t console_id, DpadPosition position, uint16_t ticks){
    std::lock_guard<std::mutex> lg(m_lock);
    auto iter = m_consoles.find(console_id);
    if (iter == m_consoles.end()){
        global_logger().log(
            "SwitchProgramTracker::press_dpad(" + std::to_string(console_id) + ", ...) - ID not found.",
            "red"
        );
        return;
    }
    pabb_pbf_press_dpad params;
    params.dpad = position;
    params.hold_ticks = ticks;
    params.release_ticks = 0;
    iter->second.first->send_request<PABB_MSG_COMMAND_PBF_PRESS_DPAD>(params);
}
void SwitchProgramTracker::press_left_joystick(uint64_t console_id, uint8_t x, uint8_t y, uint16_t ticks){
    std::lock_guard<std::mutex> lg(m_lock);
    auto iter = m_consoles.find(console_id);
    if (iter == m_consoles.end()){
        global_logger().log(
            "SwitchProgramTracker::press_left_joystick(" + std::to_string(console_id) + ", ...) - ID not found.",
            "red"
        );
        return;
    }
    pabb_pbf_move_joystick params;
    params.x = x;
    params.y = y;
    params.hold_ticks = ticks;
    params.release_ticks = 0;
    iter->second.first->send_request<PABB_MSG_COMMAND_PBF_MOVE_JOYSTICK_L>(params);
}
void SwitchProgramTracker::press_right_joystick(uint64_t console_id, uint8_t x, uint8_t y, uint16_t ticks){
    std::lock_guard<std::mutex> lg(m_lock);
    auto iter = m_consoles.find(console_id);
    if (iter == m_consoles.end()){
        global_logger().log(
            "SwitchProgramTracker::press_right_joystick(" + std::to_string(console_id) + ", ...) - ID not found.",
            "red"
        );
        return;
    }
    pabb_pbf_move_joystick params;
    params.x = x;
    params.y = y;
    params.hold_ticks = ticks;
    params.release_ticks = 0;
    iter->second.first->send_request<PABB_MSG_COMMAND_PBF_MOVE_JOYSTICK_R>(params);
}





struct SwitchProgramTracker::ProgramDataS : public ProgramData{
    SingleSwitchProgramWidget& program;
    ProgramDataS(SingleSwitchProgramWidget& p_program)
        : ProgramData(1, p_program)
        , program(p_program)
    {}
};
struct SwitchProgramTracker::ProgramDataM : public ProgramData{
    MultiSwitchProgramWidget& program;
    ProgramDataM(MultiSwitchProgramWidget& p_program)
        : ProgramData(p_program.system_count(), p_program)
        , program(p_program)
    {}
};



uint64_t SwitchProgramTracker::add_program(SingleSwitchProgramWidget& program){
    std::lock_guard<std::mutex> lg(m_lock);
    m_instance_counter++;
    m_program_lookup[&program] = m_instance_counter;
    auto ret = m_all_programs.emplace(
        m_instance_counter,
        std::unique_ptr<ProgramDataS>(new ProgramDataS(program))
    );
    m_consoles.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(m_instance_counter * 10),
        std::forward_as_tuple(&program.system(), ret.first->second.get())
    );
    return m_instance_counter;
}
void SwitchProgramTracker::remove_program(SingleSwitchProgramWidget& program){
    std::lock_guard<std::mutex> lg(m_lock);
    auto iter = m_program_lookup.find(&program);
    if (iter == m_program_lookup.end()){
        return;
    }
    m_consoles.erase(iter->second * 10);
    m_all_programs.erase(iter->second);
    m_program_lookup.erase(iter);
}
uint64_t SwitchProgramTracker::add_program(MultiSwitchProgramWidget& program){
    std::lock_guard<std::mutex> lg(m_lock);
    m_instance_counter++;
    m_program_lookup[&program] = m_instance_counter;
    auto ret = m_all_programs.emplace(
        m_instance_counter,
        std::unique_ptr<ProgramDataM>(new ProgramDataM(program))
    );
    for (size_t c = 0; c < program.system_count(); c++){
        m_consoles.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(m_instance_counter * 10 + c),
            std::forward_as_tuple(&program.system(c), ret.first->second.get())
        );
    }
    return m_instance_counter;
}
void SwitchProgramTracker::remove_program(MultiSwitchProgramWidget& program){
    std::lock_guard<std::mutex> lg(m_lock);
    auto iter = m_program_lookup.find(&program);
    if (iter == m_program_lookup.end()){
        return;
    }
    for (size_t c = 0; c < program.system_count(); c++){
        m_consoles.erase(iter->second * 10 + c);
    }
    m_all_programs.erase(iter->second);
    m_program_lookup.erase(iter);
}






}
}
