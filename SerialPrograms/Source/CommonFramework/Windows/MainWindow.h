/*  Main Window UI
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_MainWindow_H
#define PokemonAutomation_MainWindow_H

#include <QMainWindow>
#include <QVBoxLayout>
#include "OutputWindow.h"

namespace PokemonAutomation{


class RightPanel;
class ProgramListUI;
class SettingListUI;

class MainWindow : public QMainWindow{
public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    OutputWindow& output_window() const{ return *m_output_window; }

    void left_panel_enabled(bool enabled);
    void change_panel(RightPanel& factory);

    void open_output_window();

private:
    virtual void closeEvent(QCloseEvent* event) override;
    virtual void resizeEvent(QResizeEvent* event) override;

private:
    QWidget* centralwidget;
    QMenuBar* menubar;
//    QStatusBar* statusbar;

    ProgramListUI* m_program_list;
    SettingListUI* m_setting_list;

    QVBoxLayout* m_right_panel_layout;
    QWidget* m_right_panel_widget;

    std::unique_ptr<OutputWindow> m_output_window;
};


}
#endif
