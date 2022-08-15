/*  Friend Code List
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_FriendCodeListWidget_H
#define PokemonAutomation_NintendoSwitch_FriendCodeListWidget_H

#include <QWidget>
#include "Common/Qt/Options/ConfigWidget.h"
#include "NintendoSwitch/Options/NintendoSwitch_FriendCodeListOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



class FriendCodeListWidget : public QWidget, public ConfigWidget, private ConfigOption::Listener{
public:
    ~FriendCodeListWidget();
    FriendCodeListWidget(QWidget& parent, FriendCodeListOption& value);

    virtual void update() override;
    virtual void value_changed() override;

private:
    class Box;

    FriendCodeListOption& m_value;
    Box* m_box;
};





}
}
#endif
