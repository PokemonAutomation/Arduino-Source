/*  Dialog Text Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Reads the text of dialog boxes and of the party menu's action list, for
 *  telling apart boxes that look the same (VS Seeker messages, the Shift
 *  prompt vs. the move-learn prompt, which line of the party menu is CUT).
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_DialogTextReader_H
#define PokemonAutomation_PokemonFRLG_DialogTextReader_H

#include <string>
#include <vector>

namespace PokemonAutomation{
    class Logger;
    class ImageViewRGB32;
namespace NintendoSwitch{
namespace PokemonFRLG{


enum class DialogStyle{
    OVERWORLD,  //  Dark gray (or blue) text on the white overworld box.
    BATTLE,     //  White text on the dark teal battle box.
};

//  Read one line (0 = top, 1 = bottom) of the dialog box at the bottom of the
//  screen. Returns the raw OCR text in lower case, or an empty string.
//  English only.
std::string read_dialog_line(
    Logger& logger, const ImageViewRGB32& frame,
    DialogStyle style, int line
);

//  True if `text` contains any of the keywords (lower case).
bool text_contains_any(const std::string& text, const std::vector<std::string>& keywords);

//  Read the last number in `text`. Returns -1 if there is none.
int last_number_in(const std::string& text);


//  The action list of a Pokemon in the party menu (SUMMARY, field moves,
//  SWITCH, ITEM, CANCEL), as tokens from top to bottom, e.g.
//  {"summary", "sweet-scent", "cut", "switch", "item", "cancel"}.
//  A line that can't be read is an empty string. English only.
std::vector<std::string> read_party_action_list(Logger& logger, const ImageViewRGB32& frame);

//  On the Fly map: the name of the place under the cursor, from the box at
//  the top-left (white text), in lower case. Empty if the cursor isn't on a
//  named place. English only.
std::string read_fly_map_name(Logger& logger, const ImageViewRGB32& frame);

//  In the bag: the second line of the description of the item under the
//  cursor, from the box at the bottom (white text), in lower case. English
//  only.
std::string read_bag_description(Logger& logger, const ImageViewRGB32& frame);



}
}
}
#endif
