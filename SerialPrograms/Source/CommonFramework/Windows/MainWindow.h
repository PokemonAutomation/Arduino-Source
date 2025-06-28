/*  Main Window UI
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_MainWindow_H
#define PokemonAutomation_MainWindow_H

#include <QMainWindow>
#include "Common/Cpp/Options/ConfigOption.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Environment/SystemSleep.h"
#include "CommonFramework/Panels/PanelTools.h"
#include "PanelLists.h"

class QVBoxLayout;
class QLabel;
class QCheckBox;

namespace PokemonAutomation{

class ButtonDiagram;
class FileWindowLoggerWindow;


class MainWindow :
    public QMainWindow,
    public PanelHolder,
    public ConfigOption::Listener,
    public SystemSleepController::Listener
{
public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();


private:
    virtual void closeEvent(QCloseEvent* event) override;
    virtual void resizeEvent(QResizeEvent* event) override;
    virtual void moveEvent(QMoveEvent* event) override;

    void close_panel() noexcept;

    // implements PanelHolder::report_new_panel_intent()
    virtual bool report_new_panel_intent(const PanelDescriptor& descriptor) override;
    // implements PanelHolder::load_panel()
    virtual void load_panel(
        std::shared_ptr<const PanelDescriptor> descriptor,
        std::unique_ptr<PanelInstance> panel
    ) override;
    // implements PanelHolder::raw_logger()
    virtual Logger& raw_logger() override{ return global_logger_raw(); }

private:
    // implements PanelHolder::on_busy()
    // called when an automation program is running
    virtual void on_busy() override;
    // implements PanelHolder::on_idle()
    // called when no automation program is not running
    virtual void on_idle() override;
    virtual void on_config_value_changed(void* object) override;
    virtual void sleep_suppress_state_changed(SleepSuppress new_state) override;

private:
    QWidget* centralwidget;
    QMenuBar* menubar;
//    QStatusBar* statusbar;

//    ProgramTabs* m_program_list = nullptr;
    ProgramSelect* m_program_list = nullptr;
    QVBoxLayout* m_right_panel_layout;

    QWidget* m_settings;

    //  Keep a reference to the panel descriptor since it is referenced by the
    //  panel instance and the original descriptor may destroyed at any time.
    std::shared_ptr<const PanelDescriptor> m_current_panel_descriptor;
    std::unique_ptr<PanelInstance> m_current_panel;
    QWidget* m_current_panel_widget;

    std::unique_ptr<ButtonDiagram> m_button_diagram;
    std::unique_ptr<FileWindowLoggerWindow> m_output_window;

    QLabel* m_sleep_text;
    QCheckBox* m_sleep_box;
    std::unique_ptr<SleepSuppressScope> m_sleep_scope;

    bool m_pending_resize = false;
    bool m_pending_move = false;
    bool m_panel_transition = false;
};

// returns given X value, but bounded by 0 and screen_width
int32_t move_x_within_screen_bounds(int32_t x_pos);
// returns given y value, but bounded by 0 and screen_height
int32_t move_y_within_screen_bounds(int32_t y_pos);


}
#endif
