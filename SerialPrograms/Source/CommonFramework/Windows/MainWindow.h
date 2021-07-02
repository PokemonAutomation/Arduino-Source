/*  Main Window UI
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_MainWindow_H
#define PokemonAutomation_MainWindow_H

#include <QMainWindow>
#include <QVBoxLayout>
#include "CommonFramework/Panels/Panel.h"
#include "PanelLists.h"
#include "OutputWindow.h"

namespace PokemonAutomation{


class MainWindow : public QMainWindow, public PanelListener{
public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

//    OutputWindow& output_window() const{ return *m_output_window; }

    void open_output_window();

private:
    virtual void closeEvent(QCloseEvent* event) override;
    virtual void resizeEvent(QResizeEvent* event) override;

    void close_panel();

    virtual void on_panel_construct(std::unique_ptr<PanelInstance> panel) override;
public: //  Make private.
    virtual OutputWindow& output_window() override{ return *m_output_window; }
private:
    virtual void on_busy(PanelInstance& panel) override;
    virtual void on_idle(PanelInstance& panel) override;

private:
    QWidget* centralwidget;
    QMenuBar* menubar;
//    QStatusBar* statusbar;

    ProgramTabs* m_program_list = nullptr;
    QVBoxLayout* m_right_panel_layout;

    QWidget* m_settings;

    std::unique_ptr<PanelInstance> m_current_panel;
    QWidget* m_current_panel_widget;

    std::unique_ptr<OutputWindow> m_output_window;
};


}
#endif
