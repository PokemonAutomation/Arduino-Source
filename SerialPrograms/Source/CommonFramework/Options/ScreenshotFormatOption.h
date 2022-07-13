/*  Screenshot Format
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ScreenshotFormat_H
#define PokemonAutomation_ScreenshotFormat_H

#include "EnumDropdownOption.h"

namespace PokemonAutomation{


enum class ImageAttachmentMode{
    NO_SCREENSHOT,
    JPG,
    PNG,
};


class ScreenshotOption : public EnumDropdownOption{
public:
    ScreenshotOption(std::string label)
        : EnumDropdownOption(
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
        return (ImageAttachmentMode)(size_t)*this;
    }
    void operator=(ImageAttachmentMode mode){
        this->set((size_t)mode);
    }
};


}
#endif
