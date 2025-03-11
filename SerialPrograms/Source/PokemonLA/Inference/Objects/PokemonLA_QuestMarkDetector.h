/*  Quest Mark Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_QuestMarkDetector_H
#define PokemonAutomation_PokemonLA_QuestMarkDetector_H

#include <list>
#include "PokemonLA_WhiteObjectDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class QuestMarkDetector : public WhiteObjectDetector{
public:
    QuestMarkDetector();
    virtual void process_object(const ImageViewRGB32& image, const WaterfillObject& object) override;
};



}
}
}
#endif
