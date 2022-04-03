/*  Catch Screen Tracker
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_CatchScreenTracker_H
#define PokemonAutomation_PokemonSwSh_MaxLair_CatchScreenTracker_H

#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "PokemonSwSh/Inference/PokemonSwSh_SummaryShinySymbolDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


struct CaughtPokemon{
    bool read = false;
    bool shiny = false;
};

class CaughtPokemonScreen{
    using Detection = SummaryShinySymbolDetector::Detection;

public:
    CaughtPokemonScreen(ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context);

    size_t total() const;
    const CaughtPokemon& operator[](size_t position) const;

    bool current_position() const;
    bool is_summary() const;

    void enter_summary();
    void leave_summary();
    void scroll_down();
    void scroll_to(size_t position);

private:
    void process_detection(Detection detection);

private:
    ProgramEnvironment& m_env;
    BotBaseContext& m_context;
    ConsoleHandle& m_console;
    size_t m_total;
    size_t m_current_position = 0;
    bool m_in_summary = false;
    CaughtPokemon m_mons[4];
};



}
}
}
}
#endif
