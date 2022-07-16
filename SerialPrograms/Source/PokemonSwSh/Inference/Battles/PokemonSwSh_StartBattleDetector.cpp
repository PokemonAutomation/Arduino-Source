/*  Start Battle Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QImage>
#include "Common/Compiler.h"
#include "CommonFramework/ImageTypes/RGB32ImageView.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "PokemonSwSh_StartBattleDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



StartBattleWatcher::StartBattleWatcher(Color color)
    : VisualInferenceCallback("StartBattleWatcher")
    , m_color(color)
    , m_screen_box(0.2, 0.2, 0.6, 0.6)
{}
void StartBattleWatcher::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_screen_box);
}
bool StartBattleWatcher::process_frame(const QImage& frame, WallClock timestamp){
    ConstImageRef image = extract_box_reference(frame, m_screen_box);
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
