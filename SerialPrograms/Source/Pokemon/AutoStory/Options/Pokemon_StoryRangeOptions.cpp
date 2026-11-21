/*  Story Range Options
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <utility>
#include "Pokemon_StoryRangeOptions.h"

namespace PokemonAutomation{
namespace Pokemon{


StoryRangeOptions::DropdownLabels StoryRangeOptions::default_labels(){
    return DropdownLabels{
        .start = "<b>Start checkpoint:</b><br>The program starts from this checkpoint's save.",
        .end = "<b>End checkpoint:</b><br>The program stops after this checkpoint.",
    };
}

StoryRangeOptions::StoryRangeOptions(
    std::string_view label,
    const StringSelectDatabase& checkpoints,
    LockMode end_lock,
    DropdownLabels labels
)
    : GroupOption(std::string(label), LockMode::LOCK_WHILE_RUNNING)
    , START_CHECKPOINT(
        std::move(labels.start),
        checkpoints,
        LockMode::LOCK_WHILE_RUNNING,
        /*default_index=*/0
    )
    , END_CHECKPOINT(
        std::move(labels.end),
        checkpoints,
        end_lock,
        /*default_index=*/checkpoints.size() == 0 ? 0 : checkpoints.size() - 1
    )
{
    PA_ADD_OPTION(START_CHECKPOINT);
    PA_ADD_OPTION(END_CHECKPOINT);
}

size_t StoryRangeOptions::start_index() const{
    return START_CHECKPOINT.index();
}

size_t StoryRangeOptions::end_index() const{
    return END_CHECKPOINT.index();
}

std::string StoryRangeOptions::check_validity() const{
    if (start_index() > end_index()){
        return "The start checkpoint comes after the end checkpoint.";
    }
    return GroupOption::check_validity();
}



} // namespace Pokemon
} // namespace PokemonAutomation
