/*  Flag Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_FlagDetector_H
#define PokemonAutomation_PokemonLA_FlagDetector_H

#include <list>
#include "PokemonLA_WhiteObjectDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class FlagDetector : public WhiteObjectDetector{
public:
    FlagDetector();
    virtual void process_object(const ConstImageRef& image, const WaterfillObject& object) override;
    virtual void finish() override;

private:
    std::list<ImagePixelBox> m_left;
    std::list<ImagePixelBox> m_right;
};



int read_flag_distance(const QImage& screen, double flag_x, double flag_y);




}
}
}
#endif
