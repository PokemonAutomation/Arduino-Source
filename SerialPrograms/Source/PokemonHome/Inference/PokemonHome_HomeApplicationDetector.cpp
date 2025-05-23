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
    ImageFloatBox title_screen_box(0.3, 0.8, 0.4, 0.075);
    char chars[] = "\n\r—";
    std::string box_name = OCR::ocr_read(Language::English, extract_box_reference(screen, title_screen_box));
    for(auto a:chars){box_name.erase(std::remove(box_name.begin(),box_name.end(), a),box_name.end());}
    if(box_name == "Push any button"){
        return true;
    }

    // In most main menus, where the minus button for help shows
    ImageFloatBox minus_help_corner(0.03, 0.965, 0.06, 0.03);
    std::string minus_button = OCR::ocr_read(Language::English, extract_box_reference(screen, minus_help_corner));
    for(auto a:chars){minus_button.erase(std::remove(minus_button.begin(),minus_button.end(), a),minus_button.end());}
    ImageFloatBox top_green(0.36, 0.01, 0.001, 0.001);
    ImageFloatBox top_white(0.36, 0.076, 0.001, 0.001);
    FloatPixel green_pixel = image_stats(extract_box_reference(screen, top_green)).average;
    FloatPixel white_pixel = image_stats(extract_box_reference(screen, top_white)).average;
    if(minus_button == "Help" && euclidean_distance(green_pixel, FloatPixel(149, 248, 212))==0 && euclidean_distance(white_pixel, FloatPixel(255, 255, 255))==0){
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

    return m_detector.detect(screen);
}




HomeTitleScreenDetector::~HomeTitleScreenDetector() = default;

HomeTitleScreenDetector::HomeTitleScreenDetector(Color color)
    : m_color(color)
{}

void HomeTitleScreenDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}

bool HomeTitleScreenDetector::detect(const ImageViewRGB32& screen) const{

    // Title screen banner says "Push any button"
    ImageFloatBox title_screen_box(0.3, 0.8, 0.4, 0.075);
    char chars[] = "\n\r—";
    std::string box_name = OCR::ocr_read(Language::English, extract_box_reference(screen, title_screen_box));
    for(auto a:chars){box_name.erase(std::remove(box_name.begin(),box_name.end(), a),box_name.end());}
    if(box_name == "Push any button"){
        return true;
    }

    // No other instances found, did not detect
    return false;
}



HomeTitleScreenWatcher::~HomeTitleScreenWatcher() = default;

HomeTitleScreenWatcher::HomeTitleScreenWatcher(Color color)
    : VisualInferenceCallback("HomeApplicationWatcher")
    , m_detector(color)
{}

void HomeTitleScreenWatcher::make_overlays(VideoOverlaySet& items) const{
    m_detector.make_overlays(items);
}

bool HomeTitleScreenWatcher::process_frame(const ImageViewRGB32& screen, WallClock timestamp){

    return m_detector.detect(screen);
}





HomeMainMenuDetector::~HomeMainMenuDetector() = default;

HomeMainMenuDetector::HomeMainMenuDetector(Color color)
    : m_color(color)
{}

void HomeMainMenuDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}

bool HomeMainMenuDetector::detect(const ImageViewRGB32& screen) const{

    char chars[] = "\n\r—";

    // In most main menus, where the minus button for help shows
    ImageFloatBox minus_help_corner(0.03, 0.965, 0.06, 0.03);
    ImageFloatBox menu_read_corner(0.06, 0.021, 0.25, 0.05);
    std::string minus_button = OCR::ocr_read(Language::English, extract_box_reference(screen, minus_help_corner));
    std::string menu_text = OCR::ocr_read(Language::English, extract_box_reference(screen, menu_read_corner));
    for(auto a:chars){minus_button.erase(std::remove(minus_button.begin(),minus_button.end(), a),minus_button.end());}
    for(auto a:chars){menu_text.erase(std::remove(menu_text.begin(),menu_text.end(), a),menu_text.end());}
    ImageFloatBox top_green(0.36, 0.01, 0.001, 0.001);
    ImageFloatBox top_white(0.36, 0.076, 0.001, 0.001);
    FloatPixel green_pixel = image_stats(extract_box_reference(screen, top_green)).average;
    FloatPixel white_pixel = image_stats(extract_box_reference(screen, top_white)).average;
    if(menu_text == "MAIN MENU" && minus_button == "Help" && euclidean_distance(green_pixel, FloatPixel(149, 248, 212))==0 && euclidean_distance(white_pixel, FloatPixel(255, 255, 255))==0){
        return true;
    }

    // No other instances found, did not detect
    return false;
}



