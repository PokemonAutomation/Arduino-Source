/*  Screenshot Format
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ScreenshotFormat_H
#define PokemonAutomation_ScreenshotFormat_H

#include "Common/Cpp/Options/DropdownOption.h"

namespace PokemonAutomation{


enum class ImageAttachmentMode{
    NO_SCREENSHOT,
    JPG,
    PNG,
};


class ScreenshotOption : public DropdownOption{
public:
    ScreenshotOption(std::string label)
        : DropdownOption(
            std::move(label),
            {
                "No Screenshot.",
                "Attach as .jpg.",
                "Attach as .png.",
            },
            1
        )
    {}

    operator ImageAttachmentMode() const{
        return (ImageAttachmentMode)current_index();
    }
    void operator=(ImageAttachmentMode mode){
        this->set_index((size_t)mode);
    }
};


}
#endif
