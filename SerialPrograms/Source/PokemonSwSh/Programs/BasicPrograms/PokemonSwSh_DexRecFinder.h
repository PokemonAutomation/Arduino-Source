/*  Pokedex Recommendation Finder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_DexRecFinder_H
#define PokemonAutomation_PokemonSwSh_DexRecFinder_H

#include "CommonFramework/Options/SectionDivider.h"
#include "CommonFramework/Options/BooleanCheckBox.h"
#include "CommonFramework/Options/LanguageOCR.h"
#include "Pokemon/Options/Pokemon_NameSelect.h"
#include "Pokemon/Pokemon_NameReader.h"
#include "NintendoSwitch/Options/TimeExpression.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class DexRecFinder_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    DexRecFinder_Descriptor();
};



class DexRecFinder : public SingleSwitchProgramInstance{
public:
    DexRecFinder(const DexRecFinder_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    void read_line(
        bool& found,
        bool& bad_read,
        Logger& logger,
        const QImage& frame,
        const InferenceBox& box,
        const std::set<std::string>& desired
    );

private:
    struct Stats;

private:
    Pokemon::PokemonNameReader m_name_reader;

    BooleanCheckBox GO_HOME_WHEN_DONE;

    LanguageOCR LANGUAGE;
    Pokemon::PokemonNameSelect DESIRED;
    TimeExpression<uint16_t> VIEW_TIME;

    SectionDivider m_advanced_options;
    TimeExpression<uint16_t> ENTER_POKEDEX_TIME;
    TimeExpression<uint16_t> BACK_OUT_TIME;
};

}
}
}
#endif



