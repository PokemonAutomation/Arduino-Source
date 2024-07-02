/*  Item Printer RNG
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_ItemPrinterRNG_H
#define PokemonAutomation_PokemonSV_ItemPrinterRNG_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Options/LanguageOCROption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "PokemonSV/Programs/Farming/PokemonSV_MaterialFarmerTools.h"
#include "PokemonSV_ItemPrinterTools.h"
#include "PokemonSV_ItemPrinterRNGTable.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


class ItemPrinterRNG_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ItemPrinterRNG_Descriptor();
    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class ItemPrinterRNG : public SingleSwitchProgramInstance, public ConfigOption::Listener{
public:
    ~ItemPrinterRNG();
    ItemPrinterRNG();

    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;


private:
    virtual void value_changed(void* object) override;

    bool overlapping_bonus();

    void run_item_printer_rng(SingleSwitchProgramEnvironment& env, BotBaseContext& context, ItemPrinterRNG_Descriptor::Stats& stats);

    void run_print_at_date(
        SingleSwitchProgramEnvironment& env, BotBaseContext& context,
        const DateTime& date, ItemPrinterJobs jobs
    );

    void print_again(
        SingleSwitchProgramEnvironment& env, BotBaseContext& context,
        ItemPrinterJobs jobs
    ) const;

    void adjust_delay(
        Logger& logger,
        SingleSwitchProgramEnvironment& env,
        const std::array<std::string, 10>& print_results, 
        int distance_from_target
    );

    int get_distance_from_target(
        Logger& logger,
        ItemPrinterRNG_Descriptor::Stats& stats,
        const std::array<std::string, 10>& print_results,
        uint64_t seed
    );

    bool results_approximately_match(
        const std::array<std::string, 10>& print_results, 
        const std::array<std::string, 10>& expected_result
    );

    uint32_t calc_num_jobs_with_happiny_dust(
        SingleSwitchProgramEnvironment& env, BotBaseContext& context
    );

    uint32_t check_num_happiny_dust(
        SingleSwitchProgramEnvironment& env, BotBaseContext& context
    );

private:
    OCR::LanguageOCROption LANGUAGE;
    SimpleIntegerOption<uint16_t> NUM_ITEM_PRINTER_ROUNDS;
//    StaticTextOption AFTER_ITEM_PRINTER_DONE_EXPLANATION;

    StaticTextOption OVERLAPPING_BONUS_WARNING;
    ItemPrinterRngTable TABLE0;

    SimpleIntegerOption<uint16_t> DELAY_MILLIS;
    BooleanCheckBoxOption ADJUST_DELAY;

    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    BooleanCheckBoxOption FIX_TIME_WHEN_DONE;

//    BooleanCheckBoxOption AUTO_MATERIAL_FARMING;
//    SimpleIntegerOption<uint16_t> NUM_ROUNDS_OF_ITEM_PRINTER_TO_MATERIAL_FARM;
    
//    StaticTextOption MATERIAL_FARMER_DISABLED_EXPLANATION;
    enum class MaterialFarmerTrigger{
        FIXED_NUM_PRINT_JOBS,
        MINIMUM_HAPPINY_DUST,
    };
    EnumDropdownOption<MaterialFarmerTrigger> MATERIAL_FARMER_TRIGGER;
    SimpleIntegerOption<uint16_t> MATERIAL_FARMER_FIXED_NUM_JOBS;
    SimpleIntegerOption<uint16_t> MIN_HAPPINY_DUST;
    MaterialFarmerOptions MATERIAL_FARMER_OPTIONS;

    BooleanCheckBoxOption ENABLE_SEED_CALC;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};


}
}
}
#endif



