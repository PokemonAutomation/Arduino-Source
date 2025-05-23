#include "PokemonHome_HomeEnvironment.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{

Cursor::Cursor(SingleSwitchProgramEnvironment& env, ProControllerContext& context){

}

void Cursor::locate_position(){
    // TODO: Implement locate_position logic
}

void Cursor::move_cursor_to(SingleSwitchProgramEnvironment& env, ProControllerContext& context, const std::pair<size_t, size_t>& cursor){
    // TODO: Implement cursor movement logic
}

void Cursor::pick_up_pokemon(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    // TODO: Implement pick up logic
}

void Cursor::put_down_pokemon(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    // TODO: Implement put down logic
}



PokemonHome_HomeEnvironment::PokemonHome_HomeEnvironment(SingleSwitchProgramEnvironment& env, ProControllerContext& context)
    : cursor(env, context)
{
    DetectHome(env, context);
}

void PokemonHome_HomeEnvironment::DetectHome(SingleSwitchProgramEnvironment& env, ProControllerContext& context){

}

}
}
}
