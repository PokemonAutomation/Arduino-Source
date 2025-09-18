/*  Date Manipulation (US)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_DateManip_US_H
#define PokemonAutomation_NintendoSwitch_DateManip_US_H

#include <memory>
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "NintendoSwitch_DateManipTools.h"
#include "NintendoSwitch_DateManipBase.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



class DateReader_US : public DateReaderBase{
public:
    DateReader_US(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual DateTime read_date(
        Logger& logger,
        std::shared_ptr<const ImageRGB32> screen
    ) override;
    virtual void set_date(
        const ProgramInfo& info, VideoStream& stream, ProControllerContext& context,
        const DateTime& date    //  Seconds is ignored.
    ) override;

protected:
    virtual void move_horizontal(ProControllerContext& context, int& current, int desired) const = 0;
    virtual void adjust_no_wrap(ProControllerContext& context, int current, int desired) const = 0;
    virtual void adjust_wrap(ProControllerContext& context, int min, int max, int current, int desired) const = 0;

protected:
    Color m_color;
    ImageFloatBox m_month;
    ImageFloatBox m_day;
    ImageFloatBox m_year;
    ImageFloatBox m_hour;
    ImageFloatBox m_minute;
    ImageFloatBox m_ampm;
};



class DateReader_Switch1_US : public DateReader_US{
public:
    DateReader_Switch1_US(Color color);
    virtual void move_horizontal(ProControllerContext& context, int& current, int desired) const override{
        DateReaderTools::move_horizontal(context, current, desired);
        current = desired;
    }
    virtual void adjust_no_wrap(ProControllerContext& context, int current, int desired) const override{
        DateReaderTools::adjust_no_wrap_Switch1(context, current, desired);
    }
    virtual void adjust_wrap(ProControllerContext& context, int min, int max, int current, int desired) const override{
        DateReaderTools::adjust_wrap_Switch1(context, min, max, current, desired);
    }
};
class DateReader_Switch2_US : public DateReader_US{
public:
    DateReader_Switch2_US(Color color);
    virtual void move_horizontal(ProControllerContext& context, int& current, int desired) const override{
        DateReaderTools::move_horizontal(context, current, desired);
        current = desired;
    }
    virtual void adjust_no_wrap(ProControllerContext& context, int current, int desired) const override{
        DateReaderTools::adjust_no_wrap_Switch2(context, current, desired);
    }
    virtual void adjust_wrap(ProControllerContext& context, int min, int max, int current, int desired) const override{
        DateReaderTools::adjust_wrap_Switch2(context, min, max, current, desired);
    }
};







}
}
#endif
