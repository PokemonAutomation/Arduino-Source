/*  Date Manipulation (24h)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_DateManip_24h_H
#define PokemonAutomation_NintendoSwitch_DateManip_24h_H

#include <memory>
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "NintendoSwitch_DateManipTools.h"
#include "NintendoSwitch_DateManipBase.h"

namespace PokemonAutomation{
namespace NintendoSwitch{




class DateReader_24h : public DateReaderBase{
public:
    DateReader_24h(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual DateTime read_date(
        Logger& logger,
        std::shared_ptr<const ImageRGB32> screen
    ) override;

protected:
    virtual void move_horizontal(ProControllerContext& context, int& current, int desired) const = 0;
    virtual void adjust_no_wrap(ProControllerContext& context, int current, int desired) const = 0;
    virtual void adjust_wrap(ProControllerContext& context, int min, int max, int current, int desired) const = 0;


protected:
    Color m_color;
    ImageFloatBox m_year;
    ImageFloatBox m_month;
    ImageFloatBox m_day;
    ImageFloatBox m_hour;
    ImageFloatBox m_minute;
};




class DateReader_EU : public DateReader_24h{
public:
    DateReader_EU(Color color);
    virtual void set_date(
        const ProgramInfo& info, VideoStream& stream, ProControllerContext& context,
        const DateTime& date    //  Seconds is ignored.
    ) override;
};
class DateReader_Switch1_EU : public DateReader_EU{
public:
    DateReader_Switch1_EU(Color color);
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
class DateReader_Switch2_EU : public DateReader_EU{
public:
    DateReader_Switch2_EU(Color color);
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




class DateReader_JP : public DateReader_24h{
public:
    DateReader_JP(Color color);
    virtual void set_date(
        const ProgramInfo& info, VideoStream& stream, ProControllerContext& context,
        const DateTime& date    //  Seconds is ignored.
    ) override;
};
class DateReader_Switch1_JP : public DateReader_JP{
public:
    DateReader_Switch1_JP(Color color);
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
class DateReader_Switch2_JP : public DateReader_JP{
public:
    DateReader_Switch2_JP(Color color);
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
