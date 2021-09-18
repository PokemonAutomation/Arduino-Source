/*  Screenshot Format
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ScreenshotFormat_H
#define PokemonAutomation_ScreenshotFormat_H

#include "EnumDropdownOption.h"

namespace PokemonAutomation{


enum class ScreenshotMode{
    NO_SCREENSHOT,
    JPG,
    PNG,
};

class ScreenshotOption : public EnumDropdownOption{
public:
    ScreenshotOption(QString label)
        : EnumDropdownOption(
            std::move(label),
            {
                "Do not attach screenshot.",
                "Attach as .jpg.",
                "Attach as .png.",
            },
            1
        )
    {}

    operator ScreenshotMode() const{
        return (ScreenshotMode)(size_t)*this;
    }
};


}
#endif
