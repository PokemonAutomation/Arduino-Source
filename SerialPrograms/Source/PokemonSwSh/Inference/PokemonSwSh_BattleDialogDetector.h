/*  Battle Dialog Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_BattleDialogDetector_H
#define PokemonAutomation_PokemonSwSh_BattleDialogDetector_H

#include "CommonFramework/Tools/VideoFeed.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class BattleDialogDetector{
public:
    BattleDialogDetector(VideoFeed& feed);

    bool detect(const QImage& screen) const;


private:
    InferenceBoxScope m_bottom;
    InferenceBoxScope m_left;
    InferenceBoxScope m_right;
};


}
}
}
#endif
