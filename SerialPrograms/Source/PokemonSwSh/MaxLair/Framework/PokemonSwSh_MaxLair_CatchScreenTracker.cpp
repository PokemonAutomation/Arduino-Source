/*  Catch Screen Tracker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_EndBattle.h"
#include "PokemonSwSh_MaxLair_CatchScreenTracker.h"

//#define PA_FORCE_SLOT_SHINY     2

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


CaughtPokemonScreen::CaughtPokemonScreen(VideoStream& stream, ProControllerContext& context)
    : m_stream(stream)
    , m_context(context)
    , m_total(count_catches(stream.overlay(), stream.video().snapshot()))
{
    if (m_total == 0 || m_total > 4){
        stream.log("Detected " + std::to_string(m_total) + " catches. Something is wrong.", COLOR_RED);
    }
}

size_t CaughtPokemonScreen::total() const{
    return m_total;
}
const CaughtPokemon& CaughtPokemonScreen::operator[](size_t position) const{
    return m_mons[position];
}
CaughtPokemon& CaughtPokemonScreen::operator[](size_t position){
    return m_mons[position];
}
bool CaughtPokemonScreen::current_position() const{
    return m_current_position;
}
bool CaughtPokemonScreen::is_summary() const{
    return m_in_summary;
}

void CaughtPokemonScreen::enter_summary(){
    SummaryShinySymbolDetector detector(m_stream.logger(), m_stream.overlay());
    if (m_in_summary){
        //  Make sure we're actually in the summary screen.
        process_detection(detector.wait_for_detection(m_context, m_stream.video()));
        return;
    }

    pbf_press_button(m_context, BUTTON_A, 10, 100);
    pbf_press_dpad(m_context, DPAD_DOWN, 10, 50);
    pbf_press_button(m_context, BUTTON_A, 10, 0);
    m_context.wait_for_all_requests();

    Detection detection = detector.wait_for_detection(m_context, m_stream.video());
    m_in_summary = true;
    process_detection(detection);
}
void CaughtPokemonScreen::leave_summary(){
    if (!m_in_summary){
        return;
    }

    //  Make sure we're actually in the summary screen.
    SummaryShinySymbolDetector detector(m_stream.logger(), m_stream.overlay());
    process_detection(detector.wait_for_detection(m_context, m_stream.video()));

    pbf_press_button(m_context, BUTTON_B, 10, TICKS_PER_SECOND);

    PokemonCaughtMenuDetector caught_menu;

    int result = wait_until(
        m_stream, m_context,
        std::chrono::seconds(10),
        {{caught_menu}}
    );

    switch (result){
    case 0:
        pbf_wait(m_context, 125);
        m_context.wait_for_all_requests();
        break;
    default:
//        auto snapshot = m_stream.video().snapshot();
//        dump_image(m_stream, m_env.program_info(), "CaughtMenu", snapshot);
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Failed to detect caught menu.",
            m_stream
        );
    }

    m_in_summary = false;
}
void CaughtPokemonScreen::scroll_down(){
    pbf_press_dpad(m_context, DPAD_DOWN, 10, TICKS_PER_SECOND);
    m_context.wait_for_all_requests();
    m_current_position++;
    if (m_current_position >= m_total){
        m_current_position = 0;
    }
    if (m_in_summary){
        SummaryShinySymbolDetector detector(m_stream.logger(), m_stream.overlay());
        Detection detection = detector.wait_for_detection(m_context, m_stream.video());
        process_detection(detection);
    }
}
void CaughtPokemonScreen::scroll_to(size_t position){
    while (m_current_position != position){
        scroll_down();
    }
}
void CaughtPokemonScreen::process_detection(Detection detection){
#ifdef PA_FORCE_SLOT_SHINY
        if (m_current_position == PA_FORCE_SLOT_SHINY){
            detection = Detection::SHINY;
        }
#endif
    CaughtPokemon& mon = m_mons[m_current_position];
    switch (detection){
    case SummaryShinySymbolDetector::Detection::NO_DETECTION:
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Failed to detect summary screen.",
            m_stream
        );
    case SummaryShinySymbolDetector::Detection::NOT_SHINY:
        if (!mon.read){
            m_stream.log("Not shiny.", COLOR_BLUE);
            mon.shiny = false;
            mon.read = true;
        }else if (mon.shiny){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Fatal Inconsistency: Expected to see a non-shiny.",
                m_stream
            );
        }
        break;
    case SummaryShinySymbolDetector::Detection::SHINY:
        if (!mon.read){
            m_stream.log("Found shiny!", COLOR_BLUE);
            mon.shiny = true;
            mon.read = true;
        }else if (!mon.shiny){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Fatal Inconsistency: Expected to see a shiny.",
                m_stream
            );
        }
        break;
    }
}



}
}
}
}
