/*  Console Program Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_GameConsole_ConsoleProgramWidget_H
#define PokemonAutomation_GameConsole_ConsoleProgramWidget_H

#include <QVBoxLayout>
#include <QWidget>
#include "Common/Qt/UiStateQtWidget.h"
#include "CommonFramework/ProgramSession.h"
#include "CommonFramework/Panels/UI/PanelElements.h"
#include "GameConsole/Framework/ConsoleProgramSession.h"

namespace PokemonAutomation{

class ProgramResourceDownloadTableWidget;

namespace GameConsole{


class ConsoleProgramWidget
    : public QWidget
    , public UiComponentQtWidget
    , private ProgramSession::Listener
{
public:
    using ParentState = ConsoleProgramSession;


public:
    ~ConsoleProgramWidget();
    ConsoleProgramWidget(QWidget& parent, ConsoleProgramSession& session);

    virtual QWidget& widget() override{
        return *this;
    }


private:
    virtual void state_change(ProgramState state) override;
    virtual void stats_update(const StatsTracker* current_stats, const StatsTracker* historical_stats) override;
    virtual void error(const std::string& message) override;
    virtual void download_error(const std::string& message) override;
    virtual void download_added(std::shared_ptr<ResourceDownload> download_ptr) override;
    virtual void all_downloads_done() override;

    ProgramResourceDownloadTableWidget* ensure_downloads_table();


private:
    ConsoleProgramSession& m_session;
    QVBoxLayout* m_layout;
    StatsBar* m_stats_bar;
    RunnablePanelActionBar* m_actions_bar;

    // WARNING: Do not use directly. Always use ensure_downloads_table().
    ProgramResourceDownloadTableWidget* m_internal_lazy_downloads_table = nullptr;
    std::atomic<bool> m_popup_is_open{false};
};



}
}
#endif
