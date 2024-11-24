/*  Date Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_DateReader_H
#define PokemonAutomation_NintendoSwitch_DateReader_H

#include "Common/Cpp/DateTime.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"
#include "CommonFramework/Inference/VisualDetector.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
    struct ProgramInfo;
    class Logger;
    class ConsoleHandle;
    class BotBaseContext;
namespace NintendoSwitch{


enum class DateFormat{
    US,
    EU,
    JP,
};


class DateReader : public StaticScreenDetector{
public:
    DateReader();

    virtual void make_overlays(VideoOverlaySet& items) const override;

    //  Returns true if we are on the date change window.
    virtual bool detect(const ImageViewRGB32& screen) const override;

    //  Read the hours (0 - 23) while on the date change window.
    //  Returns -1 if unable to read.
    int8_t read_hours(Logger& logger, const ImageViewRGB32& screen) const;

    void set_hours(
        const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context,
        uint8_t hour    //  0 - 23
    ) const;


    std::pair<DateFormat, DateTime> read_date(Logger& logger, std::shared_ptr<const ImageRGB32> screen) const;
    void set_date(
        const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context,
        const DateTime& date    //  Seconds is ignored.
    ) const;

private:
    static ImageRGB32 filter_image(const ImageViewRGB32& image, bool white_theme);
    int read_box(
        Logger& logger,
        int min, int max,
        const ImageViewRGB32& screen, const ImageFloatBox& box,
        bool white_theme
    ) const;

    DateTime read_date_us(Logger& logger, std::shared_ptr<const ImageRGB32> screen, bool white_theme) const;
    DateTime read_date_eu(Logger& logger, std::shared_ptr<const ImageRGB32> screen, bool white_theme) const;
    DateTime read_date_jp(Logger& logger, std::shared_ptr<const ImageRGB32> screen, bool white_theme) const;

    static void move_cursor(BotBaseContext& context, int current, int desired);
    static void adjust_year(BotBaseContext& context, int current, int desired);
    static void adjust_month(BotBaseContext& context, int current, int desired);
    static void adjust_hour_24(BotBaseContext& context, int current, int desired);
    static void adjust_minute(BotBaseContext& context, int current, int desired);

private:
    ImageFloatBox m_background_top;
    ImageFloatBox m_window_top;
    ImageFloatBox m_window_text;
    ImageFloatBox m_jp_month_arrow;

    ImageFloatBox m_us_month;
    ImageFloatBox m_us_day;
    ImageFloatBox m_us_year;
    ImageFloatBox m_us_hour;
    ImageFloatBox m_us_minute;
    ImageFloatBox m_us_ampm;

    ImageFloatBox m_eu_day;
    ImageFloatBox m_eu_month;
    ImageFloatBox m_eu_year;
    ImageFloatBox m_24_hour;
    ImageFloatBox m_24_minute;

    ImageFloatBox m_jp_year;
    ImageFloatBox m_jp_month;
    ImageFloatBox m_jp_day;
};

class DateChangeWatcher : public DetectorToFinder<DateReader>{
public:
    DateChangeWatcher(std::chrono::milliseconds duration = std::chrono::milliseconds(250))
         : DetectorToFinder("DateChangeWatcher", duration)
    {}
};

// starting from Home screen, change the date to the desired date
// then go back to the home screen
void change_date(
    SingleSwitchProgramEnvironment& env, 
    BotBaseContext& context,
    const DateTime& date
);



}
}
#endif
