/*  Screenshot Format
 *
 *  From: https://github.com/PokemonAutomation/
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
inline const EnumDropdownDatabase<ImageAttachmentMode>& ImageAttachmentMode_Database(){
    static EnumDropdownDatabase<ImageAttachmentMode> database({
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
            LockMode::UNLOCK_WHILE_RUNNING,
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
            LockMode::UNLOCK_WHILE_RUNNING,
            ImageAttachmentMode::JPG
        )
    {}
};


}
#endif
