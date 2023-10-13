/*  Video Fast Code Entry
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/CancellableScope.h"
#include "Common/Cpp/Concurrency/AsyncDispatcher.h"
//#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ImageTools/ImageFilter.h"
#include "CommonFramework/ImageMatch/ImageDiff.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/OCR/OCR_RawOCR.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/Inference/Tera/PokemonSV_TeraCodeReader.h"
#include "PokemonSV_CodeEntry.h"
#include "PokemonSV_VideoFastCodeEntry.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;


VideoFceSettings::VideoFceSettings()
    : GroupOption("Join Settings (V-FCE)", LockMode::UNLOCK_WHILE_RUNNING)
    , OCR_METHOD(
        "<b>Text Recognition Method:</b><br>"
        "Each text recognition method has its own strengths and weaknesses. This option lets you choose which method to use.",
        {
            {VideoFceOcrMethod::RAW_OCR,    "raw-ocr",          "Raw OCR. No image pre-processing."},
            {VideoFceOcrMethod::BLACK_TEXT, "black-on-white",   "Filter: Black Text on White Background"},
            {VideoFceOcrMethod::WHITE_TEXT, "white-on-black",   "Filter: White Text on Black Background"},
            {VideoFceOcrMethod::TERA_CARD,  "tera-card",        "Tera Card"},
        },
        LockMode::UNLOCK_WHILE_RUNNING,
        VideoFceOcrMethod::TERA_CARD
    )
    , SKIP_INITIAL_CODE(
        "<b>Skip Initial Code:</b><br>"
        "If there is already a code up when you start the program, skip it since it's from the last raid.",
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
{
    PA_ADD_OPTION(OCR_METHOD);
    PA_ADD_OPTION(SKIP_INITIAL_CODE);
}



void wait_for_video_code_and_join(
    MultiSwitchProgramEnvironment& env, CancellableScope& scope,
    ScreenWatchOption& screen_watcher,
    VideoFceSettings& join_method,
    FastCodeEntrySettingsOption& fce_settings
){
    bool skip_initial = join_method.SKIP_INITIAL_CODE;
    VideoSnapshot snapshot;
    while (true){
        scope.throw_if_cancelled();
        VideoSnapshot current = screen_watcher.screenshot();

//        env.log("start");

        if (snapshot && current &&
            snapshot->width() == current->width() &&
            snapshot->height() == current->height()
        ){
            double rmsd = ImageMatch::pixel_RMSD(snapshot, current);
            if (rmsd < 2){
                scope.wait_for(std::chrono::milliseconds(1));
                continue;
            }
        }
        snapshot = std::move(current);
//        env.log("done new frame check");

        if (skip_initial){
            env.log("Waiting for next screen change...");
            skip_initial = false;
            continue;
        }

        std::string code;
        switch (join_method.OCR_METHOD){
        case VideoFceOcrMethod::RAW_OCR:
            code = OCR::ocr_read(Language::English, snapshot);
            env.log("OCR: " + code);
            break;
        case VideoFceOcrMethod::BLACK_TEXT:{
            ImageRGB32 filtered = to_blackwhite_rgb32_range(snapshot, 0xff000000, 0xff7f7f7f, true);
            code = OCR::ocr_read(Language::English, filtered);
            env.log("OCR: " + code);
            break;
        }
        case VideoFceOcrMethod::WHITE_TEXT:{
            ImageRGB32 filtered = to_blackwhite_rgb32_range(snapshot, 0xffc0c0c0, 0xffffffff, true);
            code = OCR::ocr_read(Language::English, filtered);
            env.log("OCR: " + code);
            break;
        }
        case VideoFceOcrMethod::TERA_CARD:
            code = read_raid_code(env.logger(), env.realtime_dispatcher(), snapshot);
        }
        const char* error = enter_code(env, scope, fce_settings, code, false);
        if (error == nullptr){
            break;
        }else{
            env.log(std::string("Invalid Code: ") + error, COLOR_RED);
        }
    }
}





VideoFastCodeEntry_Descriptor::VideoFastCodeEntry_Descriptor()
    : MultiSwitchProgramDescriptor(
        "PokemonSV:VideoFastCodeEntry",
        STRING_POKEMON + " SV", "Video Fast Code Entry (V-FCE)",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/VideoFastCodeEntry.md",
        "Read a 4, 6, or 8 digit link code from someone on your screen and enter it as quickly as possible.",
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB,
        1, 4, 1
    )
{}

VideoFastCodeEntry::VideoFastCodeEntry()
    : SCREEN_WATCHER("Capture Box:")
    , MODE(
        "<b>Mode:</b>",
        {
            {Mode::MANUAL,      "manual",   "Manual - Enter code when you start the program."},
            {Mode::AUTOMATIC,   "auto",     "Automatic - Monitor the region. Automatically enter code when it appears."},
        },
        LockMode::LOCK_WHILE_RUNNING,
        Mode::MANUAL
    )
    , SKIP_CONNECT_TO_CONTROLLER(
        "<b>Skip Connect to Controller:</b><br>"
        "If you know your controllers are already connected, you can skip this to save 64 milliseconds. (only applies to manual mode)",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
    , FCE_SETTINGS(LockMode::LOCK_WHILE_RUNNING)
    , NOTIFICATIONS({
        &NOTIFICATION_PROGRAM_FINISH,
    })
{
    PA_ADD_OPTION(SCREEN_WATCHER);
    PA_ADD_OPTION(MODE);
    PA_ADD_OPTION(SKIP_CONNECT_TO_CONTROLLER);
    PA_ADD_OPTION(JOIN_METHOD);
    PA_ADD_OPTION(FCE_SETTINGS);
    PA_ADD_OPTION(NOTIFICATIONS);

    //  Preload the OCR data.
    OCR::ensure_instances(Language::English, 6);
    preload_code_templates();
}


void VideoFastCodeEntry::program(MultiSwitchProgramEnvironment& env, CancellableScope& scope){
    FastCodeEntrySettings settings(FCE_SETTINGS);

    if (MODE == Mode::MANUAL){
        std::string code = read_raid_code(env.logger(), env.realtime_dispatcher(), SCREEN_WATCHER.screenshot());
        const char* error = enter_code(env, scope, settings, code, !SKIP_CONNECT_TO_CONTROLLER);
        if (error){
            env.log("No valid code found: " + std::string(error), COLOR_RED);
        }
        return;
    }

    //  Connect the controller.
    env.run_in_parallel(scope, [&](ConsoleHandle& console, BotBaseContext& context){
        pbf_press_button(context, BUTTON_PLUS, 5, 3);
    });

    //  Preload 6 threads to OCR the code.
    env.realtime_dispatcher().ensure_threads(6);

    wait_for_video_code_and_join(env, scope, SCREEN_WATCHER, JOIN_METHOD, FCE_SETTINGS);

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}





}
}
}