HomeMainMenuWatcher::~HomeMainMenuWatcher() = default;

HomeMainMenuWatcher::HomeMainMenuWatcher(Color color)
    : VisualInferenceCallback("HomeApplicationWatcher")
    , m_detector(color)
{}

void HomeMainMenuWatcher::make_overlays(VideoOverlaySet& items) const{
    m_detector.make_overlays(items);
}

bool HomeMainMenuWatcher::process_frame(const ImageViewRGB32& screen, WallClock timestamp){

    return m_detector.detect(screen);
}



HomeGameSelectDetector::~HomeGameSelectDetector() = default;

HomeGameSelectDetector::HomeGameSelectDetector(Color color)
    : m_color(color)
{}

void HomeGameSelectDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}

bool HomeGameSelectDetector::detect(const ImageViewRGB32& screen) const{

    char chars[] = "\n\r—.,";

    // In most main menus, where the minus button for help shows
    ImageFloatBox minus_help_corner(0.03, 0.965, 0.06, 0.03);
    ImageFloatBox help_text(0.3, 0.1, 0.41, 0.05);
    std::string minus_button = OCR::ocr_read(Language::English, extract_box_reference(screen, minus_help_corner));
    std::string menu_text = OCR::ocr_read(Language::English, extract_box_reference(screen, help_text));
    for(auto a:chars){minus_button.erase(std::remove(minus_button.begin(),minus_button.end(), a),minus_button.end());}
    for(auto a:chars){menu_text.erase(std::remove(menu_text.begin(),menu_text.end(), a),menu_text.end());}
    ImageFloatBox top_green(0.36, 0.01, 0.001, 0.001);
    ImageFloatBox top_white(0.36, 0.076, 0.001, 0.001);
    FloatPixel green_pixel = image_stats(extract_box_reference(screen, top_green)).average;
    FloatPixel white_pixel = image_stats(extract_box_reference(screen, top_white)).average;
    if(menu_text == "Select a game to connect to Pokémon HOME" && minus_button == "Help" && euclidean_distance(green_pixel, FloatPixel(149, 248, 212))==0 && euclidean_distance(white_pixel, FloatPixel(255, 255, 255))==0){
        return true;
    }

    // No other instances found, did not detect
    return false;
}



HomeGameSelectWatcher::~HomeGameSelectWatcher() = default;

HomeGameSelectWatcher::HomeGameSelectWatcher(Color color)
    : VisualInferenceCallback("HomeApplicationWatcher")
    , m_detector(color)
{}

void HomeGameSelectWatcher::make_overlays(VideoOverlaySet& items) const{
    m_detector.make_overlays(items);
}

bool HomeGameSelectWatcher::process_frame(const ImageViewRGB32& screen, WallClock timestamp){

    return m_detector.detect(screen);
}



HomeListViewDetector::~HomeListViewDetector() = default;

HomeListViewDetector::HomeListViewDetector(Color color)
    : m_color(color)
{}

void HomeListViewDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}

bool HomeListViewDetector::detect(const ImageViewRGB32& screen) const{

    char chars[] = "\n\r—";

    // In most main menus, where the minus button for help shows
    ImageFloatBox minus_help_corner(0.03, 0.965, 0.06, 0.03);
    ImageFloatBox menu_read_corner(0.06, 0.021, 0.25, 0.05);
    std::string minus_button = OCR::ocr_read(Language::English, extract_box_reference(screen, minus_help_corner));
    std::string menu_text = OCR::ocr_read(Language::English, extract_box_reference(screen, menu_read_corner));
    for(auto a:chars){minus_button.erase(std::remove(minus_button.begin(),minus_button.end(), a),minus_button.end());}
    for(auto a:chars){menu_text.erase(std::remove(menu_text.begin(),menu_text.end(), a),menu_text.end());}
    ImageFloatBox top_green(0.36, 0.01, 0.001, 0.001);
    ImageFloatBox top_white(0.36, 0.076, 0.001, 0.001);
    FloatPixel green_pixel = image_stats(extract_box_reference(screen, top_green)).average;
    FloatPixel white_pixel = image_stats(extract_box_reference(screen, top_white)).average;
    if(menu_text == "POKEMON LIST" && minus_button == "Help" && euclidean_distance(green_pixel, FloatPixel(149, 248, 212))==0 && euclidean_distance(white_pixel, FloatPixel(255, 255, 255))==0){
        return true;
    }

    // No other instances found, did not detect
    return false;
}



