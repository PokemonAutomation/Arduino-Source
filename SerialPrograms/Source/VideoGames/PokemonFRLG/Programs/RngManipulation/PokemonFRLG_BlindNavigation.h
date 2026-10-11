/*  Blind Navigation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_BlindNavigation_H
#define PokemonAutomation_PokemonFRLG_BlindNavigation_H

#include "CommonFramework/Language.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleState.h"

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
        gamecornerdratinifr,
        gamecornerdratinilg,
        gamecornerscyther,
        gamecornerpinsir,
        gamecornerporygon,
        togepi,
        togepifast,
        eggheld,
        eggpickup,
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
        rocksmash,
        fishing,
        safarizonecenter,
        safarizoneeast,
        safarizonenorth,
        safarizonewest,
        safarizonesurf,
        safarizonefish,
        raikou,
        entei,
        suicune,
        roaming
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

    struct RngTimings{
        uint64_t seed_delay;
        uint64_t csf_delay;
        uint64_t teachy_delay;
        uint64_t ingame_delay;
    };


    // the approximate fixed difference (in frames) between the in-game delay and the advances hit for each target
    int64_t fixed_advances_offset(PokemonFRLG_RngTarget target);

    // checks seed, continue screen, and in-game timings for the specificed RNG manipulation target
    // and fires an error if any of the timings are too short.
    // ingame_advances and continue_screen_frames are the uncalibrated values the timings were prepared from.
    // They're used to report the minimum advances the target needs.
    void check_timings(
        ConsoleHandle& console,
        PokemonFRLG_RngTarget TARGET,
        const RngTimings& timings,
        Language language,
        uint64_t ingame_advances,
        uint64_t continue_screen_frames
    );

    // performs the blind sequence between launching the game and arriving at the RNG manipulation target
    void perform_blind_sequence(
        ProControllerContext& context,
        Language language,
        PokemonFRLG_RngTarget target,
        SeedButton seed_button,
        BlackoutButton extra_button,
        const RngTimings& timings,
        bool safari_zone,
        ConsoleType console_type
    );

}
}
}
#endif
