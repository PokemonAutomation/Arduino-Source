/*  Visual Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_VisualDetector_H
#define PokemonAutomation_CommonFramework_VisualDetector_H

#include <vector>
#include <deque>
#include <QImage>
#include "CommonFramework/Tools/VideoFeed.h"

namespace PokemonAutomation{


class StaticScreenDetector{
public:
    virtual void make_overlays(OverlaySet& items) const = 0;
    virtual bool detect(const QImage& screen) const = 0;
};




}
#endif
