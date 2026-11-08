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


//  A dropdown database of the story segments or checkpoints [first, last) of
//  `list`, labeled by their name() and slugged by their index in the whole
//  list, so a slug picks the same entry whichever section it was chosen from.
//  Bounds are whole-list indices. `last` is exclusive and clamped to list.size();
//  first >= last or first >= list.size() yields an empty database.
template <typename Entry>
StringSelectDatabase make_story_select_database(
    const std::vector<std::unique_ptr<Entry>>& list,
    size_t first, size_t last
);

template <typename Entry>
StringSelectDatabase make_story_select_database(
    const std::vector<std::unique_ptr<Entry>>& list,
    size_t first, size_t last
){
    StringSelectDatabase ret;
    for (size_t i = first; i < last && i < list.size(); i++){
        ret.add_entry(StringSelectEntry(std::to_string(i), list[i]->name()));
    }
    return ret;
}


//  The index in the whole story list of entry `index_in_section` of the
//  tutorial or the main story, whose first entry is `first_main_story_index`.
//  NOTE: a section ends where the user must step in (e.g. level up the
//  Pokemon between the tutorial and the main story). SV has two sections.
//  Later content (postgame, DLC) appends to the last section, run only after
//  the main story; if it needs its own break (e.g. grinding before a DLC),
//  it becomes a third section and `main_story` here becomes a section index.
inline size_t story_index(bool main_story, size_t index_in_section, size_t first_main_story_index){
    return main_story ? first_main_story_index + index_in_section : index_in_section;
}


} // namespace Pokemon
} // namespace PokemonAutomation
#endif
