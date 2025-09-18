/*  Date Manip
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_DateManip_H
#define PokemonAutomation_NintendoSwitch_DateManip_H

#include <memory>
#include "Common/Cpp/DateTime.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonTools/VisualDetector.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Inference/NintendoSwitch_DateChangeDetector.h"
#include "NintendoSwitch/Programs/DateManip/NintendoSwitch_DateManip_US.h"
#include "NintendoSwitch/Programs/DateManip/NintendoSwitch_DateManip_24h.h"

namespace PokemonAutomation{
    struct ProgramInfo;
    class Logger;
namespace NintendoSwitch{




class DateReader : public StaticScreenDetector{
public:
    DateReader(ConsoleHandle& console);

    virtual void make_overlays(VideoOverlaySet& items) const override;

    //  Returns true if we are on the date change window.
    virtual bool detect(const ImageViewRGB32& screen) override;


    std::pair<DateFormat, DateTime> read_date(Logger& logger, std::shared_ptr<const ImageRGB32> screen);
    void set_date(
        const ProgramInfo& info, ConsoleHandle& console, ProControllerContext& context,
        const DateTime& date    //  Seconds is ignored.
    );

private:
    ConsoleHandle& m_console;

    DateChangeDetector_Switch1 m_switch1;
    DateChangeDetector_Switch2 m_switch2;

    DateReader_Switch1_US m_switch1_US;
    DateReader_Switch1_EU m_switch1_EU;
    DateReader_Switch1_JP m_switch1_JP;

    DateReader_Switch2_US m_switch2_US;
    DateReader_Switch2_EU m_switch2_EU;
    DateReader_Switch2_JP m_switch2_JP;
};

class DateChangeWatcher : public DetectorToFinder<DateReader>{
public:
    DateChangeWatcher(ConsoleHandle& console, std::chrono::milliseconds duration = std::chrono::milliseconds(250))
         : DetectorToFinder("DateChangeWatcher", duration, console)
    {}
};

// starting from Home screen, change the date to the desired date
// then go back to the home screen
void change_date(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context,
    const DateTime& date
);



}
}
#endif
