/*  Stats Reset
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_StatsReset_H
#define PokemonAutomation_PokemonSwSh_StatsReset_H

#include "CommonFramework/Options/BooleanCheckBox.h"
#include "CommonFramework/Options/EnumDropdown.h"
#include "CommonFramework/Options/LanguageOCR.h"
#include "PokemonSwSh/Inference/PokemonSwSh_IVCheckerReader.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class IVCheckerOption : public EnumDropdown{
public:
    IVCheckerOption(QString label, size_t default_index = 0)
        : EnumDropdown(
            std::move(label),
            {
                "Don't Care (0-31)",
                "No Good (0)",
                "Decent (0-15)",
                "Pretty Good (16-25)",
                "Very Good (26-29)",
                "Fantastic (30)",
                "Best (31)",
            },
            default_index
        )
    {}

    bool matches(uint64_t& errors, IVCheckerReader::Result result) const{
        if (result == IVCheckerReader::Result::UnableToDetect){
            errors++;
        }
        size_t desired = (size_t)*this;
        if (desired == 0){
            return true;
        }
        if (desired == (size_t)result){
            return true;
        }
        return false;
    }
};



class StatsReset_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    StatsReset_Descriptor();
};



class StatsReset : public SingleSwitchProgramInstance{
public:
    StatsReset(const StatsReset_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    struct Stats;

private:
    IVCheckerReader m_iv_checker_reader;

    BooleanCheckBox GO_HOME_WHEN_DONE;
    LanguageOCR LANGUAGE;
    EnumDropdown POKEMON;
    IVCheckerOption HP;
    IVCheckerOption ATTACK;
    IVCheckerOption DEFENSE;
    IVCheckerOption SPATK;
    IVCheckerOption SPDEF;
    IVCheckerOption SPEED;
};


}
}
}
#endif
