/*  Box Sorter Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "Pokemon/Resources/Pokemon_PokemonSlugs.h"
#include "VideoGames/PokemonFRLG/Inference/PokemonFRLG_StatsReader.h"
#include "PokemonFRLG_BoxOrganization.h"
#include "PokemonFRLG_BoxSorterLivingDex.h"
#include "PokemonFRLG_BoxSorter_Tests.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

class Test_BoxSorter_Ordering : public UnitTest{
public:
    Test_BoxSorter_Ordering() : UnitTest("PokemonFRLG::BoxSorter_Ordering") {}

    virtual UnitTestResult run(Logger&, CancellableScope&) const override{
        using namespace Pokemon;
        std::vector<SortingRule> rules{{SortingRuleType::DexNo, false}, {SortingRuleType::Level, false}, {SortingRuleType::Gender, false}};
        auto make_pokemon = [&](uint16_t dex, uint8_t level, StatsHuntGenderFilter gender){
            CollectedPokemonInfo pokemon;
            pokemon.preferences = &rules;
            pokemon.dex_number = dex;
            pokemon.level = level;
            pokemon.gender = gender;
            return std::optional<CollectedPokemonInfo>(pokemon);
        };
        const auto low = make_pokemon(1, 5, StatsHuntGenderFilter::Male);
        const auto high = make_pokemon(1, 100, StatsHuntGenderFilter::Male);
        const auto female = make_pokemon(1, 100, StatsHuntGenderFilter::Female);
        const auto genderless = make_pokemon(1, 100, StatsHuntGenderFilter::Genderless);
        const auto next_dex = make_pokemon(2, 1, StatsHuntGenderFilter::Male);
        std::vector<std::optional<CollectedPokemonInfo>> slots{std::nullopt, next_dex, genderless, female, high, low, low, std::nullopt};
        std::stable_sort(slots.begin(), slots.end());
        const std::vector<std::optional<CollectedPokemonInfo>> expected{low, low, high, female, genderless, next_dex, std::nullopt, std::nullopt};
        if (slots != expected){
            return "Dex/level/gender sorting did not preserve duplicates and move empty slots to the end.";
        }
        if (low == high){
            return "Pokemon with different levels must not compare equal during swap matching.";
        }
        rules = {{SortingRuleType::Level, true}, {SortingRuleType::Gender, true}};
        slots = {low, high, female, genderless, next_dex, std::nullopt};
        std::stable_sort(slots.begin(), slots.end());
        if (slots != std::vector<std::optional<CollectedPokemonInfo>>{genderless, female, high, low, next_dex, std::nullopt}){
            return "Reversed level/gender sorting failed.";
        }
        BoxSortingTable table("Sort rules", {SortingRuleType::DexNo, SortingRuleType::Level, SortingRuleType::Gender, SortingRuleType::Shiny});
        if (!table.database().find(SortingRuleType::Level) || !table.database().find(SortingRuleType::Shiny)){
            return "FRLG sorting table must offer level and shiny sorting.";
        }
        auto shiny = high;
        shiny->shiny = true;
        if (shiny == high){
            return "Shiny and non-shiny duplicates must not compare equal during swap matching.";
        }
        rules = {{SortingRuleType::Shiny, false}, {SortingRuleType::Level, false}};
        slots = {low, std::nullopt, high, shiny};
        std::stable_sort(slots.begin(), slots.end());
        if (slots != std::vector<std::optional<CollectedPokemonInfo>>{shiny, low, high, std::nullopt}){
            return "Shiny sorting must put shinies first and empty slots last.";
        }
        rules[0].reverse = true;
        std::stable_sort(slots.begin(), slots.end());
        if (slots != std::vector<std::optional<CollectedPokemonInfo>>{low, high, shiny, std::nullopt}){
            return "Reversed shiny sorting must put non-shinies first and empty slots last.";
        }
        if (SLUGS_TO_NATIONAL_DEX().at("bulbasaur") != 1 || SLUGS_TO_NATIONAL_DEX().at("deoxys") != 386){
            return "Species conversion must use National Dex numbers.";
        }
        const BoxCursor cursor(30);
        if (cursor.box != 1 || cursor.row != 0 || cursor.column != 0 || to_global_index(cursor.box, cursor.row, cursor.column) != 30){
            return "Box cursor must advance to the next box after 30 slots.";
        }
        return true;
    }
};

class Test_BoxSorter_Chains : public UnitTest{
public:
    Test_BoxSorter_Chains() : UnitTest("PokemonFRLG::BoxSorter_Chains") {}

    virtual UnitTestResult run(Logger&, CancellableScope&) const override{
        using namespace Pokemon;
        std::vector<SortingRule> rules{{SortingRuleType::DexNo, false}, {SortingRuleType::Level, false}, {SortingRuleType::Shiny, false}};
        auto pokemon = [&](uint16_t dex, uint8_t level){
            CollectedPokemonInfo info;
            info.preferences = &rules;
            info.dex_number = dex;
            info.level = level;
            return std::optional<CollectedPokemonInfo>(info);
        };
        auto shiny = pokemon(1, 5);
        shiny->shiny = true;
        const std::vector<std::optional<CollectedPokemonInfo>> choices{std::nullopt, pokemon(1, 5), pokemon(1, 10), pokemon(2, 1), shiny};

        auto verify = [&](const std::vector<std::optional<CollectedPokemonInfo>>& original) -> std::string{
            auto sorted = original;
            std::stable_sort(sorted.begin(), sorted.end());
            const BoxSortingPlan plan = make_box_sorting_plan(original, sorted);
            // Simulate A presses with unique identities so duplicates cannot hide
            // a Pokemon being placed twice or left on the hand.
            std::vector<std::optional<size_t>> slots(original.size());
            std::vector<bool> placed(original.size(), false);
            for (size_t i = 0; i < original.size(); i++){
                if (original[i]){
                    slots[i] = i;
                }
            }
            for (const auto& chain : plan.chains){
                if (chain.size() < 2 || chain.front() >= slots.size() || !slots[chain.front()]){
                    return "A chain must start by picking up a Pokemon.";
                }
                auto held = slots[chain.front()];
                slots[chain.front()].reset();
                for (size_t step = 1; step < chain.size(); step++){
                    const size_t destination = chain[step];
                    if (!held || destination >= slots.size()){
                        return "Invalid placement in chain.";
                    }
                    if (placed[*held] || original[*held] != sorted[destination]){
                        return "Every Pokemon must be placed exactly once, directly in its final slot.";
                    }
                    placed[*held] = true;
                    std::swap(held, slots[destination]);
                }
                if (held){
                    return "A chain left a Pokemon on the hand.";
                }
            }
            for (size_t i = 0; i < original.size(); i++){
                const auto actual = slots[i] ? original[*slots[i]] : std::nullopt;
                if (actual != sorted[i]){
                    return "Chain execution did not produce the sorted layout.";
                }
                if (original[i] && placed[i] != (original[i] != sorted[i])){
                    return "Only Pokemon outside their final slots should be moved.";
                }
            }
            return "";
        };

        // Exhaust all layouts of up to six slots, with gaps and duplicates,
        // under both ascending and descending rules.
        for (bool reverse : {false, true}){
            for (SortingRule& rule : rules){
                rule.reverse = reverse;
            }
            size_t layouts = 1;
            for (size_t count = 0; count <= 6; count++, layouts *= choices.size()){
                for (size_t layout = 0; layout < layouts; layout++){
                    std::vector<std::optional<CollectedPokemonInfo>> original(count);
                    size_t digits = layout;
                    for (auto& slot : original){
                        slot = choices[digits % choices.size()];
                        digits /= choices.size();
                    }
                    const std::string error = verify(original);
                    if (!error.empty()){
                        return error;
                    }
                }
            }
        }
        for (SortingRule& rule : rules){
            rule.reverse = false;
        }
        const std::vector<std::optional<CollectedPokemonInfo>> open_chain{std::nullopt, choices[3], choices[1], choices[2]};
        auto sorted = open_chain;
        std::stable_sort(sorted.begin(), sorted.end());
        if (make_box_sorting_plan(open_chain, sorted).chains != std::vector<std::vector<size_t>>{{3, 1, 2, 0}}){
            return "An open chain should use one pickup, starting in a slot that must end empty.";
        }
        std::vector<std::optional<CollectedPokemonInfo>> across_boxes(65);
        for (size_t i = 0; i < across_boxes.size(); i++){
            across_boxes[i] = choices[(across_boxes.size() - i) % choices.size()];
        }
        const std::string error = verify(across_boxes);
        return error.empty() ? UnitTestResult(true) : UnitTestResult(error);
    }
};


class Test_BoxSorter_LivingDex : public UnitTest{
public:
    Test_BoxSorter_LivingDex() : UnitTest("PokemonFRLG::BoxSorter_LivingDex") {}

    virtual UnitTestResult run(Logger& logger, CancellableScope&) const override{
        using namespace Pokemon;
        const std::vector<SortingRule> rules{{SortingRuleType::DexNo, false}};
        auto pokemon = [&](uint16_t dex, uint8_t level, StatsHuntGenderFilter gender = StatsHuntGenderFilter::Male){
            CollectedPokemonInfo info;
            info.preferences = &rules;
            info.dex_number = dex;
            info.level = level;
            info.gender = gender;
            return std::optional<CollectedPokemonInfo>(info);
        };
        const auto male = pokemon(3, 5);
        const auto female = pokemon(3, 10, StatsHuntGenderFilter::Female);
        const LivingDexEntry male_entry{3, "venusaur-male", true};
        const LivingDexEntry female_entry{3, "venusaur-female", true};
        if (!is_viable_for_dex(male_entry, *female, false) ||
            is_viable_for_dex(male_entry, *female, true) ||
            !is_viable_for_dex(male_entry, *male, true) ||
            is_viable_for_dex(female_entry, *male, true) ||
            !is_viable_for_dex(female_entry, *female, true) ||
            is_viable_for_dex(male_entry, *pokemon(1, 5), false) ||
            is_viable_for_dex(male_entry, *pokemon(3, 5, StatsHuntGenderFilter::Genderless), true) ||
            !is_viable_for_dex({3, "venusaur-male", false}, *female, true)
        ){
            return "Living dex species/gender eligibility failed.";
        }
        const auto deoxys = pokemon(386, 50);
        if (!is_viable_for_dex({386, "deoxys-normal-forme", false}, *deoxys, true)){
            return "Condensed living dex should ignore forms.";
        }

        auto verify_chains = [&](const std::vector<std::optional<CollectedPokemonInfo>>& original,
                                 const std::vector<std::optional<CollectedPokemonInfo>>& ordered) -> bool{
            auto actual = original;
            for (const auto& chain : make_box_sorting_plan(original, ordered).chains){
                if (chain.size() < 2 || chain.front() >= actual.size() || !actual[chain.front()]){
                    return false;
                }
                auto held = actual[chain.front()];
                actual[chain.front()].reset();
                for (size_t i = 1; i < chain.size(); i++){
                    if (!held || chain[i] >= actual.size() || held != ordered[chain[i]]){
                        return false;
                    }
                    std::swap(held, actual[chain[i]]);
                }
                if (held){
                    return false;
                }
            }
            return actual == ordered;
        };
        const auto bulbasaur = pokemon(1, 5);
        const auto duplicate = pokemon(1, 10);
        const auto extra = pokemon(150, 70);
        const std::vector<LivingDexEntry> order{{1, "bulbasaur", false}, {2, "ivysaur", false}, male_entry};
        const std::vector<std::optional<CollectedPokemonInfo>> original{extra, male, female, duplicate, bulbasaur, std::nullopt};
        const std::vector<std::optional<CollectedPokemonInfo>> expected{duplicate, std::nullopt, female, extra, male, bulbasaur};
        const auto ordered = make_living_dex_order(logger, original, order, false);
        if (ordered != expected || !verify_chains(original, ordered)){
            return "Living dex must reserve viable slots, leave missing entries empty, and preserve extras through chains.";
        }
        if (make_missing_living_dex_report(ordered, order) != "2 Ivysaur\n"){
            return "Missing Pokemon report must list only empty template slots with their dex numbers and English names.";
        }
        const std::vector<LivingDexEntry> report_order{
            {386, "deoxys-normal-forme", false}, {25, "pikachu", false},
            {1, "bulbasaur", false}, {150, "mewtwo", false}
        };
        if (make_missing_living_dex_report({std::nullopt, std::nullopt, std::nullopt, extra, std::nullopt}, report_order) !=
            "1 Bulbasaur\n25 Pikachu\n386 Deoxys\n"){
            return "Missing Pokemon report must sort numerically, use species names for forms, and ignore empty extra slots.";
        }
        const auto gender_ordered = make_living_dex_order(logger, original, order, true);
        if (gender_ordered[2] != male || gender_ordered[1] || !verify_chains(original, gender_ordered)){
            return "Gender-restricted living dex chain placement failed.";
        }
        if (!make_box_sorting_plan(ordered, make_living_dex_order(logger, ordered, order, false)).chains.empty()){
            return "An organized living dex should not move Pokemon again.";
        }
        auto shiny_bulbasaur = bulbasaur;
        shiny_bulbasaur->shiny = true;
        auto shiny_female = female;
        shiny_female->shiny = true;
        if (is_viable_for_dex(order[0], *bulbasaur, false, true) ||
            !is_viable_for_dex(order[0], *shiny_bulbasaur, false, true) ||
            is_viable_for_dex(male_entry, *shiny_female, true, true)){
            return "Shiny dex eligibility must require shininess as well as species/gender.";
        }
        const std::vector<std::optional<CollectedPokemonInfo>> shiny_original{
            bulbasaur, std::nullopt, shiny_female, duplicate, shiny_bulbasaur, male, extra, std::nullopt
        };
        const std::vector<std::optional<CollectedPokemonInfo>> shiny_expected{
            shiny_bulbasaur, std::nullopt, shiny_female, bulbasaur, duplicate, male, extra, std::nullopt
        };
        const auto shiny_ordered = make_living_dex_order(logger, shiny_original, order, false, true);
        if (shiny_ordered != shiny_expected || !verify_chains(shiny_original, shiny_ordered) ||
            !make_box_sorting_plan(shiny_ordered, make_living_dex_order(logger, shiny_ordered, order, false, true)).chains.empty()){
            return "Shiny dex must keep placed shinies, leave gaps, preserve all extras, and be stable on a second run.";
        }
        auto non_shiny_boxes = original;
        non_shiny_boxes.resize(8);
        const auto missing_shinies = make_living_dex_order(logger, non_shiny_boxes, order, false, true);
        if (missing_shinies[0] || missing_shinies[1] || missing_shinies[2] ||
            !verify_chains(non_shiny_boxes, missing_shinies)){
            return "Missing shinies must leave template slots empty while preserving non-shinies as extras.";
        }
        if (make_missing_living_dex_report(missing_shinies, order) != "1 Bulbasaur\n2 Ivysaur\n3 Venusaur\n"){
            return "Missing Pokemon report must include missing shinies even when non-shiny extras exist.";
        }
        try{
            make_living_dex_order(logger, {bulbasaur}, order, false);
            return "An undersized living dex box range should fail.";
        }catch (const UserSetupError&){}
        try{
            make_living_dex_order(logger, {bulbasaur, duplicate, extra}, order, false);
            return "Living dex overflow must fail rather than discard Pokemon or fill missing entries.";
        }catch (const UserSetupError&){}

        const auto condensed = load_living_dex_template(
            RESOURCE_PATH() + "PokemonFRLG/Box/DexTemplates/frlg_living_dex_condensed.json"
        );
        if (condensed.size() != 214 || condensed.front().nat_id != 1 || condensed.back().nat_id != 386){
            return "FRLG condensed template was not loaded in file order.";
        }
        // Include every template species in reverse order, plus a duplicate and an
        // unrelated species; simulate the shared chain executor across eight boxes.
        std::vector<std::optional<CollectedPokemonInfo>> boxes(240);
        for (size_t i = 0; i < condensed.size(); i++){
            boxes[i] = pokemon(condensed[condensed.size() - i - 1].nat_id, 5, StatsHuntGenderFilter::Female);
        }
        boxes[238] = boxes[0];
        boxes[239] = pokemon(252, 5);
        const auto dex_boxes = make_living_dex_order(logger, boxes, condensed, false);
        if (!make_missing_living_dex_report(dex_boxes, condensed).empty()){
            return "A complete living dex must produce a report with no entries.";
        }
        for (size_t i = 0; i < condensed.size(); i++){
            if (!dex_boxes[i] || !is_viable_for_dex(condensed[i], *dex_boxes[i], false)){
                return "Condensed template placement did not follow its species order.";
            }
        }
        if (std::count_if(boxes.begin(), boxes.end(), [](const auto& slot){ return slot.has_value(); }) !=
            std::count_if(dex_boxes.begin(), dex_boxes.end(), [](const auto& slot){ return slot.has_value(); }) ||
            !verify_chains(boxes, dex_boxes)
        ){
            return "Living dex chain execution lost Pokemon or failed across boxes.";
        }
        return true;
    }
};

void add_tests_BoxSorter(UnitTestDatabase& database){
    database.add<Test_BoxSorter_Ordering>();
    database.add<Test_BoxSorter_Chains>();
    database.add<Test_BoxSorter_LivingDex>();
}


}
}
}
