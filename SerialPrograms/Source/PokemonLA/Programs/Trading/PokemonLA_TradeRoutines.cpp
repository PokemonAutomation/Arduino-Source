/*  Trade Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"
#include "CommonFramework/Inference/ImageMatchDetector.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonLA/Inference/Objects/PokemonLA_ButtonDetector.h"
#include "PokemonLA_TradeRoutines.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{



TradeStats::TradeStats()
    : m_trades(m_stats["Trades"])
    , m_errors(m_stats["Errors"])
{
    m_display_order.emplace_back("Trades");
    m_display_order.emplace_back("Errors", true);
}



void trade_current_pokemon(
    ConsoleHandle& console, BotBaseContext& context,
    MultiConsoleErrorState& tracker,
    TradeStats& stats
){
    tracker.check_unrecoverable_error(console);

    context.wait_for_all_requests();
    std::shared_ptr<const ImageRGB32> box_image = console.video().snapshot();
    ImageMatchWatcher box_detector(box_image, {0.02, 0.15, 0.15, 0.80}, 50);

    {
        pbf_press_button(context, BUTTON_A, 20, 0);
        context.wait_for_all_requests();
        ButtonDetector detector(
            console, console,
            ButtonType::ButtonA, ImageFloatBox(0.25, 0.15, 0.50, 0.75),
            std::chrono::milliseconds(0), true
        );
        int ret = wait_until(
            console, context, std::chrono::seconds(120),
            {{detector}}
        );
        if (ret < 0){
            stats.m_errors++;
            tracker.report_unrecoverable_error(console, "Failed to detect trade select prompt after 2 minutes.");
        }
        console.log("Detected trade prompt.");
        context.wait_for(std::chrono::milliseconds(100));
        tracker.check_unrecoverable_error(console);
    }
    {
        pbf_press_button(context, BUTTON_A, 20, 105);
        context.wait_for_all_requests();
        ButtonDetector detector(
            console, console,
            ButtonType::ButtonA, ImageFloatBox(0.50, 0.52, 0.40, 0.10),
            std::chrono::milliseconds(0), true
        );
        int ret = wait_until(
            console, context, std::chrono::seconds(10),
            {{detector}}
        );
        if (ret < 0){
            stats.m_errors++;
            tracker.report_unrecoverable_error(console, "Failed to detect trade confirm prompt after 10 seconds.");
        }
        console.log("Detected trade confirm prompt.");
        context.wait_for(std::chrono::milliseconds(100));
        tracker.check_unrecoverable_error(console);
    }

    //  Start trade.
    pbf_press_button(context, BUTTON_A, 20, 0);

    //  Wait for black screen.
    {
        BlackScreenOverWatcher black_screen;
        int ret = wait_until(
            console, context, std::chrono::minutes(2),
            {{black_screen}}
        );
        if (ret < 0){
            stats.m_errors++;
            tracker.report_unrecoverable_error(console, "Failed to detect start of trade after 2 minutes.");
        }
        console.log("Detected start of trade.");
        context.wait_for(std::chrono::milliseconds(100));
        tracker.check_unrecoverable_error(console);
    }

    //  Mash B until 2nd black screen.
    {
        BlackScreenWatcher black_screen;
        int ret = run_until(
            console, context,
            [](BotBaseContext& context){
                pbf_mash_button(context, BUTTON_B, 120 * TICKS_PER_SECOND);
            },
            {{black_screen}}
        );
        if (ret < 0){
            stats.m_errors++;
            tracker.report_unrecoverable_error(console, "Failed to detect end of trade after 2 minutes.");
        }
        console.log("Detected end of trade.");
        context.wait_for(std::chrono::milliseconds(100));
        tracker.check_unrecoverable_error(console);
    }

    //  Wait to return to box.
    {
        int ret = wait_until(
            console, context, std::chrono::minutes(2),
            {{box_detector}}
        );
        if (ret < 0){
            stats.m_errors++;
            tracker.report_unrecoverable_error(console, "Failed to return to box after 2 minutes after a trade.");
        }
        console.log("Detected box. Trade completed.");
        tracker.check_unrecoverable_error(console);
    }
}





TradeNameReader::TradeNameReader(LoggerQt& logger, VideoOverlay& overlay, Language language)
    : m_logger(logger)
    , m_language(language)
    , m_box(overlay, 0.80, 0.155, 0.18, 0.05)
{
    std::string path = RESOURCE_PATH() + "Pokemon/Pokedex/Pokedex-Hisui.json";
    JsonValue json = load_json_file(path);
    JsonArray& array = json.get_array_throw(path);
    for (auto& item : array){
        m_slugs.insert(std::move(item.get_string_throw(path)));
    }
}

std::string TradeNameReader::read(const ImageViewRGB32& screen) const{
    ImageViewRGB32 image = extract_box_reference(screen, m_box);
    OCR::StringMatchResult result = Pokemon::PokemonNameReader::instance().read_substring(
        m_logger, m_language, image,
        OCR::WHITE_TEXT_FILTERS()
    );
    result.clear_beyond_log10p(Pokemon::PokemonNameReader::MAX_LOG10P);

    for (auto iter = result.results.begin(); iter != result.results.end();){
        //  Remove from candidates if it's not in PLA.
        if (m_slugs.find(iter->second.token) == m_slugs.end()){
            iter = result.results.erase(iter);
        }else{
            ++iter;
        }
    }

    if (result.results.size() != 1){
        dump_image(m_logger, ProgramInfo(), "TradeNameReader", screen);
        return "";
    }

    return std::move(result.results.begin()->second.token);
}




























}
}
}
