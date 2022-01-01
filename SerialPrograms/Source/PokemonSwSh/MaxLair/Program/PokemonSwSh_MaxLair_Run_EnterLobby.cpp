/*  Max Lair Enter Lobby
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Compiler.h"
#include "Common/Cpp/Exception.h"
#include "CommonFramework/Tools/VideoOverlaySet.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/OCR/OCR_RawOCR.h"
#include "CommonFramework/OCR/OCR_Filtering.h"
#include "CommonFramework/OCR/OCR_NumberReader.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSwSh/Inference/PokemonSwSh_SelectionArrowFinder.h"
#include "PokemonSwSh/Programs/PokemonSwSh_Internet.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_StateMachine.h"
#include "PokemonSwSh_MaxLair_Run_EnterLobby.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{



class GreyDialogDetector : public VisualInferenceCallback{
public:
    GreyDialogDetector()
        : m_box0(0.180, 0.815, 0.015, 0.030)
        , m_box1(0.785, 0.840, 0.030, 0.050)
    {}

    bool detect(const QImage& screen){
        ImageStats stats0 = image_stats(extract_box(screen, m_box0));
        if (!is_grey(stats0, 0, 200)){
            return false;
        }
        ImageStats stats1 = image_stats(extract_box(screen, m_box1));
        if (!is_grey(stats1, 0, 200)){
            return false;
        }
        return true;
    }
    virtual void make_overlays(VideoOverlaySet& items) const override{
        items.add(COLOR_RED, m_box0);
        items.add(COLOR_RED, m_box1);
    }
    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override{
        return detect(frame);
    }

private:
    ImageFloatBox m_box0;
    ImageFloatBox m_box1;
};


class OreReader{
public:
    OreReader(Logger& logger, VideoOverlay& overlay)
        : m_logger(logger)
        , m_box(overlay, 0.945, 0.010, 0.0525, 0.050)
    {}

    int read(const QImage& screen){
        QImage image = extract_box(screen, m_box);
        OCR::TextImageFilter{false, 600}.apply(image);
        return OCR::read_number(m_logger, image);
    }

private:
    Logger& m_logger;
    InferenceBoxScope m_box;
};



QImage enter_lobby(
    ProgramEnvironment& env, ConsoleHandle& console,
    size_t boss_slot, bool connect_to_internet,
    ReadableQuantity999& ore
){
    pbf_mash_button(console, BUTTON_B, 2 * TICKS_PER_SECOND);

    if (connect_to_internet){
        connect_to_internet_with_inference(env, console);
    }

    VideoOverlaySet boxes(console);
    SelectionArrowFinder arrow_detector(console, ImageFloatBox(0.350, 0.450, 0.500, 0.400));
    GreyDialogDetector dialog_detector;
    arrow_detector.make_overlays(boxes);
    dialog_detector.make_overlays(boxes);

    InferenceBoxScope ore_box(console, 0.900, 0.015, 0.020, 0.040);
    InferenceBoxScope ore_quantity(console, 0.945, 0.010, 0.0525, 0.050);

    size_t presses = 0;
    size_t arrow_count = 0;
    size_t ore_dialog_count = 0;
    while (presses < 50){
        presses++;
        pbf_press_button(console, BUTTON_A, 10, TICKS_PER_SECOND);
        console.botbase().wait_for_all_requests();

        QImage screen = console.video().snapshot();
        if (!arrow_detector.detect(screen)){
            continue;
        }

        arrow_count++;

        console.log("Detected dialog prompt.");
//        screen.save("test.png");

        //  We need to pay ore.
        ImageStats ore_stats = image_stats(extract_box(screen, ore_box));
        if (is_solid(ore_stats, {0.594724, 0.405276, 0.})){
            console.log("Need to pay ore.", COLOR_PURPLE);

            arrow_count = 0;
            QImage image = extract_box(screen, ore_quantity);
            OCR::TextImageFilter{false, 600}.apply(image);
            ore.update_with_ocr(console.logger(), image);

            if (ore.quantity < 20){
                PA_THROW_StringException("You have less than 20 ore. Program stopped. (Quantity: " + ore.to_str() + ")");
            }

            ore_dialog_count++;
            if (ore_dialog_count >= 2){
                PA_THROW_StringException("Unable to start adventure. Are you out of ore? (Quantity: " + ore.to_str() + ")");
            }

            continue;
        }

        //  Detected save dialog.
        if (dialog_detector.detect(screen)){
            console.log("Detected save dialog.");
            console.botbase().wait_for_all_requests();
            QImage entrance = console.video().snapshot();
            pbf_press_button(console, BUTTON_A, 10, 5 * TICKS_PER_SECOND);
            console.botbase().wait_for_all_requests();
            return entrance;
        }

        //  Select a boss.
        if (arrow_count == 2){
            console.log("Detected boss selection.");
            if (boss_slot > 0){
                for (size_t c = 1; c < boss_slot; c++){
                    pbf_press_dpad(console, DPAD_DOWN, 10, 50);
                }
                pbf_press_button(console, BUTTON_A, 10, TICKS_PER_SECOND);
            }else{
                pbf_press_button(console, BUTTON_B, 10, TICKS_PER_SECOND);
            }
        }
    }

    return QImage();
}



}
}
}
}
