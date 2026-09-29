/*  Trade Count Table
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_TradeCountTable_H
#define PokemonAutomation_PokemonLA_TradeCountTable_H

#include <string>
#include <vector>
#include "Common/Cpp/Options/ConfigOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/StaticTableOption.h"
#include "CommonFramework/Options/LabelCellOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class TradeCountTableRow : public StaticTableRow{
public:
    TradeCountTableRow(const std::string& slug, const ImageViewRGB32& icon);

    const uint8_t default_value;

    LabelCellOption pokemon;
    SimpleIntegerCell<uint8_t> count;
    LabelCellOption default_label;
};


class TradeCountTable : public StaticTableOption{
public:
    TradeCountTable();
    virtual std::vector<std::string> make_header() const;
};





}
}
}
#endif
