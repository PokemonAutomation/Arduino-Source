/*  Battle Move Selection Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QImage>
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/Tools/VideoOverlaySet.h"
#include "PokemonLA_StatusInfoScreenDetector.h"
#include "PokemonLA_CommonColorCheck.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


StatusInfoScreenDetector::StatusInfoScreenDetector()
    : VisualInferenceCallback("StatusInfoScreenDetector")
    , m_detected(0)
    , m_shiny_box(0.726, 0.133, 0.015, 0.023)
    , m_alpha_box(0.756, 0.137, 0.005, 0.005)
    //, m_other_box(0.310, 0.150, 0.015, 0.015)
    //, m_alpha_box(0.751, 0.133, 0.015, 0.023)
    //double x;
    //double y;
    //double width;
    //double height;
{}

void StatusInfoScreenDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_BLUE, m_shiny_box);
    items.add(COLOR_RED, m_alpha_box);
    //items.add(COLOR_GREEN, m_other_box);
}


bool StatusInfoScreenDetector::process_frame(
    const QImage& frame,
        std::chrono::system_clock::time_point timestamp
){
   const ImageStats shiny_box = image_stats(extract_box_reference(frame, m_shiny_box));
   if(!is_solid(shiny_box, {0.333333, 0.333333, 0.333333}, 0.2, 20)){
       m_detected = m_detected + 1;
   }

   const ImageStats alpha_box = image_stats(extract_box_reference(frame, m_alpha_box));
   //if(!is_solid(alpha_box, {0.333333, 0.333333, 0.333333}, 0.1)){ //Alternative
   //if(is_solid(alpha_box, {0.501968, 0.149606, 0.137795}, 0.2)){
   if(is_solid(alpha_box, {0.501968, 0.157480, 0.137795}, 0.2, 20)){
       m_detected = m_detected + 2;
   }

    //const ImageStats other_box = image_stats(extract_box_reference(frame, m_other_box));
    //if(is_solid(other_box, {0.501968, 0.149606, 0.137795}, 0.2)){
    //    m_detected = m_detected + 100;
    //}

    return false;
}

}
}
}
