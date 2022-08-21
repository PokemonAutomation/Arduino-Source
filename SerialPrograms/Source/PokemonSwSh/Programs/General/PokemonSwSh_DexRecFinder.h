/*  Pokedex Recommendation Finder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_DexRecFinder_H
#define PokemonAutomation_PokemonSwSh_DexRecFinder_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/TimeExpressionOption.h"
#include "CommonFramework/Options/LanguageOCROption.h"
#include "CommonFramework/Options/ScreenshotFormatOption.h"
#include "CommonFramework/Options/EditableTableOption.h"
#include "CommonFramework/Options/StringSelectTableOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "Pokemon/Options/Pokemon_NameListOption.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

using namespace Pokemon;


class DexRecFinder_Descriptor : public SingleSwitchProgramDescriptor{
public:
    DexRecFinder_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};



class DexRecFilters : public GroupOption{
public:
    DexRecFilters();

    OCR::LanguageOCR LANGUAGE;
    StringSelectOption DESIRED;
    StringSelectTableOption EXCLUSIONS;
};


class DexRecFinder : public SingleSwitchProgramInstance{
public:
    DexRecFinder();
    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

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
    TimeExpressionOption<uint16_t> VIEW_TIME;

    EventNotificationOption NOTIFICATION_PROGRAM_FINISH;
    EventNotificationsOption NOTIFICATIONS;

    SectionDividerOption m_advanced_options;
    TimeExpressionOption<uint16_t> ENTER_POKEDEX_TIME;
    TimeExpressionOption<uint16_t> BACK_OUT_TIME;
};

}
}
}
#endif



