/*  Friend Code List
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_FriendCodeListOption_H
#define PokemonAutomation_NintendoSwitch_FriendCodeListOption_H

#include <vector>
#include "Common/Cpp/Options/TextEditOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


//  A list of Switch friend codes, one per line.
class FriendCodeListOption : public TextEditOption{
public:
    FriendCodeListOption(std::string label, std::vector<std::string> default_lines);

    static std::string parse(const std::string& line);

    //  The current text split into lines.
    std::vector<std::string> lines() const;
    //  The lines that parse to a valid 12-digit code, formatted as "SW-XXXX-XXXX-XXXX".
    std::vector<std::string> list() const;

public:
    virtual JsonValue to_json() const override;
    virtual void load_json(const JsonValue& json) override;
};



}
}
#endif
