/*  Donut Berry Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_BerrySession_H
#define PokemonAutomation_PokemonLZA_BerrySession_H

#include <map>
#include <memory>
#include "CommonFramework/Language.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"
#include "PokemonLZA/Inference/Donuts/PokemonLZA_DonutBerriesDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

class DonutBerriesReader;

struct PageIngredients{
    // the line index of the current selected berry, < BERRY_PAGE_LINES
    int8_t selected = -1;
    std::set<std::string> item[DonutBerriesReader::BERRY_PAGE_LINES];
};


class BerrySession{
public:
    ~BerrySession();
    BerrySession(
        VideoStream& stream, ProControllerContext& context,
        Language language
    );

    void add_berries(
        VideoStream& stream, ProControllerContext& context,
        const std::map<std::string, uint8_t>& berries
    );


public:
    PageIngredients read_screen(std::shared_ptr<const ImageRGB32> screenshot) const;
    PageIngredients read_current_page() const;

    
    //  Move to any berry in the set. Returns the berry it moved to.
    //  Returns empty string if not found.
    std::string move_to_ingredient(const std::string& berry, bool move_topdown) const;

    bool run_move_iteration(
        std::string& slug, const std::string& berry,
        const PageIngredients& page
    ) const;


private:
    VideoStream& m_stream;
    ProControllerContext& m_context;
    Language m_language;
    VideoOverlaySet m_overlays;
    //int8_t m_num_confirmed;
};



//  Read string to move to each target berry and add them to make donut.
//  When this function returns, the berries are all confirmed with button plus.
void add_donut_berries(
    VideoStream& stream, ProControllerContext& context,
    Language language,
    const std::map<std::string, uint8_t>& berries  //  {slug, quantity}
);




}
}
}
#endif
