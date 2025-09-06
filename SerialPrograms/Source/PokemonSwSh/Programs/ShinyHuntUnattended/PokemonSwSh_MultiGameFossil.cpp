/*  Multiple Game Fossil
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Routines.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Programs/PokemonSwSh_GameEntry.h"
#include "PokemonSwSh_MultiGameFossil.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
    using namespace Pokemon;


MultiGameFossil_Descriptor::MultiGameFossil_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:MultiGameFossil",
        STRING_POKEMON + " SwSh", "Multi-Game Fossil Revive",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/MultiGameFossil.md",
        "Revive fossils. Supports multiple saves so you can go afk for longer than 5 hours.",
        ProgramControllerClass::StandardController_PerformanceClassSensitive,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}



MultiGameFossil::MultiGameFossil(){
    PA_ADD_OPTION(START_LOCATION);
    PA_ADD_OPTION(GAME_LIST);
}

void run_fossil_batch(
    ConsoleHandle& console,
    ProControllerContext& context,
    const FossilGame& batch,
    bool* game_slot_flipped,
    bool save_and_exit
){
    //  Sanitize Slots
    uint8_t game_slot = (uint8_t)batch.game_slot.current_value();
    uint8_t user_slot = (uint8_t)batch.user_slot.current_value();
    if (game_slot > 2){
        game_slot = 0;
    }

    console.log(
        std::string("Batch:") +
        "\nGame Slot: " + std::to_string(game_slot) +
        "\nUser Slot: " + std::to_string(user_slot) +
        "\nFossil: " + std::to_string(batch.fossil.current_value()) +
        "\nRevives: " + std::to_string(batch.revives)
    );

    //  Calculate current game slot.
    switch (game_slot){
    case 0:
        break;
    case 1:
        game_slot = *game_slot_flipped ? 2 : 0;
        break;
    case 2:
        game_slot = *game_slot_flipped ? 0 : 2;
        break;
    }

    start_game_from_home(
        console, context,
        ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST,
        game_slot,
        user_slot,
        false
    );
    if (game_slot == 2){
        *game_slot_flipped = !*game_slot_flipped;
    }

    //  Revive
#if 1
    for (uint16_t c = 0; c < batch.revives; c++){
#if 1
        ssf_mash_AZs(context, 170);
        pbf_wait(context, 65);
#else
        ssf_mash_AZs(context, 50);
        pbf_wait(context, 140);
        ssf_press_button1(context, BUTTON_A, 160);
#endif
        switch (batch.fossil){
        case Fossil::Dracozolt:
            ssf_press_button_ptv(context, BUTTON_A, 1280ms);
            break;
        case Fossil::Arctozolt:
            ssf_press_button_ptv(context, BUTTON_A, 1280ms);
            ssf_press_dpad_ptv(context, DPAD_DOWN, 40ms);
            break;
        case Fossil::Dracovish:
            ssf_press_dpad_ptv(context, DPAD_DOWN, 40ms);
            ssf_press_button_ptv(context, BUTTON_A, 1280ms);
            break;
        case Fossil::Arctovish:
            ssf_press_dpad_ptv(context, DPAD_DOWN, 40ms);
            ssf_press_button_ptv(context, BUTTON_A, 1280ms);
            ssf_press_dpad_ptv(context, DPAD_DOWN, 40ms);
            break;
        }

        if (context->performance_class() == ControllerPerformanceClass::SysbotBase){
            ssf_mash_AZs(context, 4000ms);
        }else{
            ssf_mash_AZs(context, 3200ms);
        }
        pbf_mash_button(
            context,
            BUTTON_B,
            GameSettings::instance().AUTO_DEPOSIT ? 1400 : 1520
        );
    }
    pbf_wait(context, 100);
#endif

    if (!save_and_exit){
//        ssf_press_button2(BUTTON_HOME, GAME_TO_HOME_DELAY_SAFE, 10);
        return;
    }

    //  Save game.
    ssf_press_button(context, BUTTON_X, GameSettings::instance().OVERWORLD_TO_MENU_DELAY0, 160ms);
    ssf_press_button(context, BUTTON_R, 1200ms, 160ms);
    ssf_press_button(context, BUTTON_A, 4000ms, 80ms);

    //  Exit game.
    ssf_press_button(context, BUTTON_HOME, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE0, 80ms);
    close_game_from_home(console, context);
}


void MultiGameFossil::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    if (START_LOCATION.start_in_grip_menu()){
        grip_menu_connect_go_home(context);
    }else{
        pbf_press_button(context, BUTTON_B, 5, 5);
        ssf_press_button(context, BUTTON_HOME, GameSettings::instance().GAME_TO_HOME_DELAY_FAST0, 160ms);
    }

    std::vector<std::unique_ptr<FossilGame>> list = GAME_LIST.copy_snapshot();

//    FossilGame2 batch;

    size_t games = list.size();

    bool game_slot_flipped = false;
    for (size_t c = 0; c < games; c++){
//        batch = GAME_LIST2[c];
        run_fossil_batch(env.console, context, *list[c], &game_slot_flipped, c + 1 < games);
    }

    ssf_press_button(context, BUTTON_HOME, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE0, 160ms);
}



}
}
}