HomeListViewWatcher::~HomeListViewWatcher() = default;

HomeListViewWatcher::HomeListViewWatcher(Color color)
    : VisualInferenceCallback("HomeApplicationWatcher")
    , m_detector(color)
{}

void HomeListViewWatcher::make_overlays(VideoOverlaySet& items) const{
    m_detector.make_overlays(items);
}

bool HomeListViewWatcher::process_frame(const ImageViewRGB32& screen, WallClock timestamp){

    return m_detector.detect(screen);
}


HomeSummaryViewDetector::~HomeSummaryViewDetector() = default;

HomeSummaryViewDetector::HomeSummaryViewDetector(Color color)
    : m_color(color)
{}

void HomeSummaryViewDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}

bool HomeSummaryViewDetector::detect(const ImageViewRGB32& screen) const{

    char chars[] = "\n\r—";

    // In most main menus, where the minus button for help shows
    ImageFloatBox minus_help_corner(0.03, 0.965, 0.06, 0.03);
    ImageFloatBox menu_read_corner(0.06, 0.021, 0.25, 0.05);
    std::string minus_button = OCR::ocr_read(Language::English, extract_box_reference(screen, minus_help_corner));
    std::string menu_text = OCR::ocr_read(Language::English, extract_box_reference(screen, menu_read_corner));
    for(auto a:chars){minus_button.erase(std::remove(minus_button.begin(),minus_button.end(), a),minus_button.end());}
    for(auto a:chars){menu_text.erase(std::remove(menu_text.begin(),menu_text.end(), a),menu_text.end());}
    ImageFloatBox top_green(0.36, 0.01, 0.001, 0.001);
    ImageFloatBox top_white(0.36, 0.076, 0.001, 0.001);
    FloatPixel green_pixel = image_stats(extract_box_reference(screen, top_green)).average;
    FloatPixel white_pixel = image_stats(extract_box_reference(screen, top_white)).average;
    if(menu_text == "CHECK SUMMARY" && minus_button == "Help" && euclidean_distance(green_pixel, FloatPixel(149, 248, 212))==0 && euclidean_distance(white_pixel, FloatPixel(255, 255, 255))==0){
        return true;
    }

    // No other instances found, did not detect
    return false;
}



HomeSummaryViewWatcher::~HomeSummaryViewWatcher() = default;

HomeSummaryViewWatcher::HomeSummaryViewWatcher(Color color)
    : VisualInferenceCallback("HomeApplicationWatcher")
    , m_detector(color)
{}

void HomeSummaryViewWatcher::make_overlays(VideoOverlaySet& items) const{
    m_detector.make_overlays(items);
}

bool HomeSummaryViewWatcher::process_frame(const ImageViewRGB32& screen, WallClock timestamp){

    return m_detector.detect(screen);
}


HomeMarkingsViewDetector::~HomeMarkingsViewDetector() = default;

HomeMarkingsViewDetector::HomeMarkingsViewDetector(Color color)
    : m_color(color)
{}

void HomeMarkingsViewDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}

