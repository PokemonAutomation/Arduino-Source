/*  Main Window UI
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_MainWindow_H
#define PokemonAutomation_MainWindow_H

#include <QMainWindow>
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Panels/PanelTools.h"
#include "PanelLists.h"

class QVBoxLayout;

namespace PokemonAutomation{

class FileWindowLoggerWindow;


class MainWindow : public QMainWindow, public PanelHolder{
public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    virtual void closeEvent(QCloseEvent* event) override;
    virtual void resizeEvent(QResizeEvent* event) override;

    void close_panel();

    virtual bool report_new_panel_intent(const PanelDescriptor& descriptor) override;
    virtual void load_panel(std::unique_ptr<PanelInstance> panel) override;
    virtual Logger& raw_logger() override{ return global_logger_raw(); }
private:
    virtual void on_busy() override;
    virtual void on_idle() override;

private:
    QWidget* centralwidget;
    QMenuBar* menubar;
//    QStatusBar* statusbar;

    ProgramTabs* m_program_list = nullptr;
    QVBoxLayout* m_right_panel_layout;

    QWidget* m_settings;

    std::unique_ptr<PanelInstance> m_current_panel;
    QWidget* m_current_panel_widget;

    std::unique_ptr<FileWindowLoggerWindow> m_output_window;
};




}
#endif
