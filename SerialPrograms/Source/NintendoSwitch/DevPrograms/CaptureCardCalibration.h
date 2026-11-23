/*  Capture Card Calibration
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  A developer tool that identifies the capture card in front of the Switch.
 *
 *  The user opens the Switch user-icon "Edit Background" color picker (a grid of flat
 *  color patches) and runs this tool. It grabs one frame from the capture card, samples
 *  the visible color patches, and matches the captured colors against the measured color
 *  distortions in the CommandLineTests repo's CaptureCardCalibration/color_profile_to_transform.json
 *  (see measure_capture_card_colors.py in that folder for how the profiles were measured).
 *
 *  The tool automatically detects which console layout (Switch 1 / Switch 2) and which page
 *  of the color picker (the top "Up" rows or the bottom "Down" rows) the frame shows, so no
 *  layout input is needed. If the captured colors match a known profile, it reports the
 *  capture card's label; otherwise it fits and reports the raw 3x4 color transform so it can
 *  be added as a new profile.
 */

#ifndef PokemonAutomation_NintendoSwitch_CaptureCardCalibration_H
#define PokemonAutomation_NintendoSwitch_CaptureCardCalibration_H

#include "Common/Cpp/Options/FloatingPointOption.h"
#include "GameConsole/ConsoleProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{




class CaptureCardCalibration_Descriptor : public GameConsole::ConsoleProgramDescriptor{
public:
    CaptureCardCalibration_Descriptor();
};




class CaptureCardCalibration : public GameConsole::ConsoleProgramInstance{
public:
    using Descriptor = CaptureCardCalibration_Descriptor;

    CaptureCardCalibration();

    virtual void program(GameConsole::ConsoleProgramEnvironment& env, CancellableScope& scope) override;

private:
    //  A screenshot matches a profile when the clip-aware RMS color error (in 0-255 levels) is
    //  below this. Single-page samples of high-saturation / high-contrast cards can reach ~4.8
    //  because of gamut clipping on the most saturated patches, so the default is generous.
    FloatingPointOption THRESHOLD;
};



}
}
#endif
