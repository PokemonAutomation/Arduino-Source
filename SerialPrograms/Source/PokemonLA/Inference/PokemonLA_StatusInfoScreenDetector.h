/*  Status Info Screen Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  Detect shiny and alpha on the Status Info Screen.
 */

#ifndef PokemonAutomation_PokemonLA_StatusInfoScreenDetector_H
#define PokemonAutomation_PokemonLA_StatusInfoScreenDetector_H

#include "CommonFramework/Logging/LoggerQt.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "PokemonLA/Inference/Objects/PokemonLA_ButtonDetector.h"
#include "PokemonLA/Resources/PokemonLA_PokemonInfo.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


PokemonDetails read_status_info(
    LoggerQt& logger, VideoOverlay& overlay,
    const QImage& frame,
    Language language
);


#if 0
class StatusInfoScreenDetector{
public:
    StatusInfoScreenDetector();

//    int16_t detected() const{
//        return m_detected;
//    }

    PokemonDetails details()const{
        return m_pokemon;
    }

    void make_overlays(VideoOverlaySet& items) const;
    void detect(const QImage& frame);

    void get_pokemon_name(ConsoleHandle& console, QImage infoScreen, Language language);

private:
//    int16_t m_detected;
    PokemonDetails m_pokemon;
    ImageFloatBox m_shiny_box;
    ImageFloatBox m_alpha_box;
    ImageFloatBox m_gender_box;
    ImageFloatBox m_name_box;
};
#endif



}
}
}
#endif
