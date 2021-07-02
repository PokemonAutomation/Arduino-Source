/*  Game Entry Routines
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#ifndef PokemonAutomation_PokemonSwShGameEntry_H
#define PokemonAutomation_PokemonSwShGameEntry_H

#ifdef __AVR__
#include "NativePrograms/SwitchFramework/Master.h"
#endif
#include "Common/MessageProtocol.h"
#include "Common/SwitchFramework/SwitchControllerDefs.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  Universal
#if 0

//  From the Switch home menu, resume the game.
//  Nothing in front of you should be interactable.
void resume_game_no_interact(bool tolerate_update_menu);

//  From the Switch home menu, resume the game.
//  If something in front of you is interactable, you should be able to back out
//  by mashing B.
void resume_game_back_out(bool tolerate_update_menu, uint16_t mash_B_time);

//  From the Switch home menu, resume the game.
//  You may be standing in front of a den.
void resume_game_front_of_den_nowatts(bool tolerate_update_menu);

//  From anywhere in the Switch settings except the home menu, return to the game.
//  This operation is intended to be fast.
//  If (fast = true), it will run faster, but with a small chance of not making it into the game.
//
//  This function does not wait after the final press. Recommend waiting 100 ticks
//  before you are safely in the game.
void settings_to_enter_game(bool fast);

//  From anywhere in the Switch settings except the home menu, return to the game.
//  Return to the game where you are inside an open den lobby with the cursor over
//  "Switch Pokemon".
void settings_to_enter_game_den_lobby(
    bool tolerate_update_menu, bool fast,
    uint16_t enter_switch_pokemon_delay,
    uint16_t exit_switch_pokemon_delay
);

//  Enter the game when you're sitting in the game intro.
void enter_game(bool backup_save, uint16_t enter_game_mash, uint16_t enter_game_wait);

//  With the cursor over the game you wish to close, close it in a reliable manner.
void close_game(void);

//  From the Switch home with the cursor over the 1st game and the game closed,
//  start the specified game. When the function returns, you should be in the
//  game and ready to play. (not the loading menu)
void start_game_from_home(
    bool tolerate_update_menu,
    uint8_t game_slot,
    uint8_t user_slot,
    bool backup_save
);

//  With the cursor over the game you wish to reset, close and reopen the game
//  from the same user. There must be no system update window and starting the
//  game must not require checking the internet.
void fast_reset_game(
    uint16_t start_game_mash, uint16_t start_game_wait,
    uint16_t enter_game_mash, uint16_t enter_game_wait
);

//  From the Switch home menu, close the game and open another game.
//      If "user_slot" is 0, start whatever the current user is.
//      If "game_slot2" is true, start the game in the 2nd slot instead of the 1st slot.
void reset_game_from_home(bool tolerate_update_menu);

#endif
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  Client Side
#ifdef __cplusplus
namespace PokemonAutomation{
    class BotBaseContext;

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
    void close_game                         (const BotBaseContext& device);
    void start_game_from_home               (const BotBaseContext& device, bool tolerate_update_menu, uint8_t game_slot, uint8_t user_slot, bool backup_save);
    void fast_reset_game(
        const BotBaseContext& device,
        uint16_t start_game_mash, uint16_t start_game_wait,
        uint16_t enter_game_mash, uint16_t enter_game_wait
    );
    void reset_game_from_home               (const BotBaseContext& device, bool tolerate_update_menu);
}
#endif
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  Protocols
#if _WIN32
#pragma pack(push, 1)
#define PABB_PACK
#else
#define PABB_PACK   __attribute__((packed))
#endif
////////////////////////////////////////////////////////////////////////////////

#define PABB_MSG_COMMAND_SETTINGS_TO_ENTER_GAME_DEN_LOBBY       0xb0
typedef struct{
    seqnum_t seqnum;
    bool tolerate_update_menu;
    bool fast;
    uint16_t enter_switch_pokemon_delay;
    uint16_t exit_switch_pokemon_delay;
} PABB_PACK pabb_settings_to_enter_game_den_lobby;

#define PABB_MSG_COMMAND_ENTER_GAME                             0xb1
typedef struct{
    seqnum_t seqnum;
    bool backup_save;
    uint16_t enter_game_mash;
    uint16_t enter_game_wait;
} PABB_PACK pabb_enter_game;

#define PABB_MSG_COMMAND_CLOSE_GAME                             0xb2
typedef struct{
    seqnum_t seqnum;
} PABB_PACK pabb_close_game;

#define PABB_MSG_COMMAND_FAST_RESET_GAME                        0xb3
typedef struct{
    seqnum_t seqnum;
    uint16_t start_game_mash;
    uint16_t start_game_wait;
    uint16_t enter_game_mash;
    uint16_t enter_game_wait;
} PABB_PACK pabb_fast_reset_game;

////////////////////////////////////////////////////////////////////////////////
#if _WIN32
#pragma pack(pop)
#endif
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#endif

