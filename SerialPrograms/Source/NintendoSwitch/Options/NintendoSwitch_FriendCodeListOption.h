/*  Friend Code List
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_FriendCodeList_H
#define PokemonAutomation_FriendCodeList_H

#include <vector>
#include <QString>
#include "CommonFramework/Options/ConfigOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class FriendCodeListOption : public ConfigOption{
public:
    FriendCodeListOption(QString label, std::vector<QString> default_lines);
    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    static std::vector<uint8_t> parse(const QString& line);
    const std::vector<QString>& lines() const{ return m_lines; }

    std::vector<std::string> list() const;

    virtual void restore_defaults() override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    friend class FriendCodeListWidget;
    QString m_label;
    std::vector<QString> m_default;
    std::vector<QString> m_lines;
};




}
}
#endif

