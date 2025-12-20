/*  Overworld Party Selection Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/FatalProgramException.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "PokemonLZA_OverworldPartySelectionDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

namespace {

std::array<ImageFloatBox, 6> get_boxes(){
    std::array<ImageFloatBox, 6> boxes;
    for(size_t i = 0; i < 6; i++){
        boxes[i] = {0.048 + 0.0398*i, 0.926, 0.017, 0.058};
    }
    return boxes;
}

const std::array<ImageFloatBox, 6>& BOXES(){
    static const std::array<ImageFloatBox, 6> boxes = get_boxes();
    return boxes;
}

}


OverworldPartySelectionDetector::OverworldPartySelectionDetector(Color color, VideoOverlay* overlay)
    : m_dpad_ups{
        ButtonDetector(color, ButtonType::ButtonDpadUp, BOXES()[0], overlay),
        ButtonDetector(color, ButtonType::ButtonDpadUp, BOXES()[1], overlay),
        ButtonDetector(color, ButtonType::ButtonDpadUp, BOXES()[2], overlay),
        ButtonDetector(color, ButtonType::ButtonDpadUp, BOXES()[3], overlay),
        ButtonDetector(color, ButtonType::ButtonDpadUp, BOXES()[4], overlay),
        ButtonDetector(color, ButtonType::ButtonDpadUp, BOXES()[5], overlay)
    }
    , m_dpad_downs{
        ButtonDetector(color, ButtonType::ButtonDpadDown, BOXES()[0], overlay),
        ButtonDetector(color, ButtonType::ButtonDpadDown, BOXES()[1], overlay),
        ButtonDetector(color, ButtonType::ButtonDpadDown, BOXES()[2], overlay),
        ButtonDetector(color, ButtonType::ButtonDpadDown, BOXES()[3], overlay),
        ButtonDetector(color, ButtonType::ButtonDpadDown, BOXES()[4], overlay),
        ButtonDetector(color, ButtonType::ButtonDpadDown, BOXES()[5], overlay)
    }
    , m_dpad_up_interiors{
        ButtonDetector(color, ButtonType::ButtonDpadUpInterior, BOXES()[0]),
        ButtonDetector(color, ButtonType::ButtonDpadUpInterior, BOXES()[1]),
        ButtonDetector(color, ButtonType::ButtonDpadUpInterior, BOXES()[2]),
        ButtonDetector(color, ButtonType::ButtonDpadUpInterior, BOXES()[3]),
        ButtonDetector(color, ButtonType::ButtonDpadUpInterior, BOXES()[4]),
        ButtonDetector(color, ButtonType::ButtonDpadUpInterior, BOXES()[5])
    }
    , m_dpad_down_interiors{
        ButtonDetector(color, ButtonType::ButtonDpadDownInterior, BOXES()[0]),
        ButtonDetector(color, ButtonType::ButtonDpadDownInterior, BOXES()[1]),
        ButtonDetector(color, ButtonType::ButtonDpadDownInterior, BOXES()[2]),
        ButtonDetector(color, ButtonType::ButtonDpadDownInterior, BOXES()[3]),
        ButtonDetector(color, ButtonType::ButtonDpadDownInterior, BOXES()[4]),
        ButtonDetector(color, ButtonType::ButtonDpadDownInterior, BOXES()[5])
    }
{}

void OverworldPartySelectionDetector::make_overlays(VideoOverlaySet& items) const{
    for(size_t i = 0; i < 6; i++){
        m_dpad_ups[i].make_overlays(items);
        m_dpad_downs[i].make_overlays(items);
    }
}

bool OverworldPartySelectionDetector::detect(const ImageViewRGB32& screen){
    m_detected_up_idx = INVALID_PARTY_IDX, m_detected_down_idx = INVALID_PARTY_IDX;
    for(uint8_t i = 0; i < 6; i++){
        if (m_debug_mode || m_detected_up_idx == INVALID_PARTY_IDX){
            if (m_dpad_ups[i].detect(screen) || m_dpad_up_interiors[i].detect(screen)){
                if (m_debug_mode && m_dpad_up_interiors[i].detect(screen)){
                    cout << "DPAD UP Interior detected at index " << i << endl;
                }
                if (m_debug_mode && m_detected_up_idx != INVALID_PARTY_IDX){
                    cout << "Multiple dpad up buttons detected! First detection " << int(m_detected_up_idx)
                         << " second detection (" << int(i) << endl;
                    throw FatalProgramException(ErrorReport::NO_ERROR_REPORT,
                        "Multiple dpad up buttons detected!", nullptr, screen.copy());
                }
                m_detected_up_idx = i;
            }
        }
        if (m_debug_mode || m_detected_down_idx == INVALID_PARTY_IDX){
            if (m_dpad_downs[i].detect(screen) || m_dpad_down_interiors[i].detect(screen)){
                if (m_debug_mode && m_dpad_down_interiors[i].detect(screen)){
                    cout << "DPAD DOWN Interior detected at index " << i << endl;
                }
                if (m_debug_mode && m_detected_down_idx != INVALID_PARTY_IDX){
                    cout << "Multiple dpad down buttons detected! First detection " << int(m_detected_down_idx)
                         << " second detection (" << int(i) << endl;
                    throw FatalProgramException(ErrorReport::NO_ERROR_REPORT,
                        "Multiple dpad down buttons detected!", nullptr, screen.copy());
                }
                m_detected_down_idx = i;
            }
        }
    }

    return m_detected_down_idx != INVALID_PARTY_IDX || m_detected_up_idx != INVALID_PARTY_IDX;
}

uint8_t OverworldPartySelectionDetector::selected_party_idx() const{
    return m_detected_up_idx != INVALID_PARTY_IDX ? m_detected_up_idx : m_detected_down_idx;
}




}
}
}
