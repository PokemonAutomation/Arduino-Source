/*  Connect to Integer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Battles/PokemonSV_NormalBattleMenus.h"
#include "PokemonSV/Inference/PokemonSV_MainMenuDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV_ConnectToInternet.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



class NewsDetector : public StaticScreenDetector{
public:
    NewsDetector(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

private:
    Color m_color;
    ImageFloatBox m_bottom_white;
    ImageFloatBox m_bottom_buttons;
};
NewsDetector::NewsDetector(Color color)
    : m_color(color)
    , m_bottom_white(0.15, 0.92, 0.20, 0.06)
    , m_bottom_buttons(0.40, 0.92, 0.58, 0.06)
{}
void NewsDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_bottom_white);
    items.add(m_color, m_bottom_buttons);
}
bool NewsDetector::detect(const ImageViewRGB32& screen) const{
    ImageStats bottom_white = image_stats(extract_box_reference(screen, m_bottom_white));
    if (!is_white(bottom_white)){
        return false;
    }

    ImageStats bottom_buttons = image_stats(extract_box_reference(screen, m_bottom_buttons));
//    cout << bottom_buttons.average << bottom_buttons.stddev << endl;
    if (bottom_buttons.stddev.sum() < 100){
        return false;
    }

    return true;
}
class NewsWatcher : public DetectorToFinder<NewsDetector>{
public:
    NewsWatcher(Color color = COLOR_RED)
         : DetectorToFinder("NewsWatcher", std::chrono::milliseconds(1000), color)
    {}
};




void connect_to_internet_from_menu(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    WallClock start = current_time();
    bool connected = false;
    while (true){
        if (current_time() - start > std::chrono::minutes(5)){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "connect_to_internet_from_menu(): Failed to connect to internet after 5 minutes.",
                true
            );
        }

        OverworldWatcher overworld(COLOR_RED);
        MainMenuWatcher main_menu(COLOR_YELLOW);
        AdvanceDialogWatcher dialog(COLOR_GREEN);
        PromptDialogWatcher prompt(COLOR_CYAN);
        NewsWatcher news(COLOR_BLUE);
        NormalBattleMenuWatcher battle_menu(COLOR_MAGENTA);
        context.wait_for_all_requests();
        int ret = wait_until(
            console, context,
            std::chrono::seconds(60),
            {overworld, main_menu, dialog, prompt, news, battle_menu}
        );
        context.wait_for(std::chrono::milliseconds(100));
        switch (ret){
        case 0:
            console.log("Detected overworld. (unexpected)", COLOR_RED);
            pbf_press_button(context, BUTTON_X, 20, 105);
            continue;
        case 1:
            console.log("Detected main menu.");
            if (connected){
                return;
            }else{
                pbf_press_button(context, BUTTON_L, 20, 105);
            }
            continue;
        case 2:
            console.log("Detected dialog.");
            connected = true;
            pbf_press_button(context, BUTTON_B, 20, 105);
            continue;
        case 3:
            console.log("Already connected to internet.");
            connected = true;
            pbf_press_button(context, BUTTON_B, 20, 105);
            continue;
        case 4:
            console.log("Detected news menu...");
            connected = true;
            pbf_press_button(context, BUTTON_B, 20, 105);
            continue;
        case 5:
            console.log("Detected battle menu...");
            throw OperationFailedException(
                ErrorReport::NO_ERROR_REPORT, console,
                "connect_to_internet_from_menu(): Looks like you got attacked.",
                true
            );
        default:
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "connect_to_internet_from_menu(): No recognized state after 60 seconds.",
                true
            );
        }
    }
}
void connect_to_internet_from_overworld(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    WallClock start = current_time();
    bool connected = false;
    while (true){
        if (current_time() - start > std::chrono::minutes(5)){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "connect_to_internet_from_overworld(): Failed to connect to internet after 5 minutes.",
                true
            );
        }

        OverworldWatcher overworld(COLOR_RED);
        MainMenuWatcher main_menu(COLOR_YELLOW);
        AdvanceDialogWatcher dialog(COLOR_GREEN);
        PromptDialogWatcher prompt(COLOR_CYAN);
        NewsWatcher news(COLOR_BLUE);
        NormalBattleMenuWatcher battle_menu(COLOR_MAGENTA);
        context.wait_for_all_requests();
        int ret = wait_until(
            console, context,
            std::chrono::seconds(60),
            {overworld, main_menu, dialog, prompt, news, battle_menu}
        );
        context.wait_for(std::chrono::milliseconds(100));
        switch (ret){
        case 0:
            console.log("Detected overworld.");
            if (connected){
                return;
            }else{
                pbf_press_button(context, BUTTON_X, 20, 105);
                continue;
            }
        case 1:
            console.log("Detected main menu.");
            if (connected){
                pbf_press_button(context, BUTTON_B, 20, 105);
            }else{
                pbf_press_button(context, BUTTON_L, 20, 105);
            }
            continue;
        case 2:
            console.log("Detected dialog.");
            connected = true;
            pbf_press_button(context, BUTTON_B, 20, 105);
            continue;
        case 3:
            console.log("Already connected to internet.");
            connected = true;
            pbf_press_button(context, BUTTON_B, 20, 105);
            continue;
        case 4:
            console.log("Detected news menu...");
            connected = true;
            pbf_press_button(context, BUTTON_B, 20, 105);
            continue;
        case 5:
            console.log("Detected battle menu...");
            throw OperationFailedException(
                ErrorReport::NO_ERROR_REPORT, console,
                "connect_to_internet_from_overworld(): Looks like you got attacked.",
                true
            );
        default:
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "connect_to_internet_from_overworld(): No recognized state after 60 seconds.",
                true
            );
        }
    }
}




}
}
}
