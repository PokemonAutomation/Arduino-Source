/*  Story Range Options
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  The start and end checkpoint dropdowns of an AutoStory program, over one
 *  list of the story's checkpoints, and the check that the start does not
 *  come after the end.
 *
 */

#ifndef PokemonAutomation_Pokemon_StoryRangeOptions_H
#define PokemonAutomation_Pokemon_StoryRangeOptions_H

#include <string>
#include <string_view>
#include "Common/Cpp/Options/GroupOption.h"
#include "CommonTools/Options/StringSelectOption.h"

namespace PokemonAutomation{
namespace Pokemon{


//  The options in an AutoStory program that pick the range of checkpoints
//  to run.
class StoryRangeOptions : public GroupOption{
public:
    //  The labels of the two dropdowns.
    struct DropdownLabels{
        std::string start;
        std::string end;
    };
    static DropdownLabels default_labels();

    //  `checkpoints`: the dropdown database (make_story_select_database()),
    //  kept alive by the caller. The dropdowns start on its first and last
    //  entries.
    //  `end_lock`: UNLOCK_WHILE_RUNNING for a program that reads the end
    //  after each checkpoint, so the end can be moved during a run.
    StoryRangeOptions(
        std::string_view label,
        const StringSelectDatabase& checkpoints,
        LockMode end_lock = LockMode::LOCK_WHILE_RUNNING,
        DropdownLabels labels = default_labels()
    );

    //  The checkpoints chosen, by index in the database, both inclusive.
    size_t start_index() const;
    size_t end_index() const;
    //  Checks that the start checkpoint does not come after the end.
    virtual std::string check_validity() const override;

    StringSelectOption START_CHECKPOINT;
    StringSelectOption END_CHECKPOINT;
};



} // namespace Pokemon
} // namespace PokemonAutomation
#endif
