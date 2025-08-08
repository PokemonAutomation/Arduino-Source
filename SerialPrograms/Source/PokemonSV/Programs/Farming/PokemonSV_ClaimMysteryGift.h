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
#include "Common/Cpp/Options/TextEditOption.h"
#include "PokemonSV/Programs/FastCodeEntry/PokemonSV_CodeEntry.h"

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

    void run_autostory_until_pokeportal_unlocked(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    void enter_mystery_gift_via_internet_window(SingleSwitchProgramEnvironment& env, ProControllerContext& context, int menu_index);
    void claim_internet_mystery_gift(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    void enter_mystery_gift_code_window(SingleSwitchProgramEnvironment& env, ProControllerContext& context, int menu_index);
    void enter_mystery_gift_code(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

private:
    virtual void on_config_value_changed(void* object) override;

    std::string start_segment_description();
    std::string end_segment_description();

private:
    OCR::LanguageOCROption LANGUAGE;
    
    enum class StartingPoint{
        NEW_GAME,
        IN_MYSTERY_GIFT_CODE_WINDOW,
        DONE_TUTORIAL,
    };

        enum class ObtainingMethod{
        VIA_INTERNET_ALL,
        VIA_INTERNET_NONE,
        VIA_CODE,
    };

    EnumDropdownOption<StartingPoint> STARTING_POINT;
    EnumDropdownOption<ObtainingMethod> OBTAINING_METHOD;
    StaticTextOption MYSTERY_GIFT_NOTE;
    StaticTextOption MULTISWITCH_NOTE;
    TextEditOption CODE;
    FastCodeEntrySettingsOption SETTINGS;

    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};







}
}
}
#endif
