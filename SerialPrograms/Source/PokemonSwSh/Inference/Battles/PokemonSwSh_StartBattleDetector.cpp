/*  Start Battle Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Compiler.h"
#include "CommonFramework/Inference/ImageTools.h"
#include "PokemonSwSh_StartBattleDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



StartBattleWatcher::StartBattleWatcher(Color color)
    : m_color(color)
    , m_screen_box(0.2, 0.2, 0.6, 0.6)
{}
void StartBattleWatcher::make_overlays(OverlaySet& items) const{
    items.add(m_color, m_screen_box);
}
bool StartBattleWatcher::process_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point timestamp
){
    QImage image = extract_box(frame, m_screen_box);

    ImageStats stats = image_stats(image);

    //  White screen.
    if (stats.average.sum() > 600 && stats.stddev.sum() < 10){
        return true;
    }

    //  Grey text box.
    bool dialog = stats.stddev.sum() > 50;
    dialog &= m_dialog.detect(frame);
    if (dialog){
//        cout << stats0.stddev.sum() << endl;
    }
    return dialog;
}



}
}
}
