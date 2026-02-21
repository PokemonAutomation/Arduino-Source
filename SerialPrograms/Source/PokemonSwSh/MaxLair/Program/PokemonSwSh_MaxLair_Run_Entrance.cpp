/*  Max Lair Run Entrance
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PokemonReader.h"
#include "PokemonSwSh_MaxLair_Run_Entrance.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{

using namespace Pokemon;

namespace {
std::string read_saved_name(
    Logger& logger,
    Language language,
    const ImageViewRGB32& image,
    const ImageFloatBox& box
                            ) {
    auto cropped = extract_box_reference(image, box);
    OCR::StringMatchResult result = PokemonNameReader::instance().read_substring(
            logger, language, cropped, OCR::BLACK_OR_WHITE_TEXT_FILTERS(),
0.01, 0.50, 2.0
                                                    );
    if (result.results.empty()) {
        return "";
    }
    
    return result.results.begin()->second.token;
}

std::vector<std::string> read_saved_paths(
    VideoStream& stream,
    Language language
) {
    auto snapshot = stream.video().snapshot();
    if (!snapshot) return { "", "", "" };

    const ImageRGB32* img_ptr = snapshot.get();
    const ImageViewRGB32& screen = *img_ptr;
    
    const double width = snapshot->width();
    const double height = snapshot->height();

    // To be adjusted
    const ImageFloatBox name_region(0.30, 0.75, 0.40, 0.05);

    std::vector<std::string> slugs;
    for (int i = 0; i < 3; i++) {
        ImageFloatBox box(
            name_region.x + i * (name_region.width / 3.0),
            name_region.y,
            name_region.width / 3.0,
            name_region.height
        );
        std::string slug = read_saved_name(stream.logger(), language, screen, box);
        slugs.push_back(slug);
    }
    return slugs;
}
}


// Read the three currently saved paths (if any) from the entrance screen.
// Returns a vector of three slugs (empty strings for empty slots).


void run_entrance(
    AdventureRuntime& runtime,
    ProgramEnvironment& env, size_t console_index,
    VideoStream& stream, ProControllerContext& context,
    const std::string& boss_slug,
    const EndBattleDecider& decider,
    GlobalStateTracker& state_tracker
){
    GlobalState& state = state_tracker[console_index];

    if (!state.adventure_started){
        stream.log("Failed to start raid.", COLOR_RED);
        runtime.session_stats.add_error();
    }else if (state.wins == 0){
        stream.log("Lost on first raid.", COLOR_PURPLE);
        runtime.session_stats.add_run(0);
        if (console_index == runtime.host_index){
            runtime.path_stats.clear();
        }
    }
    
    // Read the three currently saved paths (if any)
    Language language = runtime.console_settings[console_index].language;
    std::vector<std::string> saved = read_saved_paths(stream, language);
    
    // Determine whether we should save this boss
    bool should_save = false;
    if (!boss_slug.empty()) {
        // Check if already saved
        bool already_saved = false;
        for (const auto& s : saved) {
            if (s == boss_slug) {
                already_saved = true;
                break;
            }
        }
        // Count non‑empty slots
        size_t occupied = 0;
        for (const auto& s : saved) {
            if (!s.empty()) ++occupied;
        }
        if (!already_saved && occupied < 3) {
            should_save = decider.save_path(boss_slug);
        }
    }


    OverlayBoxScope box(stream.overlay(), {0.782, 0.850, 0.030, 0.050});

    pbf_wait(context, 2000ms);
    while (true){
        if (should_save){
            pbf_press_button(context, BUTTON_A, 160ms, 1000ms);
        }else{
            pbf_press_button(context, BUTTON_B, 160ms, 1000ms);
        }
        context.wait_for_all_requests();

        VideoSnapshot screen_snap = stream.video().snapshot();
        const ImageRGB32* img_ptr = screen_snap.get();
        ImageStats stats = image_stats(extract_box_reference(*img_ptr, box));
        if (!is_grey(stats, 400, 1000)){
            break;
        }
    }
}



}
}
}
}
