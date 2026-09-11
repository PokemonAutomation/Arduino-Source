/*  Console Panel Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ConsoleInfra_ConsolePanelWidget_H
#define PokemonAutomation_ConsoleInfra_ConsolePanelWidget_H

#include <QWidget>
#include <QVBoxLayout>
#include "Common/Qt/UiStateQtWidget.h"
//#include "CommonFramework/Panels/UI/PanelElements.h"
#include "ConsoleInfra/ConsolePanel.h"
#include "ConsoleInfra/ConsolePanelSession.h"

namespace PokemonAutomation{
namespace ConsoleInfra{


class ConsolePanelWidget : public QWidget, public UiComponentQtWidget{
public:
    using ParentState = ConsolePanelSession;

public:
    ~ConsolePanelWidget();
    ConsolePanelWidget(QWidget& parent, ConsolePanelSession& session);

    virtual QWidget& widget() override{
        return *this;
    }

private:
    ConsolePanelSession& m_session;
    QVBoxLayout* m_layout;
    QWidget* m_system;
    ConfigWidget* m_options;
//    RunnablePanelActionBar* m_actions_bar;
};




}
}
#endif
