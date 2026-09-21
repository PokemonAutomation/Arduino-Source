/*  Panel Elements
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PanelElements_H
#define PokemonAutomation_PanelElements_H

#include <string>
#include <QLabel>
#include "Common/Qt/CollapsibleGroupBox.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/Panels/ProgramDescriptor.h"
//#include "Controllers/ControllerCapability.h"

class QPushButton;

namespace PokemonAutomation{


class ConfigOption;
class ProgramSession;


CollapsibleGroupBox* make_panel_header(
    QWidget& parent,
    const std::string& display_name,
    const std::string& doc_link,
    const std::string& description
);
CollapsibleGroupBox* make_panel_header(
    QWidget& parent,
    const std::string& display_name,
    const std::string& doc_link,
    const std::string& description,
    ProgramControllerClass color_class
);



class StatsBar : public QLabel{
public:
    StatsBar(QWidget& parent);
    StatsBar(QWidget& parent, ProgramSession& session);

    void set_stats(std::string current_stats, std::string historical_stats);
};



class RunnablePanelActionBar : public QGroupBox{
public:
    RunnablePanelActionBar(
        QWidget& parent,
        PanelSession& panel_session,
        ProgramSession& program_session,
        ProgramState initial_state
    );

    void set_state(PokemonAutomation::ProgramState state);

private:
    PokemonAutomation::ProgramState m_last_known_state;
    QPushButton* m_start_button;
    QPushButton* m_default_button;
};




QWidget* make_actions_bar(
    QWidget& panel,
    PanelSession& session
);

void populate_panel_widget(
    QWidget& panel,
    const PanelDescriptor& descriptor,
    UiState<>* console_system,
    ConfigOption& options,
    std::vector<QWidget*> footers
);






}
#endif
