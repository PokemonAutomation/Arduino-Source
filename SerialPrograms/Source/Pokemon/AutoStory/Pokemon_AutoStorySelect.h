/*  AutoStory Select
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Pokemon_AutoStorySelect_H
#define PokemonAutomation_Pokemon_AutoStorySelect_H

#include <memory>
#include <string>
#include <vector>
#include "CommonTools/Options/StringSelectOption.h"

namespace PokemonAutomation{
namespace Pokemon{


//  A dropdown database of the story segments or checkpoints [begin, end) of
//  `list`, labeled by their name() and slugged by their index in the whole
//  list, so a slug picks the same entry whichever section it was chosen from.
template <typename Entry>
StringSelectDatabase make_story_select_database(
    const std::vector<std::unique_ptr<Entry>>& list,
    size_t begin, size_t end
){
    StringSelectDatabase ret;
    for (size_t i = begin; i < end && i < list.size(); i++){
        ret.add_entry(StringSelectEntry(std::to_string(i), list[i]->name()));
    }
    return ret;
}


//  The index in the whole story list of entry `index_in_section` of the
//  tutorial or the main story, whose first entry is `first_main_story_index`.
inline size_t story_index(bool main_story, size_t index_in_section, size_t first_main_story_index){
    return main_story ? first_main_story_index + index_in_section : index_in_section;
}


}
}
#endif
