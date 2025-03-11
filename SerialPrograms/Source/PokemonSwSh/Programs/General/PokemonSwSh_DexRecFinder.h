/*  Pokedex Recommendation Finder
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_DexRecFinder_H
#define PokemonAutomation_PokemonSwSh_DexRecFinder_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/Options/StringSelectTableOption.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



class DexRecFinder_Descriptor : public SingleSwitchProgramDescriptor{
public:
    DexRecFinder_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};



class DexRecFilters : public GroupOption{
public:
    DexRecFilters();

    OCR::LanguageOCROption LANGUAGE;
    StringSelectOption DESIRED;
    StringSelectTableOption EXCLUSIONS;
};


class DexRecFinder : public SingleSwitchProgramInstance{
public:
    DexRecFinder();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    void read_line(
        bool& found,
        bool& excluded,
        bool& bad_read,
        Logger& logger,
        Language language,
        const ImageViewRGB32& frame,
        const ImageFloatBox& box,
        const std::set<std::string>& desired,
        const std::set<std::string>& exclusions
    );

private:
    StartInGripOrGameOption START_LOCATION;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    DexRecFilters FILTERS;
    MillisecondsOption VIEW_TIME0;

    EventNotificationOption NOTIFICATION_PROGRAM_FINISH;
    EventNotificationsOption NOTIFICATIONS;

    SectionDividerOption m_advanced_options;
    MillisecondsOption ENTER_POKEDEX_TIME0;
    MillisecondsOption BACK_OUT_TIME0;
};

}
}
}
#endif



