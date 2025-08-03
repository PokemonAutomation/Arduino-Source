/*  Catch Screen Tracker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_CatchScreenTracker_H
#define PokemonAutomation_PokemonSwSh_MaxLair_CatchScreenTracker_H

#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "PokemonSwSh/Inference/PokemonSwSh_SummaryShinySymbolDetector.h"

namespace PokemonAutomation{
    class ProgramEnvironment;
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
    CaughtPokemonScreen(VideoStream& stream, ProControllerContext& context);

    size_t total() const;
    const CaughtPokemon& operator[](size_t position) const;
          CaughtPokemon& operator[](size_t position);

    bool current_position() const;
    bool is_summary() const;

    void enter_summary();
    void leave_summary();
    void scroll_down();
    void scroll_to(size_t position);

private:
    void process_detection(Detection detection);

private:
    VideoStream& m_stream;
    ProControllerContext& m_context;
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
