/*  Panel Elements
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PanelElements_H
#define PokemonAutomation_PanelElements_H

#include <string>
#include <QLabel>
#include "Common/Qt/CollapsibleGroupBox.h"
#include "CommonFramework/Globals.h"
#include "Common/PokemonSwSh/PokemonProgramIDs.h"

class QPushButton;

namespace PokemonAutomation{



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
    FeedbackType feedback,
    PABotBaseLevel serial_level
);



class StatsBar : public QLabel{
    Q_OBJECT
public:
    StatsBar(QWidget& parent);

public slots:
    void set_stats(std::string current_stats, std::string historical_stats);
};



class RunnablePanelActionBar : public QGroupBox{
    Q_OBJECT
public:
    RunnablePanelActionBar(QWidget& parent, ProgramState initial_state);

public slots:
    void set_state(PokemonAutomation::ProgramState state);

signals:
    void start_clicked();
    void defaults_clicked();

private:
    QPushButton* m_start_button;
    QPushButton* m_default_button;
};









}
#endif
