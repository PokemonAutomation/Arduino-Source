/*  Visual Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_VisualDetector_H
#define PokemonAutomation_CommonFramework_VisualDetector_H

namespace PokemonAutomation{

class ImageViewRGB32;
class VideoOverlaySet;


class StaticScreenDetector{
public:
    virtual void make_overlays(VideoOverlaySet& items) const = 0;
    virtual bool detect(const ImageViewRGB32& screen) const = 0;
};




}
#endif
