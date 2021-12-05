/*  Game Entry Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_Commands_GameEntry_H
#define PokemonAutomation_PokemonSwSh_Commands_GameEntry_H

#include "Common/PokemonSwSh/PokemonSwSh_Protocol_GameEntry.h"
#include "ClientSource/Connection/BotBase.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


void resume_game_no_interact            (const BotBaseContext& device, bool tolerate_update_menu);
void resume_game_back_out               (const BotBaseContext& device, bool tolerate_update_menu, uint16_t mash_B_time);
void resume_game_front_of_den_nowatts   (const BotBaseContext& device, bool tolerate_update_menu);
void settings_to_enter_game             (const BotBaseContext& device, bool fast);
void settings_to_enter_game_den_lobby   (
    const BotBaseContext& device,
    bool tolerate_update_menu, bool fast,
    uint16_t enter_switch_pokemon_delay,
    uint16_t exit_switch_pokemon_delay
);
void enter_game                         (const BotBaseContext& device, bool backup_save, uint16_t enter_game_mash, uint16_t enter_game_wait);
void start_game_from_home               (const BotBaseContext& device, bool tolerate_update_menu, uint8_t game_slot, uint8_t user_slot, bool backup_save);
void fast_reset_game(
    const BotBaseContext& device,
    uint16_t start_game_mash, uint16_t start_game_wait,
    uint16_t enter_game_mash, uint16_t enter_game_wait
);
void reset_game_from_home               (const BotBaseContext& device, bool tolerate_update_menu);



class DeviceRequest_fast_reset_game : public BotBaseRequest{
public:
    pabb_fast_reset_game params;
    DeviceRequest_fast_reset_game(
        uint16_t start_game_mash, uint16_t start_game_wait,
        uint16_t enter_game_mash, uint16_t enter_game_wait
    )
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
        params.start_game_mash = start_game_mash;
        params.start_game_wait = start_game_wait;
        params.enter_game_mash = enter_game_mash;
        params.enter_game_wait = enter_game_wait;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_FAST_RESET_GAME, params);
    }
};
class DeviceRequest_settings_to_enter_game_den_lobby : public BotBaseRequest{
public:
    pabb_settings_to_enter_game_den_lobby params;
    DeviceRequest_settings_to_enter_game_den_lobby(
        bool tolerate_update_menu, bool fast,
        uint16_t enter_switch_pokemon_delay,
        uint16_t exit_switch_pokemon_delay
    )
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
        params.tolerate_update_menu = tolerate_update_menu;
        params.fast = fast;
        params.enter_switch_pokemon_delay = enter_switch_pokemon_delay;
        params.exit_switch_pokemon_delay = exit_switch_pokemon_delay;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_SETTINGS_TO_ENTER_GAME_DEN_LOBBY, params);
    }
};
class DeviceRequest_enter_game : public BotBaseRequest{
public:
    pabb_enter_game params;
    DeviceRequest_enter_game(
        bool backup_save, uint16_t enter_game_mash, uint16_t enter_game_wait
    )
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
        params.backup_save = backup_save;
        params.enter_game_mash = enter_game_mash;
        params.enter_game_wait = enter_game_wait;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_ENTER_GAME, params);
    }
};



}
}
}
#endif
