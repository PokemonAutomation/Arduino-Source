/*  Main Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "PokemonSwSh_MainMenuDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


MainMenuDetector::MainMenuDetector(Color color)
    : m_color(color)
    , m_top_red(0.302449, 0.028409, 0.630458, 0.060606)
    , m_top_left(0.015974, 0.028409, 0.068158, 0.096591)
    , m_bottom_left(0.095847, 0.782197, 0.122471, 0.140152)
{}

void MainMenuDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_top_red);
    items.add(m_color, m_top_left);
    items.add(m_color, m_bottom_left);
}
bool MainMenuDetector::detect(const ImageViewRGB32& screen){
    ImageStats top_red = image_stats(extract_box_reference(screen, m_top_red));
    if (!is_solid(top_red, {0.66369, 0.145833, 0.190476}, 0.20)){
        return false;
    }

    ImageStats top_left = image_stats(extract_box_reference(screen, m_top_left));
    if (!is_solid(top_left, {0.352778, 0.323611, 0.323611})){
        return false;
    }

    ImageStats bottom_left = image_stats(extract_box_reference(screen, m_bottom_left));
    if (!is_solid(bottom_left, {0.66369, 0.145833, 0.190476}, 0.20)){
        return false;
    }

    if (euclidean_distance(top_red.average, bottom_left.average) > 10){
        return false;
    }

    return true;
}





PartyMenuDetector::PartyMenuDetector(Color color)
    : m_color(color)
    , m_top_right(0.767838, 0.018939, 0.220447, 0.034091)
    , m_top_dark_red(0.619808, 0.013258, 0.078807, 0.022727)
    , m_bottom_dark_red(0.383387, 0.922348, 0.078807, 0.026515)
    , m_left(0.007455, 0.143939, 0.011715, 0.793561)
{}

void PartyMenuDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_top_right);
    items.add(m_color, m_top_dark_red);
    items.add(m_color, m_bottom_dark_red);
    items.add(m_color, m_left);
}
bool PartyMenuDetector::detect(const ImageViewRGB32& screen){
    ImageStats top_right = image_stats(extract_box_reference(screen, m_top_right));
//    cout << top_right.average << top_right.stddev << endl;
    if (!is_white(top_right, 500, 20)){
        return false;
    }

    ImageStats top_dark_red = image_stats(extract_box_reference(screen, m_top_dark_red));
//    cout << top_dark_red.average << top_dark_red.stddev << endl;
    if (!is_solid(top_dark_red, {0.681818, 0.136364, 0.181818}, 0.20)){
        return false;
    }

    ImageStats bottom_dark_red = image_stats(extract_box_reference(screen, m_bottom_dark_red));
//    cout << bottom_dark_red.average << bottom_dark_red.stddev << endl;
    if (!is_solid(bottom_dark_red, {0.681818, 0.136364, 0.181818}, 0.20)){
        return false;
    }

    ImageStats left = image_stats(extract_box_reference(screen, m_left));
//    cout << left.average << left.stddev << endl;
    if (!is_solid(left, {0.66369, 0.145833, 0.190476}, 0.20)){
        return false;
    }

    if (euclidean_distance(top_dark_red.average, bottom_dark_red.average) > 10){
        return false;
    }

    return true;
}


BoxMenuDetector::BoxMenuDetector(Color color)
    : m_color(color)
    , m_left_green(0.007455, 0.085227, 0.010650, 0.587121)
    , m_top_white(0.621938, 0.032197, 0.050053, 0.121212)
    , m_bottom_white(0.234292, 0.818182, 0.009585, 0.113636)
{}

void BoxMenuDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_left_green);
    items.add(m_color, m_top_white);
    items.add(m_color, m_bottom_white);
}
bool BoxMenuDetector::detect(const ImageViewRGB32& screen){
    ImageStats left_green = image_stats(extract_box_reference(screen, m_left_green));
    if (!is_solid(left_green, {0.387097, 0.501075, 0.111828}, 0.20)){
        return false;
    }

    ImageStats top_white = image_stats(extract_box_reference(screen, m_top_white));
    if (!is_white(top_white, 500, 20)){
        return false;
    }

    ImageStats bottom_white = image_stats(extract_box_reference(screen, m_bottom_white));
    if (!is_white(bottom_white, 500, 15)){
        return false;
    }

    return true;
}









}
}
}
