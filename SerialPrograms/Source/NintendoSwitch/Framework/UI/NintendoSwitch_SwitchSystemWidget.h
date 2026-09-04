/*  Switch System
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  This is the Qt Widget implementation of the UI for SwitchSystemSession.
 *
 *  On construction, this class attaches itself to the session it is constructed
 *  with and automatically detaches on destruction. Therefore, this class must
 *  not outlive the session it is constructed with. While not useful, it is also
 *  safe to construct multiple UI classes attached to the same session.
 *
 *  Modifications directly to the session object will automatically update this
 *  UI class. For example, if you use Discord to change the volume of the
 *  audio playback, it will move the slider as shown by this UI.
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_SwitchSystemWidget_H
#define PokemonAutomation_NintendoSwitch_SwitchSystemWidget_H

#include "NintendoSwitch/Framework/NintendoSwitch_SwitchSystemSession.h"
#include "ConsoleInfra/ConsoleSystemWidget.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class SwitchSystemWidget final : public ConsoleInfra::ConsoleSystemWidget{
public:
    using ParentState = SwitchSystemSession;

public:
    SwitchSystemWidget(
        QWidget& parent,
        SwitchSystemSession& session
    );
};




}
}
#endif
