/*  Orbeetle Attack Animation Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *
 *      Detects whether Orbeetle's animation is special or physical.
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/Tools/DebugDumper.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSwSh_OrbeetleAttackAnimationDetector.h"


namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

OrbeetleAttackAnimationDetector::OrbeetleAttackAnimationDetector(VideoStream& stream, ProControllerContext& context)
    : m_stream(stream)
    , m_context(context)
    , m_box(stream.overlay(), {0.86, 0.2, 0.1, 0.15})
{}


OrbeetleAttackAnimationDetector::Detection OrbeetleAttackAnimationDetector::run(bool save_screenshot, bool log_values)
{
    //  Grab baseline image.
    VideoSnapshot baseline_image = m_stream.video().snapshot();
    if (!baseline_image){
        m_stream.log("Orbeetle Attack Animation: Screenshot failed.", COLOR_PURPLE);
        return Detection::NO_DETECTION;
    }

    if (save_screenshot){
        //baseline_image->save("orbeetle-baseline-" + now_to_filestring() + ".png");
        dump_debug_image(m_stream.logger(), "rng", "orbeetle-baseline", baseline_image);
    }

    FloatPixel baseline_values = image_average(extract_box_reference(baseline_image, m_box));
    FloatPixel baseline_ratios = baseline_values / baseline_values.sum();


    //  Play the attack animation.
    pbf_press_button(m_context, BUTTON_RCLICK, 10, 155);
    m_context.wait_for_all_requests();


    //  Grab the animation image.
    VideoSnapshot animation_image = m_stream.video().snapshot();
    if (!animation_image){
        m_stream.log("Orbeetle Attack Animation: Screenshot failed.", COLOR_PURPLE);
        return Detection::NO_DETECTION;
    }

    FloatPixel animation_values = image_average(extract_box_reference(animation_image, m_box));
    FloatPixel animation_ratios = animation_values / animation_values.sum();

    if (log_values){
        m_stream.log("Orbeetle baseline value red: " + std::to_string(baseline_values.r));
        m_stream.log("Orbeetle attack value red: " + std::to_string(animation_values.r));
        m_stream.log("Orbeetle baseline value green: " + std::to_string(baseline_values.g));
        m_stream.log("Orbeetle attack value green: " + std::to_string(animation_values.g));
        m_stream.log("Orbeetle baseline value blue: " + std::to_string(baseline_values.b));
        m_stream.log("Orbeetle attack value blue: " + std::to_string(animation_values.b));

        m_stream.log("Orbeetle baseline ratio red: " + std::to_string(baseline_ratios.r));
        m_stream.log("Orbeetle attack ratio red: " + std::to_string(animation_ratios.r));
        m_stream.log("Orbeetle baseline ratio green: " + std::to_string(baseline_ratios.g));
        m_stream.log("Orbeetle attack ratio green: " + std::to_string(animation_ratios.g));
        m_stream.log("Orbeetle baseline ratio blue: " + std::to_string(baseline_ratios.b));
        m_stream.log("Orbeetle attack ratio blue: " + std::to_string(animation_ratios.b));
    }

    if ((animation_ratios.r >= 1.4 * baseline_ratios.r)
//        && (animation_ratios.g <= 0.85 * baseline_ratios.g)
//        && (animation_ratios.b <= 0.85 * baseline_ratios.b)
        )
    {
        if (save_screenshot){
            //animation_image->save("orbeetle-attack-special-" + now_to_filestring() + ".png");
            dump_debug_image(m_stream.logger(), "rng", "orbeetle-special", animation_image);
        }
        m_stream.log("Orbeetle Attack Animation: Special animation detected.");
        return Detection::SPECIAL;
    }
    if (save_screenshot){
        animation_image->save("orbeetle-attack-physical-" + now_to_filestring() + ".png");
        dump_debug_image(m_stream.logger(), "rng", "orbeetle-physical", animation_image);
    }
    m_stream.log("Orbeetle Attack Animation: Physical animation detected.");
    return Detection::PHYSICAL;
}



}
}
}
