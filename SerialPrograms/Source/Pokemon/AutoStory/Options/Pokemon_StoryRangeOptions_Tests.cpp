/*  Story Range Options Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <format>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include "Common/Cpp/TestRunners/UnitTest.h"
#include "Pokemon/AutoStory/Pokemon_AutoStorySelect.h"
#include "Pokemon_StoryRangeOptions.h"
#include "Pokemon_StoryRangeOptions_Tests.h"

namespace PokemonAutomation{
namespace Pokemon{

namespace{

//  A stand-in for a story checkpoint: only its name is read.
struct NamedCheckpoint{
    std::string label;
    std::string name() const{
        return label;
    }
};

//  Two tutorial checkpoints, then three of the main story, as SV's list is
//  laid out.
const std::vector<std::unique_ptr<NamedCheckpoint>>& story_list(){
    static const std::vector<std::unique_ptr<NamedCheckpoint>> list = []{
        std::vector<std::unique_ptr<NamedCheckpoint>> ret;
        for (std::string_view label : {"tutorial 0", "tutorial 1", "main 0", "main 1", "main 2"}){
            ret.emplace_back(std::make_unique<NamedCheckpoint>(NamedCheckpoint{.label = std::string(label)}));
        }
        return ret;
    }();
    return list;
}

//  The main story's dropdowns: entries 2..4 of the list, slugged 2..4.
const StringSelectDatabase& main_story_database(){
    static const StringSelectDatabase database = make_story_select_database(
        story_list(),
        /*first=*/2,
        /*last=*/story_list().size()
    );
    return database;
}

// A range chosen in the main story's dropdowns and what the group reads from it.
struct StoryRangeCase{
    //  The start and end set by slug, as saved settings load them; empty
    //  leaves the default.
    struct Input{
        std::string start_slug;
        std::string end_slug;
    };

    //  The indexes in the dropdown, and whether the range is valid.
    struct Expectation{
        size_t start;
        size_t end;
        bool valid;
    };

    std::string name;
    Input input;
    Expectation expectation;
};

class Test_StoryRange : public UnitTest{
public:
    Test_StoryRange(StoryRangeCase test_case)
        : UnitTest(std::format("Pokemon::StoryRangeOptions - {}", test_case.name))
        , m_case(std::move(test_case))
    {}

    virtual UnitTestResult run(Logger&, CancellableScope&) const override{
        StoryRangeOptions range("Checkpoints", main_story_database());
        if (!m_case.input.start_slug.empty()){
            range.START_CHECKPOINT.set_by_slug(m_case.input.start_slug);
        }
        if (!m_case.input.end_slug.empty()){
            range.END_CHECKPOINT.set_by_slug(m_case.input.end_slug);
        }
        const StoryRangeCase::Expectation& expected = m_case.expectation;
        if (range.start_index() != expected.start || range.end_index() != expected.end){
            return std::format(
                "Expected indexes {} to {}, got {} to {}",
                expected.start, expected.end, range.start_index(), range.end_index()
            );
        }
        const std::string error = range.check_validity();
        if (error.empty() != expected.valid){
            return expected.valid ? std::format("Expected a valid range, got: {}", error) : "Expected the range refused.";
        }
        if (!expected.valid && error != "The start checkpoint comes after the end checkpoint."){
            return std::format("Unexpected message: {}", error);
        }
        return true;
    }

private:
    const StoryRangeCase m_case;
};

const StoryRangeCase STORY_RANGE_CASES[] = {
    {
        .name = "the whole database by default",
        .input = {.start_slug = "", .end_slug = ""},
        .expectation = {.start = 0, .end = 2, .valid = true},
    },
    {
        .name = "slugs are whole-list indexes, indexes are the dropdown's",
        .input = {.start_slug = "3", .end_slug = "4"},
        .expectation = {.start = 1, .end = 2, .valid = true},
    },
    {
        .name = "one checkpoint",
        .input = {.start_slug = "3", .end_slug = "3"},
        .expectation = {.start = 1, .end = 1, .valid = true},
    },
    {
        .name = "a start after the end is refused",
        .input = {.start_slug = "4", .end_slug = "2"},
        .expectation = {.start = 2, .end = 0, .valid = false},
    },
};

//  The dropdown labels a program passes, and the labels the dropdowns show.
struct StoryRangeLabelsCase{
    //  The labels passed to the constructor; none uses the default.
    struct Input{
        std::optional<StoryRangeOptions::DropdownLabels> labels;
    };

    //  The labels of the start and end dropdowns.
    struct Expectation{
        std::string start;
        std::string end;
    };

    std::string name;
    Input input;
    Expectation expectation;
};

class Test_StoryRangeLabels : public UnitTest{
public:
    Test_StoryRangeLabels(StoryRangeLabelsCase test_case)
        : UnitTest(std::format("Pokemon::StoryRangeOptions labels - {}", test_case.name))
        , m_case(std::move(test_case))
    {}

    virtual UnitTestResult run(Logger&, CancellableScope&) const override{
        const std::optional<StoryRangeOptions::DropdownLabels>& labels = m_case.input.labels;
        const StoryRangeOptions range = labels
            ? StoryRangeOptions("Checkpoints", main_story_database(), LockMode::LOCK_WHILE_RUNNING, *labels)
            : StoryRangeOptions("Checkpoints", main_story_database());
        const StoryRangeLabelsCase::Expectation& expected = m_case.expectation;
        if (range.START_CHECKPOINT.label() != expected.start || range.END_CHECKPOINT.label() != expected.end){
            return std::format(
                "Expected labels \"{}\" and \"{}\", got \"{}\" and \"{}\"",
                expected.start, expected.end, range.START_CHECKPOINT.label(), range.END_CHECKPOINT.label()
            );
        }
        return true;
    }

private:
    const StoryRangeLabelsCase m_case;
};

const StoryRangeLabelsCase STORY_RANGE_LABELS_CASES[] = {
    {
        .name = "the shared labels by default",
        .input = {.labels = std::nullopt},
        .expectation = {
            .start = "<b>Start checkpoint:</b><br>The program starts from this checkpoint's save.",
            .end = "<b>End checkpoint:</b><br>The program stops after this checkpoint.",
        },
    },
    {
        .name = "a program keeps its own labels",
        .input = {.labels = StoryRangeOptions::DropdownLabels{.start = "<b>Start Checkpoint:</b>", .end = "<b>End Checkpoint:</b>"}},
        .expectation = {.start = "<b>Start Checkpoint:</b>", .end = "<b>End Checkpoint:</b>"},
    },
};

} // namespace


void add_tests_StoryRangeOptions(UnitTestDatabase& database){
    for (const StoryRangeCase& test_case : STORY_RANGE_CASES){
        database.add<Test_StoryRange>(test_case);
    }
    for (const StoryRangeLabelsCase& test_case : STORY_RANGE_LABELS_CASES){
        database.add<Test_StoryRangeLabels>(test_case);
    }
}



} // namespace Pokemon
} // namespace PokemonAutomation
