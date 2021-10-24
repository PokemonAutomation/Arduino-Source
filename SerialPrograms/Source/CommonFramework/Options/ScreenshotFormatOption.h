/*  Screenshot Format
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ScreenshotFormat_H
#define PokemonAutomation_ScreenshotFormat_H

#include "CommonFramework/Notifications/MessageAttachment.h"
#include "EnumDropdownOption.h"

namespace PokemonAutomation{


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

    operator ImageAttachmentMode() const{
        return (ImageAttachmentMode)(size_t)*this;
    }
    void operator=(ImageAttachmentMode mode){
        this->set((size_t)mode);
    }
};


}
#endif
