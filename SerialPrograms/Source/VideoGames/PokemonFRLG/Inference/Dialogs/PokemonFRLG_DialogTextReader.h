/*  Dialog Text Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Read the text of dialog boxes and other text windows, for telling apart
 *  boxes that look the same (e.g. a VS Seeker message vs. a trainer's line).
 *
 *  The readers return the raw OCR text, trimmed, with line breaks as spaces.
 *  To look for words in it, use text_contains_any(), which compares
 *  normalized text (case, accents and punctuation don't matter), with the
 *  words of the game's language.
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_DialogTextReader_H
#define PokemonAutomation_PokemonFRLG_DialogTextReader_H

#include <string>
#include <vector>
#include "CommonFramework/Language.h"

namespace PokemonAutomation{
    class Logger;
    class ImageViewRGB32;
namespace NintendoSwitch{
namespace PokemonFRLG{


enum class DialogStyle{
    OVERWORLD,  //  Dark gray (or blue) text on the white overworld box.
    BATTLE,     //  White text on the dark teal battle box.
};

//  One line (0 = top, 1 = bottom) of the dialog box at the bottom of the
//  screen. Empty if nothing could be read.
std::string read_dialog_line(
    Logger& logger, Language language, const ImageViewRGB32& frame,
    DialogStyle style, int line
);

//  On the Fly map: the name of the place under the cursor, from the window at
//  the top-left. Empty if the cursor isn't on a named place.
std::string read_fly_map_name(Logger& logger, Language language, const ImageViewRGB32& frame);

//  In the bag: the description of the item under the cursor, from the window
//  at the bottom (both lines).
std::string read_bag_description(Logger& logger, Language language, const ImageViewRGB32& frame);


//  Whether `text` contains any of the words, comparing normalized text (see
//  OCR::normalize_utf32()), so "POKéMON" matches "pokemon".
bool text_contains_any(const std::string& text, const std::vector<std::string>& words);

//  The last number in `text`, e.g. 250 in "...for 250 steps.". -1 if there
//  is none (or it is longer than 4 digits).
int last_number_in(const std::string& text);



}
}
}
#endif
