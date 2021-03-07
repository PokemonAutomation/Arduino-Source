/*  Test Program
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/SwitchFramework/FrameworkSettings.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "CommonFramework/Inference/ImageTools.h"
#include "PokemonSwSh/Inference/PokemonSwSh_RaidLobbyReader.h"
#include "TestProgram.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

TestProgram::TestProgram()
    : SingleSwitchProgram(
        FeedbackType::REQUIRED, PABotBaseLevel::PABOTBASE_12KB,
        "Test Program",
        "",
        "Test Program"
    )
{}





void test_inference0(VideoFeed& feed, const InferenceBox& box){
//    InferenceBoxScope box(feed, Qt::red, 0.5, 0.5, 0.1, 0.1);

    QImage image = feed.snapshot();
    QImage cropped = extract_box(image, box);
    cropped.save("f:/test.jpg");

    FloatPixel average = pixel_average(cropped);
    cout << average.r << " " << average.g << " " << average.b << endl;
}


void test_inference(VideoFeed& feed, const InferenceBox& box){

}






void TestProgram::program(SingleSwitchProgramEnvironment& env) const{

    BotBase& botbase = env.console;
    VideoFeed& feed = env.console;

//    BeamReader reader(feed, env.logger);
//    if (!reader.run(env, botbase, 3 * TICKS_PER_SECOND)){
//        pbf_press_button(botbase, BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
//    }
}







}
}
}




