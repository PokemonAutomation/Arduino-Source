/*  Pokemon Sword & Shield Arduino Programs
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

//
//  This is a template making your own program. The example here is a copy/paste
//  of the TurboA program. Feel free to modify accordingly.
//
//  Every program that is fully integrated into the package has 4 files:
//      1.  NativePrograms/(category)/ProgramName.h
//      2.  NativePrograms/(category)/ProgramName.c
//      3.  NativePrograms/(category)/ProgramName_Core.c
//      4.  GeneratorConfig/(category)/ProgramName.json
//
//  The program is then referenced by the following files/scripts:
//      1.  NativePrograms/(category)/Scripts/BuildAll.cmd
//      2.  NativePrograms/(category)/Scripts/BuildAllUnix.sh
//      3.  GeneratorConfig/(category)/ProgramList.txt
//
//  Thus to add a new program, you must edit all of the above.
//

#include "Common/PokemonSwSh/PokemonProgramIDs.h"
#include "NativePrograms/DeviceFramework/DeviceSettings.h"
#include "NativePrograms/NintendoSwitch/Libraries/FrameworkSettings.h"
#include "NativePrograms/NintendoSwitch/Libraries/NintendoSwitch_Device.h"
#include "NativePrograms/NintendoSwitch/Libraries/NintendoSwitch_PushButtons.h"
#include "NativePrograms/NintendoSwitch/Programs/CustomProgram.h"
#include "NativePrograms/PokemonSwSh/Libraries/PokemonSettings.h"


int main(void){
    //  Do not delete these two lines!
    start_program_callback();
    initialize_framework(PABB_PID_CUSTOM_PROGRAM);

    //  Start the program in the grip menu. Then go home.
    start_program_flash(CONNECT_CONTROLLER_DELAY);
    grip_menu_connect_go_home();

    //  Enter the game.
    pbf_press_button(BUTTON_HOME, 10, HOME_TO_GAME_DELAY);

    //  Turbo A forever...
    while (true){
        pbf_press_button(BUTTON_A, 5, 5);
    }

    //  Not really relevant here, but for programs that finish, go to
    //  Switch home to idle.
    pbf_press_button(BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
    end_program_callback();
    end_program_loop();
};

