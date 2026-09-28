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

enum JoystickSide{
    NEITHER,
    LEFT,
    RIGHT,
};

//  Empty error means no error.

void pai_run_command                (DllSafeString& error, const char* commands);

void pai_status                     (DllSafeString& description);
void pai_screenshot                 (DllSafeString& error, uint64_t console_id, const char* path);

void pai_reset_camera               (DllSafeString& error, uint64_t console_id);
void pai_reset_controller           (DllSafeString& error, uint64_t console_id, uint64_t controller_index);

void pai_start_program              (DllSafeString& error, uint64_t program_id);
void pai_stop_program               (DllSafeString& error, uint64_t program_id);

void pai_run_controller_command(
    DllSafeString& error,
    uint64_t console_id, uint64_t controller_index,
    uint32_t milliseconds,
    const char* command
);


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
inline std::string reset_controller(uint64_t console_id, uint64_t controller_index){
    DllSafeString error;
    pai_reset_controller(error, console_id, controller_index);
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
inline std::string run_controller_command(
    uint64_t console_id, uint64_t controller_index,
    uint32_t milliseconds,
    const char* command
){
    DllSafeString error;
    pai_run_controller_command(error, console_id, controller_index, milliseconds, command);
    return error;
}



}
}
#endif


