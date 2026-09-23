/*  Multi-Console Program Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_GameConsole_MultiConsoleProgramWidget_H
#define PokemonAutomation_GameConsole_MultiConsoleProgramWidget_H

#include <QVBoxLayout>
#include <QWidget>
#include "Common/Qt/UiStateQtWidget.h"
#include "CommonFramework/ProgramSession.h"
#include "CommonFramework/Panels/UI/PanelElements.h"
#include "GameConsole/Framework/MultiConsoleProgramSession.h"

namespace PokemonAutomation{

class ProgramResourceDownloadTableWidget;

namespace GameConsole{


class MultiConsoleProgramWidget
    : public QWidget
    , public UiComponentQtWidget
    , private ProgramSession::Listener
    , private MultiConsoleSystemSession::Listener
{
public:
    using ParentState = MultiConsoleProgramSession;


public:
    ~MultiConsoleProgramWidget();
    MultiConsoleProgramWidget(QWidget& parent, MultiConsoleProgramSession& session);

    virtual QWidget& widget() override{
        return *this;
    }


private:
    virtual void on_console_count_lock(bool locked) override{}
    virtual void shutdown() override{}
    virtual void startup(size_t console_count) override{}

    virtual void state_change(ProgramState state) override;
    virtual void stats_update(const StatsTracker* current_stats, const StatsTracker* historical_stats) override;
    virtual void error(const std::string& message) override;
    virtual void download_error(const std::string& message) override;
    virtual void download_added(std::shared_ptr<ResourceDownload> download_ptr) override;
    virtual void all_downloads_done() override;

    ProgramResourceDownloadTableWidget* ensure_downloads_table();


private:
    MultiConsoleProgramSession& m_session;
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
