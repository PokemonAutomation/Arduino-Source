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



class FriendCodeListWidget : public QWidget, public ConfigWidget{
public:
    ~FriendCodeListWidget();
    FriendCodeListWidget(QWidget& parent, FriendCodeListOption& value);

    virtual void update_value() override;
    virtual void update_visibility(bool program_is_running) override;
    virtual void value_changed() override;

private:
    class Box;

    FriendCodeListOption& m_value;
    Box* m_box;
};





}
}
#endif
