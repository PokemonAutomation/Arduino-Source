/*  Arc Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_ArcDetector_H
#define PokemonAutomation_PokemonLA_ArcDetector_H

#include <list>
#include "PokemonLA_WhiteObjectDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class ArcDetector : public WhiteObjectDetector{
public:
    ArcDetector();
    virtual void process_object(const QImage& image, const WaterfillObject& object) override;
    virtual void finish() override;

private:
    std::list<ImagePixelBox> m_left;
    std::list<ImagePixelBox> m_right;
};



}
}
}
#endif
