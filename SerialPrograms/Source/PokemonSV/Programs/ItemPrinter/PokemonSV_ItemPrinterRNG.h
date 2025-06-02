/*  Item Printer RNG
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_ItemPrinterRNG_H
#define PokemonAutomation_PokemonSV_ItemPrinterRNG_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonTools/Options/LanguageOCROption.h"
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

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;


private:
    enum class MaterialFarmerTrigger{
        FIXED_NUM_PRINT_JOBS,
        MINIMUM_HAPPINY_DUST,
    };

    enum class ItemPrinterMode{
        STANDARD_MODE,
        AUTO_MODE,
    };

    virtual void on_config_value_changed(void* object) override;

    bool overlapping_bonus();

    void run_item_printer_rng_automode(SingleSwitchProgramEnvironment& env, ProControllerContext& context, ItemPrinterRNG_Descriptor::Stats& stats);

    void run_item_printer_rng(SingleSwitchProgramEnvironment& env, ProControllerContext& context, ItemPrinterRNG_Descriptor::Stats& stats);

    std::vector<ItemPrinterRngRowSnapshot> desired_print_table(
        ItemPrinter::PrebuiltOptions desired_item,
        uint16_t quantity_to_print
    );

    // return Ball bonus or item bonus, based on the desired_item
    // if the desired_item is a type of ball, return Ball Bonus, else return Item Bonus
    ItemPrinter::PrebuiltOptions get_bonus_type(ItemPrinter::PrebuiltOptions desired_item);

    int16_t check_obtained_quantity(std::map<std::string, uint16_t> obtained_prizes, std::string desired_slug);

    // move from item printer to material farming, run material farmer, then 
    // return to item printer
    void run_material_farming_then_return_to_item_printer(
        SingleSwitchProgramEnvironment& env, 
        ProControllerContext& context, 
        ItemPrinterRNG_Descriptor::Stats& stats,
        MaterialFarmerOptions& material_farmer_options
    );

    ItemPrinterPrizeResult run_print_at_date(
        SingleSwitchProgramEnvironment& env, ProControllerContext& context,
        const DateTime& date, ItemPrinterJobs jobs
    );

    void print_again(
        SingleSwitchProgramEnvironment& env, ProControllerContext& context,
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

    uint32_t calc_num_jobs_using_happiny_dust(
        SingleSwitchProgramEnvironment& env, 
        ProControllerContext& context,
        uint16_t min_happiny_dust
    );

    uint32_t check_num_happiny_dust(
        SingleSwitchProgramEnvironment& env, ProControllerContext& context
    );

private:
    OCR::LanguageOCROption LANGUAGE;
    SimpleIntegerOption<uint16_t> NUM_ITEM_PRINTER_ROUNDS;

    StaticTextOption OVERLAPPING_BONUS_WARNING;
    EnumDropdownOption<ItemPrinterMode> MODE;
    ItemPrinterDesiredItemTable DESIRED_ITEM_TABLE;
    ItemPrinterRngTable DATE_SEED_TABLE;

    SimpleIntegerOption<uint16_t> DELAY_MILLIS;
    BooleanCheckBoxOption ADJUST_DELAY;

    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    BooleanCheckBoxOption FIX_TIME_WHEN_DONE;

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



