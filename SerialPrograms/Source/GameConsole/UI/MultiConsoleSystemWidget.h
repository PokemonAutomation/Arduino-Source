/*  Multi-Console System Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomationn_GameConsole_MultiConsoleSystemWidget_H
#define PokemonAutomationn_GameConsole_MultiConsoleSystemWidget_H

#include <QWidget>
#include <QComboBox>
#include "Common/Qt/UiStateQtWidget.h"
#include "GameConsole/MultiConsoleSystemSession.h"

namespace PokemonAutomation{
namespace GameConsole{


class MultiConsoleSystemWidget final
    : public QWidget
    , public UiComponentQtWidget
    , private MultiConsoleSystemSession::Listener
{
public:
    using ParentState = MultiConsoleSystemSession;


public:
    virtual ~MultiConsoleSystemWidget();

    MultiConsoleSystemWidget(
        QWidget& parent,
        MultiConsoleSystemSession& session
    );

    virtual QWidget& widget() override{
        return *this;
    }


private:
    virtual void on_console_count_lock(bool locked) override;
    virtual void shutdown() override;
    virtual void startup(size_t console_count) override;

    void redraw_videos();


private:
    MultiConsoleSystemSession& m_session;
    QComboBox* m_console_count_box;
    QWidget* m_videos = nullptr;
};



}
}
#endif
