/*  Friend Code List
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_FriendCodeList_H
#define PokemonAutomation_FriendCodeList_H

#include <vector>
#include "CommonFramework/Options/ConfigOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class FriendCodeListOption : public ConfigOption{
public:
    FriendCodeListOption(std::string label, std::vector<std::string> default_lines);
    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    static std::vector<uint8_t> parse(const std::string& line);
    const std::vector<std::string>& lines() const{ return m_lines; }

    std::vector<std::string> list() const;

    virtual void restore_defaults() override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    friend class FriendCodeListWidget;
    std::string m_label;
    std::vector<std::string> m_default;
    std::vector<std::string> m_lines;
};




}
}
#endif

