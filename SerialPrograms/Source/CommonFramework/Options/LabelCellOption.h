/*  Label Cell
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Options_LabelCellOption_H
#define PokemonAutomation_Options_LabelCellOption_H

#include "Common/Cpp/Containers/Pimpl.h"
#include "Common/Cpp/Options/ConfigOption.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"

namespace PokemonAutomation{



class LabelCellOption : public ConfigOption{
public:
    ~LabelCellOption();
    LabelCellOption(std::string text, const ImageViewRGB32& icon);

    const std::string& text() const;
    const ImageViewRGB32& icon() const;

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual void restore_defaults() override{}

    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    struct Data;
    Pimpl<Data> m_data;
};





}
#endif


