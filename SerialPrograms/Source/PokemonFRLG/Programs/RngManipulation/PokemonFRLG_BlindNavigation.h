/*  Blind Navigation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_BlindNavigation_H
#define PokemonAutomation_PokemonFRLG_BlindNavigation_H

#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
    class ConsoleHandle;
    class ProController;
    using ProControllerContext = ControllerContext<ProController>;
namespace PokemonFRLG{


    enum class PokemonFRLG_RngTarget{
        starters,
        magikarp,
        hitmon,
        hitmonchan,
        hitmonlee,
        eevee,
        lapras,
        fossils,
        omanyte,
        kabuto,
        aerodactyl,
        gamecornerabra,
        gamecornerclefairy,
        gamecornerdratini,
        gamecornerbug,
        gamecornerscyther,
        gamecornerpinsir,
        gamecornerporygon,
        togepi,
        electrode,
        articuno,
        zapdos,
        moltres,
        lugia,
        deoxys_attack,
        deoxys_defense,
        staticencounter,
        snorlax,
        mewtwo,
        hooh,
        hypno,
        sweetscent,
        fishing,
        safarizonecenter,
        safarizoneeast,
        safarizonenorth,
        safarizonewest,
        safarizonesurf,
        safarizonefish,
        // roaming
    };

    enum class SeedButton{
        A,
        Start,
        L
    };

    enum class BlackoutButton{
        None,
        L,
        R
    };

    // checks seed, continue screen, and in-game timings for the specificed RNG manipulation target
    // and fires an error if any of the timings are too short.
    void check_timings(
        ConsoleHandle& console, 
        PokemonFRLG_RngTarget TARGET,
        uint64_t SEED_DELAY,
        uint64_t CONTINUE_SCREEN_DELAY, 
        uint64_t INGAME_DELAY,
        bool SAFARI_ZONE
    );

    // performs the blind sequence between launching the game and arriving at the RNG manipulation target
    void perform_blind_sequence(
        ProControllerContext& context, 
        PokemonFRLG_RngTarget TARGET,
        SeedButton SEED_BUTTON,
        BlackoutButton BLACKOUT_BUTTON,
        uint64_t SEED_DELAY,
        uint64_t CONTINUE_SCREEN_DELAY, 
        uint64_t TEACHY_DELAY, 
        uint64_t INGAME_DELAY, 
        bool SAFARI_ZONE
    );

}
}
}
#endif
