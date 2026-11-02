/*  AutoStory Select Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "Pokemon_AutoStorySelect.h"
#include "Pokemon_AutoStorySelect_Tests.h"

namespace PokemonAutomation{
namespace Pokemon{

namespace{

// An entry of a story section, its index in the whole story list, and the
// index of the first main story entry.
struct AutoStoryIndexScenario{
    std::string name;
    bool main_story;
    size_t index_in_section;
    size_t first_main_story_index;
    size_t expected;
};

class Test_AutoStoryIndex : public UnitTest{
public:
    Test_AutoStoryIndex(AutoStoryIndexScenario scenario)
        : UnitTest("Pokemon::AutoStoryIndex - " + scenario.name)
        , m_scenario(std::move(scenario))
    {}

    virtual UnitTestResult run(Logger&, CancellableScope&) const override{
        const size_t got = story_index(
            m_scenario.main_story, m_scenario.index_in_section, m_scenario.first_main_story_index
        );
        if (got != m_scenario.expected){
            return "Expected index " + std::to_string(m_scenario.expected) + ", got " + std::to_string(got);
        }
        return true;
    }

private:
    const AutoStoryIndexScenario m_scenario;
};

// A stand-in for a story segment or checkpoint: only its name is read.
struct NamedEntry{
    std::string label;
    std::string name() const{ return label; }
};

// A dropdown built from [begin, end) of a story list of two tutorial entries,
// then two of the main story (which starts at index 2), as SV's lists are
// laid out, and its (slug, label) entries: slugs are indices in the whole
// list, not in the dropdown.
struct AutoStorySelectScenario{
    std::string name;
    size_t begin;
    size_t end;
    std::vector<std::pair<std::string, std::string>> expected;
};

class Test_AutoStorySelectDatabase : public UnitTest{
public:
    Test_AutoStorySelectDatabase(AutoStorySelectScenario scenario)
        : UnitTest("Pokemon::AutoStorySelectDatabase - " + scenario.name)
        , m_scenario(std::move(scenario))
    {}

    virtual UnitTestResult run(Logger&, CancellableScope&) const override{
        //  unique_ptr cannot be copied out of a braced list: build it in place.
        const std::vector<std::unique_ptr<NamedEntry>> list = []{
            std::vector<std::unique_ptr<NamedEntry>> list;
            for (const char* label : {"tutorial 0", "tutorial 1", "main story 0", "main story 1"}){
                list.emplace_back(std::make_unique<NamedEntry>(NamedEntry{label}));
            }
            return list;
        }();
        const StringSelectDatabase database = make_story_select_database(list, m_scenario.begin, m_scenario.end);
        std::vector<std::pair<std::string, std::string>> got;
        for (const StringSelectEntry& entry : database.case_list()){
            got.emplace_back(entry.slug, entry.display_name);
        }
        if (got != m_scenario.expected){
            return "Expected " + describe(m_scenario.expected) + ", got " + describe(got);
        }
        return true;
    }

private:
    static std::string describe(const std::vector<std::pair<std::string, std::string>>& entries){
        std::string text;
        for (const auto& [slug, label] : entries){
            text += (text.empty() ? "" : ", ") + slug + " \"" + label + "\"";
        }
        return "[" + text + "]";
    }

    const AutoStorySelectScenario m_scenario;
};

void add_index_tests(UnitTestDatabase& database){
    const AutoStoryIndexScenario scenarios[] = {
        {
            .name = "a tutorial entry is its own index",
            .main_story = false,
            .index_in_section = 3,
            .first_main_story_index = 10,
            .expected = 3,
        },
        {
            // SV: segment 10 is the first of the main story.
            .name = "the first main story entry follows the tutorial",
            .main_story = true,
            .index_in_section = 0,
            .first_main_story_index = 10,
            .expected = 10,
        },
        {
            // SV: checkpoint 21 is the first of the main story.
            .name = "a later main story checkpoint",
            .main_story = true,
            .index_in_section = 5,
            .first_main_story_index = 21,
            .expected = 26,
        },
    };
    for (const AutoStoryIndexScenario& scenario : scenarios){
        database.add<Test_AutoStoryIndex>(scenario);
    }
}

void add_select_database_tests(UnitTestDatabase& database){
    const AutoStorySelectScenario select_scenarios[] = {
        {
            .name = "the tutorial dropdown",
            .begin = 0,
            .end = 2,
            .expected = {{"0", "tutorial 0"}, {"1", "tutorial 1"}},
        },
        {
            .name = "the main story dropdown keeps whole-list slugs",
            .begin = 2,
            .end = 4,
            .expected = {{"2", "main story 0"}, {"3", "main story 1"}},
        },
        {
            .name = "an end past the list is clamped to it",
            .begin = 2,
            .end = 10,
            .expected = {{"2", "main story 0"}, {"3", "main story 1"}},
        },
    };
    for (const AutoStorySelectScenario& scenario : select_scenarios){
        database.add<Test_AutoStorySelectDatabase>(scenario);
    }
}

}

void add_tests_AutoStorySelect(UnitTestDatabase& database){
    add_index_tests(database);
    add_select_database_tests(database);
}



}
}
