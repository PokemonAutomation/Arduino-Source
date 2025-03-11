/*  Label Cell
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Options_LabelCellOption_H
#define PokemonAutomation_Options_LabelCellOption_H

#include "Common/Cpp/ImageResolution.h"
#include "Common/Cpp/Containers/Pimpl.h"
#include "Common/Cpp/Options/ConfigOption.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
//#include "CommonFramework/ImageTypes/ImageRGB32.h"

namespace PokemonAutomation{



class LabelCellOption : public ConfigOption{
public:
    ~LabelCellOption();

    //  Make label with no icon.
    LabelCellOption(
        LockMode lock_while_running,
        std::string text
    );

    //  Make label with the icon in its original resolution.
    LabelCellOption(
        LockMode lock_while_running,
        std::string text, const ImageViewRGB32& icon
    );

    //  Make label with icon and scale it so that the largest dimension is "icon_size" pixels.
    LabelCellOption(
        LockMode lock_while_running,
        std::string text, const ImageViewRGB32& icon, size_t icon_size
    );

//    LabelCellOption(std::string text, ImageRGB32 icon);

    const std::string& text() const;
    const ImageViewRGB32& icon() const;
    Resolution resolution() const;

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual void restore_defaults() override{}

    virtual ConfigWidget* make_QtWidget(QWidget& parent) override;

private:
    struct Data;
    Pimpl<Data> m_data;
};





}
#endif


