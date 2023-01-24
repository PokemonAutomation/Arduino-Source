/*  Multiple Game Fossil
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Device.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Routines.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/FixedInterval.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_GameEntry.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_Misc.h"
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
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



MultiGameFossil::MultiGameFossil(){
    PA_ADD_OPTION(START_LOCATION);
    PA_ADD_OPTION(GAME_LIST);
}

void run_fossil_batch(
    Logger& logger,
    BotBaseContext& context,
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

    logger.log(
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
        context,
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
        mash_A(context, 170);
        pbf_wait(context, 65);
#else
        mash_A(context, 50);
        pbf_wait(context, 140);
        ssf_press_button1(context, BUTTON_A, 160);
#endif
        switch (batch.fossil){
        case Fossil::Dracozolt:
            ssf_press_button1(context, BUTTON_A, 160);
            break;
        case Fossil::Arctozolt:
            ssf_press_button1(context, BUTTON_A, 160);
            ssf_press_dpad1(context, DPAD_DOWN, 5);
            break;
        case Fossil::Dracovish:
            ssf_press_dpad1(context, DPAD_DOWN, 5);
            ssf_press_button1(context, BUTTON_A, 160);
            break;
        case Fossil::Arctovish:
            ssf_press_dpad1(context, DPAD_DOWN, 5);
            ssf_press_button1(context, BUTTON_A, 160);
            ssf_press_dpad1(context, DPAD_DOWN, 5);
            break;
        }
        mash_A(context, 400);
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
    ssf_press_button2(context, BUTTON_X, GameSettings::instance().OVERWORLD_TO_MENU_DELAY, 20);
    ssf_press_button2(context, BUTTON_R, 150, 20);
    ssf_press_button2(context, BUTTON_A, 500, 10);

    //  Exit game.
    ssf_press_button2(context, BUTTON_HOME, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE, 10);
    close_game(context);
}


void MultiGameFossil::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    if (START_LOCATION.start_in_grip_menu()){
        grip_menu_connect_go_home(context);
    }else{
        pbf_press_button(context, BUTTON_B, 5, 5);
        pbf_press_button(context, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY_FAST);
    }

    std::vector<std::unique_ptr<FossilGame>> list = GAME_LIST.copy_snapshot();

//    FossilGame2 batch;

    size_t games = list.size();

    bool game_slot_flipped = false;
    for (size_t c = 0; c < games; c++){
//        batch = GAME_LIST2[c];
        run_fossil_batch(env.logger(), context, *list[c], &game_slot_flipped, c + 1 < games);
    }

    ssf_press_button2(context, BUTTON_HOME, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE, 10);
}



}
}
}

