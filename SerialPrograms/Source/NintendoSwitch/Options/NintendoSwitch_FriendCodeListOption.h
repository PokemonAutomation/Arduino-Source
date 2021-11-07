/*  Friend Code List
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_FriendCodeList_H
#define PokemonAutomation_FriendCodeList_H

#include <array>
#include <vector>
#include "CommonFramework/Options/ConfigOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class FriendCodeListOption : public ConfigOption{
public:
    FriendCodeListOption(QString label, std::vector<QString> default_lines);
    virtual void load_json(const QJsonValue& json) override;
    virtual QJsonValue to_json() const override;

    static std::vector<uint8_t> parse(const QString& line);
    const std::vector<QString>& lines() const{ return m_lines; }

    std::vector<std::string> list() const;

    virtual void restore_defaults() override;

    virtual ConfigOptionUI* make_ui(QWidget& parent) override;

private:
    friend class FriendCodeListOptionUI;
    QString m_label;
    std::vector<QString> m_default;
    std::vector<QString> m_lines;
};


class FriendCodeListOptionUI : public QWidget, public ConfigOptionUI{
public:
    FriendCodeListOptionUI(QWidget& parent, FriendCodeListOption& value);
    virtual void restore_defaults() override;

private:
    class Box;

    FriendCodeListOption& m_value;
    Box* m_box;
};


}
}
#endif

