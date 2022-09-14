/*  Screenshot Format
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ScreenshotFormat_H
#define PokemonAutomation_ScreenshotFormat_H

#include "Common/Cpp/Options/EnumDropdownOption.h"

namespace PokemonAutomation{


enum class ImageAttachmentMode{
    NO_SCREENSHOT,
    JPG,
    PNG,
};
inline const EnumDatabase<ImageAttachmentMode>& ImageAttachmentMode_Database(){
    static EnumDatabase<ImageAttachmentMode> database({
        {ImageAttachmentMode::NO_SCREENSHOT,    "none", "No Screenshot."},
        {ImageAttachmentMode::JPG,              "jpg",  "Attach as .jpg."},
        {ImageAttachmentMode::PNG,              "png",  "Attach as .png."},
    });
    return database;
}



class ScreenshotCell : public EnumDropdownCell<ImageAttachmentMode>{
public:
    ScreenshotCell(ImageAttachmentMode default_mode = ImageAttachmentMode::JPG)
        : EnumDropdownCell<ImageAttachmentMode>(
            ImageAttachmentMode_Database(),
            default_mode
        )
    {}
};
class ScreenshotOption : public EnumDropdownOption<ImageAttachmentMode>{
public:
    ScreenshotOption(std::string label)
        : EnumDropdownOption<ImageAttachmentMode>(
            std::move(label),
            ImageAttachmentMode_Database(),
            ImageAttachmentMode::JPG
        )
    {}
};


}
#endif
