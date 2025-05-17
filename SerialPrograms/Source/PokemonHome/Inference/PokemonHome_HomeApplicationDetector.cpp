/*  Object Name Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/ImageTools/ImageStats.h"
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
    std::cout << box_name << std::endl;
    if(box_name == "Push any button"){
        return true;
    }

    // In most main menus, where the minus button for help shows
    ImageFloatBox minus_help_corner(0.03, 0.965, 0.06, 0.03); // Level box
    std::string help_button = OCR::ocr_read(Language::English, extract_box_reference(screen, minus_help_corner));
    for(auto a:chars){help_button.erase(std::remove(help_button.begin(),help_button.end(), a),help_button.end());}
    ImageFloatBox top_green(0.36, 0.01, 0.001, 0.001); // Level box
    ImageFloatBox top_white(0.36, 0.076, 0.001, 0.001); // Level box
    FloatPixel green_pixel = image_stats(extract_box_reference(screen, top_green)).average;
    FloatPixel white_pixel = image_stats(extract_box_reference(screen, top_white)).average;
    std::cout << help_button << std::endl;
    std::cout << green_pixel.r << " " << green_pixel.g << " " << green_pixel.b << std::endl;
    std::cout << white_pixel.r << " " << white_pixel.g << " " << white_pixel.b << std::endl;
    if(help_button == "Help" && euclidean_distance(green_pixel, FloatPixel(149, 248, 212))==0 && euclidean_distance(white_pixel, FloatPixel(255, 255, 255))==0){
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
