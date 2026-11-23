/*  Box Sorter Living Dex
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_BoxSorterLivingDex_H
#define PokemonAutomation_PokemonFRLG_BoxSorterLivingDex_H

#include <cstdint>
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{ class Logger; }

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

struct BoxOrganizationStats;

class BoxSorterLivingDex_Descriptor : public SingleSwitchProgramDescriptor{
public:
    BoxSorterLivingDex_Descriptor();

    using Stats = BoxOrganizationStats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class BoxSorterLivingDex : public SingleSwitchProgramInstance{
public:
    using Descriptor = BoxSorterLivingDex_Descriptor;
    BoxSorterLivingDex();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;
    virtual void start_program_border_check(VideoStream&, FeedbackType) override{}

private:
    enum class LivingDexTemplate{
        Condensed,
    };

    OCR::LanguageOCROption LANGUAGE;
    SimpleIntegerOption<uint16_t> NUM_BOXES;
    EnumDropdownOption<LivingDexTemplate> LIVING_DEX_TEMPLATE;
    BooleanCheckBoxOption SHINY_DEX;
    BooleanCheckBoxOption DRY_RUN;
    EventNotificationsOption NOTIFICATIONS;
};



}
}
}
#endif
