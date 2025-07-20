/*  Date Change Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_DateChangeDetector_H
#define PokemonAutomation_NintendoSwitch_DateChangeDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


enum class DateFormat{
    US,
    EU,
    JP,
};




class DateChangeDetector : public StaticScreenDetector{
public:
    virtual DateFormat detect_date_format(const ImageViewRGB32& screen) const = 0;
};
class DateChangeDetector_Switch1 : public DateChangeDetector{
public:
    DateChangeDetector_Switch1(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;
    virtual DateFormat detect_date_format(const ImageViewRGB32& screen) const override;

private:
    Color m_color;
    ImageFloatBox m_background_top;
    ImageFloatBox m_window_top;
    ImageFloatBox m_window_text;
    ImageFloatBox m_jp_year;
    ImageFloatBox m_us_hour;
    ImageFloatBox m_jp_month_arrow;
};
class DateChangeDetector_Switch2 : public DateChangeDetector{
public:
    DateChangeDetector_Switch2(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;
    virtual DateFormat detect_date_format(const ImageViewRGB32& screen) const override;

private:
    Color m_color;
    ImageFloatBox m_background_top;
    ImageFloatBox m_window_bottom;
    ImageFloatBox m_window_text;
    ImageFloatBox m_jp_year;
    ImageFloatBox m_us_hour;
    ImageFloatBox m_jp_month_arrow;
};




}
}
#endif
