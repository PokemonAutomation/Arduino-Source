/*  Object Name Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/OCR/OCR_RawOCR.h"
#include <iostream>
#include "PokemonHome_HomeApplicationDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{



HomeApplicationDetector::~HomeApplicationDetector() = default;

HomeApplicationDetector::HomeApplicationDetector(Color color)
    : m_color(color)
{}

void HomeApplicationDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}

bool HomeApplicationDetector::detect(const ImageViewRGB32& screen) const{

    // Title screen banner says "Push any button"
    ImageFloatBox title_screen_box(0.3, 0.8, 0.4, 0.075); // Level box
    char chars[] = "\n\r—";
    std::string box_name = OCR::ocr_read(Language::English, extract_box_reference(screen, title_screen_box));
    for(auto a:chars){box_name.erase(std::remove(box_name.begin(),box_name.end(), a),box_name.end());}
    std::cout << box_name;
    if(box_name == "Push any button"){
        return true;
    }

    // No other instances found, did not detect
    return false;
}



HomeApplicationWatcher::~HomeApplicationWatcher() = default;

HomeApplicationWatcher::HomeApplicationWatcher(Color color)
    : VisualInferenceCallback("HomeApplicationWatcher")
    , m_detector(color)
{}

void HomeApplicationWatcher::make_overlays(VideoOverlaySet& items) const{
    m_detector.make_overlays(items);
}

bool HomeApplicationWatcher::process_frame(const ImageViewRGB32& screen, WallClock timestamp){
    // std::vector<ImageFloatBox> hits = m_detector.detect_all(screen);
    
    // m_hits.reset(hits.size());
    // for (const ImageFloatBox& hit : hits){
    //     m_hits.emplace_back(m_overlay, hit, COLOR_MAGENTA);
    // }
    // return !hits.empty();
    return m_detector.detect(screen);
}









}
}
}
