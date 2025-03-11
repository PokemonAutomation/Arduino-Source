/*  Integrations API
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_IntegrationsAPI_H
#define PokemonAutomation_IntegrationsAPI_H

#include <stdint.h>
#include "Common/Cpp/Containers/DllSafeString.h"

namespace PokemonAutomation{
namespace Integration{
extern "C" {


//  Empty error means no error.

void pai_run_command                (DllSafeString& error, const char* commands);

void pai_status                     (DllSafeString& description);
void pai_screenshot                 (DllSafeString& error, uint64_t console_id, const char* path);

void pai_reset_camera               (DllSafeString& error, uint64_t console_id);
void pai_reset_serial               (DllSafeString& error, uint64_t console_id);

void pai_start_program              (DllSafeString& error, uint64_t program_id);
void pai_stop_program               (DllSafeString& error, uint64_t program_id);

void pai_nsw_press_button           (DllSafeString& error, uint64_t console_id, uint16_t button, uint16_t ticks);
void pai_nsw_press_dpad             (DllSafeString& error, uint64_t console_id, uint8_t position, uint16_t ticks);
void pai_nsw_press_left_joystick    (DllSafeString& error, uint64_t console_id, uint8_t x, uint8_t y, uint16_t ticks);
void pai_nsw_press_right_joystick   (DllSafeString& error, uint64_t console_id, uint8_t x, uint8_t y, uint16_t ticks);


}



inline std::string status(){
    DllSafeString description;
    pai_status(description);
    return description;
}
inline std::string screenshot(uint64_t console_id, const char* path){
    DllSafeString error;
    pai_screenshot(error, console_id, path);
    return error;
}
inline std::string reset_camera(uint64_t console_id){
    DllSafeString error;
    pai_reset_camera(error, console_id);
    return error;
}
inline std::string reset_serial(uint64_t console_id){
    DllSafeString error;
    pai_reset_serial(error, console_id);
    return error;
}
inline std::string start_program(uint64_t program_id){
    DllSafeString error;
    pai_start_program(error, program_id);
    return error;
}
inline std::string stop_program(uint64_t program_id){
    DllSafeString error;
    pai_stop_program(error, program_id);
    return error;
}
inline std::string press_button(uint64_t console_id, uint16_t button, uint16_t ticks){
    DllSafeString error;
    pai_nsw_press_button(error, console_id, button, ticks);
    return error;
}
inline std::string press_dpad(uint64_t console_id, uint8_t position, uint16_t ticks){
    DllSafeString error;
    pai_nsw_press_dpad(error, console_id, position, ticks);
    return error;
}
inline std::string press_left_joystick(uint64_t console_id, uint8_t x, uint8_t y, uint16_t ticks){
    DllSafeString error;
    pai_nsw_press_left_joystick(error, console_id, x, y, ticks);
    return error;
}
inline std::string press_right_joystick(uint64_t console_id, uint8_t x, uint8_t y, uint16_t ticks){
    DllSafeString error;
    pai_nsw_press_right_joystick(error, console_id, x, y, ticks);
    return error;
}



}
}
#endif


