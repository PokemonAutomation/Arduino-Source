/*  Mass Attach Items
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_MassAttachItems_H
#define PokemonAutomation_PokemonSV_MassAttachItems_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

class BoxDetector;


class MassAttachItems_Descriptor : public SingleSwitchProgramDescriptor{
public:
    MassAttachItems_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class MassAttachItems : public SingleSwitchProgramInstance{
public:
    MassAttachItems();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    void attach_one(BoxDetector& box_detector, SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    void attach_box(BoxDetector& box_detector, SingleSwitchProgramEnvironment& env, ProControllerContext& context);

private:
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    enum class ItemCategory{
        Medicines,
        PokeBalls,
        BattleItems,
        Berries,
        OtherItems,
        TMs,
        Treasures,
    };
    EnumDropdownOption<ItemCategory> ITEM_CATEGORY;

    SimpleIntegerOption<uint8_t> BOXES;
    EventNotificationsOption NOTIFICATIONS;
};


}
}
}
#endif



