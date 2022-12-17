/*  Date Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_DateReader_H
#define PokemonAutomation_NintendoSwitch_DateReader_H

#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"
#include "CommonFramework/Inference/VisualDetector.h"

namespace PokemonAutomation{
    struct ProgramInfo;
    class Logger;
    class ConsoleHandle;
    class BotBaseContext;
namespace NintendoSwitch{


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
    );

private:
    ImageFloatBox m_background_top;
    ImageFloatBox m_window_top;
    ImageFloatBox m_window_text;
    ImageFloatBox m_us_hours;
    ImageFloatBox m_us_min;
    ImageFloatBox m_us_ampm;
    ImageFloatBox m_24_hours;
    ImageFloatBox m_24_min;
};



}
}
#endif
