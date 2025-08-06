/*  Claim Mystery Gift
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_ClaimMysteryGift_H
#define PokemonAutomation_PokemonSV_ClaimMysteryGift_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/FloatingPointOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonTools/Options/StringSelectOption.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



class ClaimMysteryGift_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ClaimMysteryGift_Descriptor();

    struct Stats;
    // virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class ClaimMysteryGift : public SingleSwitchProgramInstance, public ConfigOption::Listener{
public:
    ~ClaimMysteryGift();
    ClaimMysteryGift();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    virtual void on_config_value_changed(void* object) override;

    std::string start_segment_description();
    std::string end_segment_description();

private:
    OCR::LanguageOCROption LANGUAGE;
    
    enum class StartingPoint{
        NEW_GAME,
        DONE_TUTORIAL,
    };

    EnumDropdownOption<StartingPoint> STARTING_POINT;

    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
