#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTools/ImageDiff.h"
#include "CommonTools/Images/ImageFilter.h"
#include "PokemonLZA_WeatherDetector.h"

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonLZA {


//-----------------------------------------------------
//  Weather Info Table (two images per weather)
//-----------------------------------------------------

static const WeatherTemplateInfo WEATHER_TABLE[] = {

    // Clear
    {
        "PokemonLZA/Weather/sun_drop.png",
        "PokemonLZA/Weather/clear_core.png",
        ImageFloatBox(0.8945, 0.0570, 0.0050, 0.0100),
        ImageFloatBox(0.8910, 0.0360, 0.0110, 0.0210),
        COLOR_RED,
        COLOR_GREEN
    },

    // Sunny
    {
        "PokemonLZA/Weather/sun_drop.png",
        "PokemonLZA/Weather/sunny_core.png",
         ImageFloatBox(0.8945, 0.0570, 0.0050, 0.0100),
        ImageFloatBox(0.8910, 0.0360, 0.0110, 0.0210),
        COLOR_RED,
        COLOR_GREEN
    },

    // Rain
    {
        "PokemonLZA/Weather/rain_cloud.png",
        "PokemonLZA/Weather/rain_drop.png",
         ImageFloatBox(0.8865, 0.0265, 0.0210, 0.0270),
        ImageFloatBox(0.8885, 0.0525, 0.0050, 0.0150),
        COLOR_RED,
        COLOR_GREEN
    },

    // Cloudy
    {
        "PokemonLZA/Weather/cloudy_cloud.png",
        "PokemonLZA/Weather/cloudy_drop.png",
        ImageFloatBox(0.8895, 0.0430, 0.0210, 0.0265),
        ImageFloatBox(0.8915, 0.0255, 0.0050, 0.0100),
        COLOR_RED,
        COLOR_GREEN
    },

    // Foggy
    {
        "PokemonLZA/Weather/foggy_tray_1.png",
        "PokemonLZA/Weather/foggy_tray_2.png",
         ImageFloatBox(0.8893, 0.0487, 0.0218, 0.0080),
        ImageFloatBox(0.8880, 0.0555, 0.0225, 0.0080),
        COLOR_RED,
        COLOR_GREEN
    },

    // Rainbow
    {
        "PokemonLZA/Weather/rainbow_cloud.png",
        "PokemonLZA/Weather/rainbow_arch.png",
         ImageFloatBox(0.8840, 0.0465, 0.0140, 0.0165),
        ImageFloatBox(0.8930, 0.0420, 0.0120, 0.0100),
        COLOR_RED,
        COLOR_GREEN
    },
};


const WeatherTemplateInfo& weather_template_info(WeatherIconType icon){
    return WEATHER_TABLE[(int)icon];
}

//-----------------------------------------------------
//  Detector
//-----------------------------------------------------

WeatherIconDetector::WeatherIconDetector(WeatherIconType type, VideoOverlay* overlay)
{
    m_info = &weather_template_info(type);

    if (overlay){
        m_overlay1.emplace(*overlay, m_info->box1, m_info->color1);
        m_overlay2.emplace(*overlay, m_info->box2, m_info->color2);
    }
}

void WeatherIconDetector::make_overlays(VideoOverlaySet& items) const {
    items.add(m_info->color1, m_info->box1);
    items.add(m_info->color2, m_info->box2);
}

bool WeatherIconDetector::detect(const ImageViewRGB32& screen){

    // Extract screen patches
    ImageRGB32 c1 = extract_box_reference(screen, m_info->box1).copy();
    ImageRGB32 c2 = extract_box_reference(screen, m_info->box2).copy();

    // Load templates
    ImageRGB32 t1(RESOURCE_PATH() + m_info->path1);
    ImageRGB32 t2(RESOURCE_PATH() + m_info->path2);

    if (t1.width() == 0 || t2.width() == 0){
        return false;
    }

    double rms1 = ImageMatch::pixel_RMSD(c1, t1);
    double rms2 = ImageMatch::pixel_RMSD(c2, t2);

    return rms1 < 90 && rms2 < 90;
}


}
}
}
