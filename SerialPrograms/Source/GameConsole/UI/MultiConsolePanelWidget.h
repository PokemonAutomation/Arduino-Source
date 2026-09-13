/*  Multi-Console Panel Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_GameConsole_MultiConsolePanelWidget_H
#define PokemonAutomation_GameConsole_MultiConsolePanelWidget_H

#include <QWidget>
#include "Common/Qt/UiStateQtWidget.h"
#include "GameConsole/MultiConsolePanel.h"
#include "GameConsole/MultiConsolePanelSession.h"

namespace PokemonAutomation{
namespace GameConsole{


class MultiConsolePanelWidget : public QWidget, public UiComponentQtWidget{
public:
    using ParentState = MultiConsolePanelSession;

public:
    ~MultiConsolePanelWidget();
    MultiConsolePanelWidget(QWidget& parent, MultiConsolePanelSession& session);

    virtual QWidget& widget() override{
        return *this;
    }

private:
    MultiConsolePanelSession& m_session;
};




}
}
#endif