bool HomeMarkingsViewDetector::detect(const ImageViewRGB32& screen) const{

    char chars[] = "\n\r—";

    // In most main menus, where the minus button for help shows
    ImageFloatBox minus_help_corner(0.03, 0.965, 0.06, 0.03);
    ImageFloatBox menu_read_corner(0.06, 0.021, 0.25, 0.05);
    ImageFloatBox markings_box(0.76, 0.325, 0.1, 0.05);
    std::string minus_button = OCR::ocr_read(Language::English, extract_box_reference(screen, minus_help_corner));
    std::string menu_text = OCR::ocr_read(Language::English, extract_box_reference(screen, menu_read_corner));
    std::string markings_text = OCR::ocr_read(Language::English, extract_box_reference(screen, markings_box));
    for(auto a:chars){minus_button.erase(std::remove(minus_button.begin(),minus_button.end(), a),minus_button.end());}
    for(auto a:chars){menu_text.erase(std::remove(menu_text.begin(),menu_text.end(), a),menu_text.end());}
    for(auto a:chars){markings_text.erase(std::remove(markings_text.begin(),markings_text.end(), a),markings_text.end());}
    ImageFloatBox top_green(0.36, 0.01, 0.001, 0.001);
    ImageFloatBox top_white(0.36, 0.076, 0.001, 0.001);
    FloatPixel green_pixel = image_stats(extract_box_reference(screen, top_green)).average;
    FloatPixel white_pixel = image_stats(extract_box_reference(screen, top_white)).average;
    if(markings_text == "Markings" && menu_text == "POKEMON" && minus_button == "Help" && euclidean_distance(green_pixel, FloatPixel(101, 179, 150))==0 && euclidean_distance(white_pixel, FloatPixel(207, 206, 206))==0){
        return true;
    }

    // No other instances found, did not detect
    return false;
}



HomeMarkingsViewWatcher::~HomeMarkingsViewWatcher() = default;

HomeMarkingsViewWatcher::HomeMarkingsViewWatcher(Color color)
    : VisualInferenceCallback("HomeApplicationWatcher")
    , m_detector(color)
{}

void HomeMarkingsViewWatcher::make_overlays(VideoOverlaySet& items) const{
    m_detector.make_overlays(items);
}

bool HomeMarkingsViewWatcher::process_frame(const ImageViewRGB32& screen, WallClock timestamp){

    return m_detector.detect(screen);
}


HomeBoxViewDetector::~HomeBoxViewDetector() = default;

HomeBoxViewDetector::HomeBoxViewDetector(Color color)
    : m_color(color)
{}

void HomeBoxViewDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}

bool HomeBoxViewDetector::detect(const ImageViewRGB32& screen) const{

    char chars[] = "\n\r—";

    // In most main menus, where the minus button for help shows
    ImageFloatBox minus_help_corner(0.03, 0.965, 0.06, 0.03);
    ImageFloatBox menu_read_corner(0.06, 0.021, 0.25, 0.05);
    std::string minus_button = OCR::ocr_read(Language::English, extract_box_reference(screen, minus_help_corner));
    std::string menu_text = OCR::ocr_read(Language::English, extract_box_reference(screen, menu_read_corner));
    for(auto a:chars){minus_button.erase(std::remove(minus_button.begin(),minus_button.end(), a),minus_button.end());}
    for(auto a:chars){menu_text.erase(std::remove(menu_text.begin(),menu_text.end(), a),menu_text.end());}
    ImageFloatBox top_green(0.36, 0.01, 0.001, 0.001);
    ImageFloatBox top_white(0.36, 0.076, 0.001, 0.001);
    FloatPixel green_pixel = image_stats(extract_box_reference(screen, top_green)).average;
    FloatPixel white_pixel = image_stats(extract_box_reference(screen, top_white)).average;
    if(menu_text == "POKEMON" && minus_button == "Help" && euclidean_distance(green_pixel, FloatPixel(149, 248, 212))==0 && euclidean_distance(white_pixel, FloatPixel(255, 255, 255))==0){
        return true;
    }

    // No other instances found, did not detect
    return false;
}



HomeBoxViewWatcher::~HomeBoxViewWatcher() = default;

HomeBoxViewWatcher::HomeBoxViewWatcher(Color color)
    : VisualInferenceCallback("HomeApplicationWatcher")
    , m_detector(color)
{}

void HomeBoxViewWatcher::make_overlays(VideoOverlaySet& items) const{
    m_detector.make_overlays(items);
}

bool HomeBoxViewWatcher::process_frame(const ImageViewRGB32& screen, WallClock timestamp){

    return m_detector.detect(screen);
}



}
}
}
