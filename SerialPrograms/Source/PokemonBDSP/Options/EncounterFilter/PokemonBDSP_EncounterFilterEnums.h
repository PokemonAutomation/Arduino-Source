/*  Encounter Filter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_EncounterFilterEnums_H
#define PokemonAutomation_PokemonBDSP_EncounterFilterEnums_H

#include <string>
#include <vector>
#include <map>
#include "Common/Cpp/Options/EnumDropdownOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


enum class EncounterAction{
    StopProgram,
    RunAway,
    ThrowBalls,
    ThrowBallsAndSave,
};
extern const std::vector<std::string> EncounterAction_NAMES;
extern const std::map<std::string, EncounterAction> EncounterAction_MAP;

class EncounterActionCell : public EnumDropdownCell{
public:
    EncounterActionCell()
        : EnumDropdownCell(
            {
                "Stop Program",
                "Run Away",
                "Throw balls.",
                "Throw balls. Save if caught.",
            }, 0
        )
    {}
    operator EncounterAction() const{
        return (EncounterAction)(size_t)*this;
    }
    void set(EncounterAction action){
        EnumDropdownCell::set((size_t)action);
    }
};




enum class ShinyFilter{
    ANYTHING,
    NOT_SHINY,
    SHINY,
    NOTHING,
};
extern const std::vector<std::string> ShinyFilter_NAMES;
extern const std::map<std::string, ShinyFilter> ShinyFilter_MAP;

class ShinyFilterCell : public EnumDropdownCell{
public:
    ShinyFilterCell()
        : EnumDropdownCell(
            {
                "Anything",
                "Not Shiny",
                "Shiny",
                "Nothing",
            },
            1
        )
    {}
    operator ShinyFilter() const{
        return (ShinyFilter)(size_t)*this;
    }
    void set(ShinyFilter action){
        EnumDropdownCell::set((size_t)action);
    }
};




}
}
}
#endif
