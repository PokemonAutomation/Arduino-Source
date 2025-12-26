/*  Event Notification Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_EventNotificationWidget_H
#define PokemonAutomation_EventNotificationWidget_H

#include "Common/Qt/Options/ConfigWidget.h"
#include "EventNotificationOption.h"

namespace PokemonAutomation{


class TestButtonWidget : public ConfigWidget{
public:
    using ParentOption = TestMessageButton;

public:
    TestButtonWidget(QWidget& parent, TestMessageButton& value);
};



}
#endif
