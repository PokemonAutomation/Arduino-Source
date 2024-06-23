/*  Item Printer Jobs Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_ItemPrinterMaterialDetector_H
#define PokemonAutomation_PokemonSV_ItemPrinterMaterialDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/Language.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Inference/VisualDetector.h"

namespace PokemonAutomation{
    class Logger;
    class AsyncDispatcher;
    class ConsoleHandle;
    class BotBaseContext;
namespace NintendoSwitch{
namespace PokemonSV{

class ItemPrinterMaterialDetector{
public:
    ItemPrinterMaterialDetector(Color color, Language language);

    void make_overlays(VideoOverlaySet& items) const;

    void find_happiny_dust(
        AsyncDispatcher& dispatcher,
        ConsoleHandle& console, BotBaseContext& context
    ) const;

    uint16_t detect_material_quantity(
        AsyncDispatcher& dispatcher,
        ConsoleHandle& console, BotBaseContext& context
    ) const;


private:
    uint16_t read_box(
        Logger& logger, AsyncDispatcher& dispatcher,
        const ImageViewRGB32& screen, const ImageFloatBox& box
    ) const;

private:
    Color m_color;
    Language m_language;
    ImageFloatBox m_box_mat_value;
    ImageFloatBox m_box_mat_quantity;

};



}
}
}
#endif
