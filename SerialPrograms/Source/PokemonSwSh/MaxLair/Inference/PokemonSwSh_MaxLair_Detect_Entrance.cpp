/*  Max Lair Detect Entrance
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Compiler.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageMatch/ImageDiff.h"
#include "PokemonSwSh_MaxLair_Detect_Entrance.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


EntranceDetector::EntranceDetector(const QImage& entrance_screen)
    : m_box0(0.020, 0.020, 0.500, 0.750)
    , m_entrance_screen(extract_box(entrance_screen, m_box0))
{
    add_box(m_box0, Qt::darkGreen);
}
bool EntranceDetector::process_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point timestamp
){
    return detect(frame);
}

bool EntranceDetector::detect(const QImage& screen){
    if (screen.isNull()){
        return false;
    }

    QImage image = extract_box(screen, m_box0);
    if (image.size() != m_entrance_screen.size()){
        image = image.scaled(m_entrance_screen.size());
    }

    double diff = ImageMatch::pixel_RMSD(m_entrance_screen, image);
//    cout << diff << endl;

    return diff < 20;
}



}
}
}
}
