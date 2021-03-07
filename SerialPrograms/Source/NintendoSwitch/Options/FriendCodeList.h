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


class FriendCodeList : public ConfigOption{
public:
    FriendCodeList();
    virtual void load_json(const QJsonValue& json) override;
    virtual QJsonValue to_json() const override;

    static std::vector<uint8_t> parse(const QString& line);

    const std::vector<QString>& lines() const{ return m_lines; }
    virtual bool is_valid() const override;
    virtual void restore_defaults() override;

    virtual ConfigOptionUI* make_ui(QWidget& parent) override;

private:
    friend class FriendCodeListUI;
    std::vector<QString> m_lines;
};


class FriendCodeListUI : public ConfigOptionUI, public QWidget{
public:
    FriendCodeListUI(QWidget& parent, FriendCodeList& value);
    virtual QWidget* widget() override{ return this; }
    virtual void restore_defaults() override;

private:
    class Box;

    FriendCodeList& m_value;
    Box* m_box;
};


}
}
#endif

