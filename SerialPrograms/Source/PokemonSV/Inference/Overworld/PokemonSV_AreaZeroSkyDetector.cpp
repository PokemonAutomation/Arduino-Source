/*  Area Zero Sky Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "CommonTools/Async/InterruptableCommands.h"
#include "CommonTools/Async/InferenceSession.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV_AreaZeroSkyDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



bool AreaZeroSkyDetector::detect(Kernels::Waterfill::WaterfillObject& object, const ImageViewRGB32& screen) const{
    using namespace Kernels::Waterfill;

    PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(screen, 0xffc0c0c0, 0xffffffff);

    size_t min_width = screen.width() / 4;
    size_t min_height = screen.height() / 4;

    std::unique_ptr<WaterfillSession> session = make_WaterfillSession(matrix);
    auto iter = session->make_iterator(10000);
    while (iter->find_next(object, false)){
//        if (object.min_y != 0){
//            continue;
//        }
        if (object.width() < min_width || object.height() < min_height){
            continue;
        }
        return true;
    }
    return false;
}
bool AreaZeroSkyDetector::detect(const ImageViewRGB32& screen){
    using namespace Kernels::Waterfill;

    WaterfillObject object;
    return detect(object, screen);
}




AreaZeroSkyTracker::AreaZeroSkyTracker(VideoOverlay& overlay)
    : VisualInferenceCallback("AreaZeroSkyTracker")
    , m_overlay(overlay)
{}

bool AreaZeroSkyTracker::sky_location(double& x, double& y) const{
    ReadSpinLock lg(m_lock);
    if (!m_box){
        return false;
    }
    x = m_center_x;
    y = m_center_y;
    return true;
}

void AreaZeroSkyTracker::make_overlays(VideoOverlaySet& items) const{}
bool AreaZeroSkyTracker::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    using namespace Kernels::Waterfill;

    WaterfillObject object;
    bool detected = this->detect(object, frame);
    WriteSpinLock lg(m_lock);
    if (detected){
        m_box.reset(new OverlayBoxScope(
            m_overlay,
            COLOR_GREEN,
            translate_to_parent(frame, {0, 0, 1, 1}, object),
            "Area Zero Sky"
        ));
        m_center_x = object.center_of_gravity_x() / frame.width();
        m_center_y = object.center_of_gravity_y() / frame.height();
    }else{
        m_box.reset();
    }
    return false;
}





enum class OverworldState{
    None,
    FindingSky,
    TurningLeft,
    TurningRight,
};
void find_and_center_on_sky(
    ProgramEnvironment& env, VideoStream& stream, ProControllerContext& context
){
    context.wait_for_all_requests();
    stream.log("Looking for the sky...");

    AreaZeroSkyTracker sky_tracker(stream.overlay());
    InferenceSession inference_session(
        context, stream,
        {sky_tracker}
    );

    AsyncCommandSession<ProController> session(
        context, stream.logger(),
        env.realtime_dispatcher(),
        context.controller()
    );
    OverworldState state = OverworldState::None;
    WallClock start = current_time();
    while (true){
        if (current_time() - start > std::chrono::minutes(1)){
            OperationFailedException::fire(
                ErrorReport::NO_ERROR_REPORT,
                "Failed to find the sky after 1 minute. (state = " + std::to_string((int)state) + ")",
                stream
            );
        }

        context.wait_for(std::chrono::milliseconds(200));

        if (!session.command_is_running()){
            state = OverworldState::None;
        }

        double sky_x, sky_y;
        bool sky = sky_tracker.sky_location(sky_x, sky_y);

        if (!sky){
            if (state != OverworldState::FindingSky){
                stream.log("Sky not detected. Attempting to find the sky...", COLOR_ORANGE);
                session.dispatch([](ProControllerContext& context){
                    pbf_move_right_joystick(context, 128, 0, 250, 0);
                    pbf_move_right_joystick(context, 0, 0, 10 * TICKS_PER_SECOND, 0);
                });
                state = OverworldState::FindingSky;
            }
            continue;
        }

//        cout << sky_x << " - " << sky_y << endl;

        if (sky_x < 0.45){
            if (state != OverworldState::TurningLeft){
                stream.log("Centering the sky... Moving left.");
                uint8_t magnitude = (uint8_t)((0.5 - sky_x) * 96 + 31);
                uint16_t duration = (uint16_t)((0.5 - sky_x) * 125 + 20);
                session.dispatch([=](ProControllerContext& context){
                    pbf_move_right_joystick(context, 128 - magnitude, 128, duration, 0);
                });
                state = OverworldState::TurningLeft;
            }
            continue;
        }
        if (sky_x > 0.55){
            if (state != OverworldState::TurningRight){
                stream.log("Centering the sky... Moving Right.");
                uint8_t magnitude = (uint8_t)((sky_x - 0.5) * 96 + 31);
                uint16_t duration = (uint16_t)((sky_x - 0.5) * 125 + 20);
                session.dispatch([=](ProControllerContext& context){
                    pbf_move_right_joystick(context, 128 + magnitude, 128, duration, 0);
                });
                state = OverworldState::TurningRight;
            }
            continue;
        }

        stream.log("Found the sky!", COLOR_ORANGE);
        break;
    }

    session.stop_session_and_rethrow();
}













}
}
}
