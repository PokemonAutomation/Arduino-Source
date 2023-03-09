/*  Main Window UI
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_MainWindow_H
#define PokemonAutomation_MainWindow_H

#include <QVBoxLayout>
#include <QComboBox>
#include <QMainWindow>
#include "BoardList.h"
#include "Panels/RightPanel.h"

namespace PokemonAutomation{
namespace HexGenerator{


class ProgramListUI;
class SettingsListUI;

class MainWindow : public QMainWindow{
    Q_OBJECT

public:
    ~MainWindow();
    MainWindow(QWidget* parent = nullptr);

    const std::string& current_board() const;
    void change_panel(RightPanel& panel);

private:
    void close_panel();

private:
    QWidget* m_centralwidget;
    QMenuBar* m_menubar;
//    QStatusBar* m_statusbar;

    BoardList* m_mcu_list;
//    ProgramListUI* m_program_list;
//    SettingsListUI* m_settings_list;

    QVBoxLayout* m_right_panel_layout;
    QWidget* m_right_panel_widget;
};



}
}
#endif
