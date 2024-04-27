/*  Material Farmer - Happiny dust
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_MaterialFarmer_H
#define PokemonAutomation_PokemonSV_MaterialFarmer_H

#include <functional>
#include "Common/Cpp/Options/FloatingPointOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "CommonFramework/Options/LanguageOCROption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "PokemonSV/Options/PokemonSV_EncounterBotCommon.h"
#include "PokemonSV/Options/PokemonSV_SandwichMakerOption.h"
#include "PokemonSV_MaterialFarmerTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


class MaterialFarmer : public SingleSwitchProgramInstance{
public:
    MaterialFarmer();

    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    MaterialFarmerOptions MATERIAL_FARMER_OPTIONS;

};





}
}
}
#endif
