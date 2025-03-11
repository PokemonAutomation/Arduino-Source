/*  Friend Code List
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_FriendCodeListOption_H
#define PokemonAutomation_NintendoSwitch_FriendCodeListOption_H

#include <vector>
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Common/Cpp/Options/ConfigOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class FriendCodeListOption : public ConfigOption{
public:
    FriendCodeListOption(std::string label, std::vector<std::string> default_lines);

    static std::string parse(const std::string& line);

    void set(const std::string& text);
    std::vector<std::string> lines() const;
    std::vector<std::string> list() const;

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual void restore_defaults() override;

    virtual ConfigWidget* make_QtWidget(QWidget& parent) override;

private:
    friend class FriendCodeListWidget;
    const std::string m_label;
    const std::vector<std::string> m_default;

    mutable SpinLock m_lock;
    std::vector<std::string> m_lines;
};




}
}
#endif

