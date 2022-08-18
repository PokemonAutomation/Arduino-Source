/*  Encounter Filter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_EncounterFilterEnums_H
#define PokemonAutomation_PokemonSwSh_EncounterFilterEnums_H

#include <string>
#include <vector>
#include <map>
#include "Common/Cpp/Options/DropdownOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



enum class EncounterAction{
    StopProgram,
    RunAway,
    ThrowBalls,
    ThrowBallsAndSave,
};
extern const std::vector<std::string> EncounterAction_NAMES;
extern const std::map<std::string, EncounterAction> EncounterAction_MAP;

class EncounterActionCell : public DropdownCell{
public:
    EncounterActionCell()
        : DropdownCell(
            {
                "Stop Program",
                "Run Away",
                "Throw balls.",
                "Throw balls. Save if caught.",
            }, 0
        )
    {}
    operator EncounterAction() const{
        return (EncounterAction)current_index();
    }
    void set(EncounterAction action){
        DropdownCell::set_index((size_t)action);
    }
};



enum class ShinyFilter{
    ANYTHING,
    NOT_SHINY,
    ANY_SHINY,
    STAR_ONLY,
    SQUARE_ONLY,
    NOTHING,
};
extern const std::vector<std::string> ShinyFilter_NAMES;
extern const std::map<std::string, ShinyFilter> ShinyFilter_MAP;



class ShinyFilterCell : public DropdownCell{
public:
    PA_NO_INLINE ShinyFilterCell(bool rare_stars)
        : DropdownCell(
            rare_stars
                ? std::vector<std::string>{
                    "Not Shiny",
                    "Square Shiny",
                }
                : std::vector<std::string>{
                    "Anything",
                    "Not Shiny",
                    "Any Shiny",
                    "Star Shiny",
                    "Square Shiny",
                    "Nothing",
                },
//            rare_stars ? 0 : 1
            no_opt_wrap(rare_stars) //  COMPILER-BUG: MSVC2019
        )
        , m_rare_stars(rare_stars)
    {}


    static PA_NO_INLINE size_t no_opt_wrap(bool& x){
    //    cout << "no_opt_wrap = " << x << endl;
        return x ? 0 : 1;
    }

    operator ShinyFilter() const;
    void set(ShinyFilter action);

private:
    using DropdownCell::set_index;

private:
    bool m_rare_stars;
};






}
}
}
#endif
