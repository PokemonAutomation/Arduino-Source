/*  Main Window UI
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QMenuBar>
//#include <QStatusBar>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include "Common/Cpp/CpuId/CpuId.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Logging/FileWindowLogger.h"
#include "CommonFramework/Startup/NewVersionCheck.h"
#include "CommonFramework/Options/ResolutionOption.h"
#include "CommonFramework/Options/Environment/ThemeSelectorOption.h"
#include "CommonFramework/Windows/DpiScaler.h"
#include "PanelLists.h"
#include "WindowTracker.h"
#include "ButtonDiagram.h"
#include "MainWindow.h"


#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_current_panel_widget(nullptr)
{
    if (objectName().isEmpty()){
        setObjectName(QString::fromUtf8("MainWindow"));
    }
    this->setWindowIcon(QIcon(QString::fromStdString(RESOURCE_PATH() + "icon.png")));
//    QSize window_size = PERSISTENT_SETTINGS().window_size;
    resize(
        GlobalSettings::instance().WINDOW_SIZE->WIDTH,
        GlobalSettings::instance().WINDOW_SIZE->HEIGHT
    );
    // move main window to desired position on startup
    // auto const screen_geometry = QGuiApplication::primaryScreen()->availableGeometry();
    // uint32_t const screen_width = (uint32_t)screen_geometry.width();
    // uint32_t const screen_height = (uint32_t)screen_geometry.height();
    int32_t x_pos_main = GlobalSettings::instance().WINDOW_SIZE->X_POS;
    int32_t y_pos_main = GlobalSettings::instance().WINDOW_SIZE->Y_POS;
    int32_t move_x_main = move_x_within_screen_bounds(x_pos_main);
    int32_t move_y_main = move_y_within_screen_bounds(y_pos_main);
    move(move_x_main, move_y_main);

    centralwidget = new QWidget(this);
    centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
    setCentralWidget(centralwidget);
    menubar = new QMenuBar(this);
    menubar->setObjectName(QString::fromUtf8("menubar"));
    setMenuBar(menubar);
//    statusbar = new QStatusBar(this);
//    statusbar->setObjectName(QString::fromUtf8("statusbar"));
//    setStatusBar(statusbar);
    setWindowTitle(QString::fromStdString(PROGRAM_NAME + " Computer-Control Programs (" + PROGRAM_VERSION + ")"));

    QHBoxLayout* hbox = new QHBoxLayout(centralwidget);
    QVBoxLayout* left_layout = new QVBoxLayout();
    hbox->addLayout(left_layout, 0);

#if 0
    QGroupBox* program_box = new QGroupBox("Program Select", centralwidget);
    left_layout->addWidget(program_box, 1);
    QVBoxLayout* program_layout = new QVBoxLayout(program_box);
    program_layout->setAlignment(Qt::AlignTop);

//    NoWheelComboBox* program_dropdown = new NoWheelComboBox(this);
//    program_layout->addWidget(program_dropdown);

    m_program_list = new ProgramTabs(*this, *this);
    program_layout->addWidget(m_program_list);
#else
    m_program_list = new ProgramSelect(*this, *this);
    left_layout->addWidget(m_program_list, 1);
#endif


    QGroupBox* support_box = new QGroupBox(
        QString::fromStdString(PROGRAM_NAME + " " + PROGRAM_VERSION + " (" + PA_ARCH_STRING + ")"),
        centralwidget
    );


    left_layout->addWidget(support_box);
    QVBoxLayout* support_layout = new QVBoxLayout(support_box);

    {
        QHBoxLayout* layout = new QHBoxLayout();
        support_layout->addLayout(layout);
        layout->setAlignment(Qt::AlignHCenter);
        layout->setContentsMargins(0, 0, 0, 0);
        m_sleep_text = new QLabel(support_box);
        layout->addWidget(m_sleep_text, 2);
        m_sleep_box = new QCheckBox("Force On", support_box);
        layout->addWidget(m_sleep_box, 1, Qt::AlignHCenter);
        MainWindow::sleep_suppress_state_changed(SystemSleepController::instance().current_state());
#if QT_VERSION < 0x060700
        connect(
            m_sleep_box, &QCheckBox::stateChanged,
            this, [this](int){
                if (m_sleep_box->isChecked()){
                    m_sleep_scope.reset(new SleepSuppressScope(SleepSuppress::SCREEN_ON));
                }else{
                    m_sleep_scope.reset();
                }
            }
        );
#else
        connect(
            m_sleep_box, &QCheckBox::checkStateChanged,
            this, [this](Qt::CheckState state){
                if (state == Qt::CheckState::Checked){
                    m_sleep_scope.reset(new SleepSuppressScope(SleepSuppress::SCREEN_ON));
                }else{
                    m_sleep_scope.reset();
                }
            }
        );
#endif
    }

    QHBoxLayout* support = new QHBoxLayout();
    support_layout->addLayout(support);
//    support->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout* links = new QVBoxLayout();
    support->addLayout(links);
    {
        QLabel* github = new QLabel(support_box);
        links->addWidget(github);
        github->setText(QString::fromStdString(make_text_url(ONLINE_DOC_URL_BASE + "ComputerControl/", "Online Documentation")));
        github->setTextFormat(Qt::RichText);
        github->setTextInteractionFlags(Qt::TextBrowserInteraction);
        github->setOpenExternalLinks(true);
    }
    {
        QLabel* discord = new QLabel(support_box);
        links->addWidget(discord);
        discord->setText(QString::fromStdString(make_text_url(DISCORD_LINK_URL_PROGRAM, DISCORD_LINK_TEXT)));
        discord->setTextFormat(Qt::RichText);
        discord->setTextInteractionFlags(Qt::TextBrowserInteraction);
        discord->setOpenExternalLinks(true);
    }
    {
        QLabel* github = new QLabel(support_box);
        links->addWidget(github);
        github->setText(QString::fromStdString(make_text_url(GITHUB_LINK_URL, GITHUB_LINK_TEXT)));
//        github->setText("<a href=\"" + GITHUB_REPO + "\">GitHub Repository</a>");
        github->setTextFormat(Qt::RichText);
        github->setTextInteractionFlags(Qt::TextBrowserInteraction);
        github->setOpenExternalLinks(true);
    }
    {
        QLabel* about = new QLabel(support_box);
        links->addWidget(about);
        about->setText(QString::fromStdString(make_text_url(GITHUB_LINK_URL, "About this Program")));
        about->setTextFormat(Qt::RichText);
        connect(
            about, &QLabel::linkActivated,
            this, [](const QString&){
                std::string str;
                str += PROGRAM_NAME + " Computer-Control Programs<br>";
                str += "Copyright: 2020 - 2025<br>";
                str += "Version: " + PROGRAM_VERSION + "<br>";
                str += "<br>";
                str += "Framework: Qt " + std::to_string(QT_VERSION_MAJOR);
                str += "." + std::to_string(QT_VERSION_MINOR);
                str += "." + std::to_string(QT_VERSION_PATCH);
                str += "<br>";
                str += "Compiler: " + COMPILER_VERSION;
                str += "<br><br>";
                str += "Made by the " + PROGRAM_NAME + " Discord Server.<br>";
                str += "<br>";
                str += "This program uses Qt and dynamically links to unmodified Qt libraries under LGPL.<br>";

                QMessageBox box;
                box.information(nullptr, "About", QString::fromStdString(str));
            }
        );
    }

    QVBoxLayout* buttons = new QVBoxLayout();
    support->addLayout(buttons);
    {
        QPushButton* keyboard = new QPushButton("Keyboard Layout", support_box);
        buttons->addWidget(keyboard);
        m_button_diagram.reset(new ButtonDiagram());
        connect(
            keyboard, &QPushButton::clicked,
            this, [button = m_button_diagram.get()](bool){
                button->show();
                button->raise(); // bring the window to front on macOS
                button->activateWindow(); // bring the window to front on Windows
            }
        );
    }
    {
        m_output_window.reset(new FileWindowLoggerWindow((FileWindowLogger&)global_logger_raw()));
        QPushButton* output = new QPushButton("Output Window", support_box);
        buttons->addWidget(output);
        connect(
            output, &QPushButton::clicked,
            this, [&](bool){
                m_output_window->show();
                m_output_window->raise(); // bring the window to front on macOS
                m_output_window->activateWindow(); // bring the window to front on Windows
            }
        );
        // get snapshot of the saved initial x/y position, since activating the window seems to change the window coordinates, 
        // which then causes initial x/y position to change, due to triggering moveEvent().
        int32_t x_pos_log = GlobalSettings::instance().LOG_WINDOW_SIZE->X_POS;
        int32_t y_pos_log = GlobalSettings::instance().LOG_WINDOW_SIZE->Y_POS;    

        if (GlobalSettings::instance().LOG_WINDOW_STARTUP){ // show the Output Window on startup
            m_output_window->show();
            m_output_window->raise(); // bring the window to front on macOS
            m_output_window->activateWindow(); // bring the window to front on Windows
        }

        // move the output window to desired position on startup
        int32_t move_x_log = move_x_within_screen_bounds(x_pos_log);
        int32_t move_y_log = move_y_within_screen_bounds(y_pos_log);
        m_output_window->move(move_x_log, move_y_log);
    }
    {
        QPushButton* settings = new QPushButton("Settings", support_box);
        m_settings = settings;
        buttons->addWidget(settings);
        connect(
            settings, &QPushButton::clicked,
            this, [this](bool){
                if (report_new_panel_intent(GlobalSettings_Descriptor::INSTANCE)){
                    load_panel(nullptr, GlobalSettings_Descriptor::INSTANCE.make_panel());
                }
            }
        );
    }

    QVBoxLayout* right = new QVBoxLayout();
    m_right_panel_layout = right;
    hbox->addLayout(right, 4);
    right->setAlignment(Qt::AlignTop);

    // Load the program panel specified in the persistent setting.
    m_program_list->load_persistent_panel();

    GlobalSettings::instance().WINDOW_SIZE->WIDTH.add_listener(*this);
    GlobalSettings::instance().WINDOW_SIZE->HEIGHT.add_listener(*this);
    GlobalSettings::instance().WINDOW_SIZE->X_POS.add_listener(*this);
    GlobalSettings::instance().WINDOW_SIZE->Y_POS.add_listener(*this);    
    SystemSleepController::instance().add_listener(*this);
//    cout << "Done constructing" << endl;
}
MainWindow::~MainWindow(){
    close_panel();
    SystemSleepController::instance().remove_listener(*this);
    GlobalSettings::instance().WINDOW_SIZE->WIDTH.remove_listener(*this);
    GlobalSettings::instance().WINDOW_SIZE->HEIGHT.remove_listener(*this);
    GlobalSettings::instance().WINDOW_SIZE->X_POS.remove_listener(*this);
    GlobalSettings::instance().WINDOW_SIZE->Y_POS.remove_listener(*this);    
}

int32_t move_x_within_screen_bounds(int32_t x_pos){
    auto static const screen_geometry = QGuiApplication::primaryScreen()->availableGeometry();
    uint32_t static const screen_width = (uint32_t)screen_geometry.width();    
     // ensure move_x is greater than 0, but less than screen_width
    return scale_dpi_width(std::max(0, std::min(x_pos, (int32_t)(screen_width*0.97))));

}

int32_t move_y_within_screen_bounds(int32_t y_pos){
    auto static const screen_geometry = QGuiApplication::primaryScreen()->availableGeometry();
    uint32_t const screen_height = (uint32_t)screen_geometry.height();  
    // ensure move_y is greater than 0, but less than screen_height
    return scale_dpi_width(std::max(0, std::min(y_pos, (int32_t)(screen_height*0.97))));

}


void MainWindow::closeEvent(QCloseEvent* event){
    close_all_windows();
    QMainWindow::closeEvent(event);
}
void MainWindow::resizeEvent(QResizeEvent* event){
    m_pending_resize = true;
    GlobalSettings::instance().WINDOW_SIZE->WIDTH.set(width());
    GlobalSettings::instance().WINDOW_SIZE->HEIGHT.set(height());
    m_pending_resize = false;
}

void MainWindow::moveEvent(QMoveEvent* event){
    m_pending_move = true;
    GlobalSettings::instance().WINDOW_SIZE->X_POS.set(x());
    GlobalSettings::instance().WINDOW_SIZE->Y_POS.set(y());
    m_pending_move = false;
}

void MainWindow::close_panel() noexcept{
//    cout << "close_panel(): enter: " << m_current_panel_widget << endl;
    //  Must destroy the widget first since it references the instance.
    if (m_current_panel_widget != nullptr){
        //  Possible optimization. Hide it so Qt doesn't need to recalculate
        //  all the layouts.
        m_current_panel_widget->hide();

        m_right_panel_layout->removeWidget(m_current_panel_widget);
        QWidget* widget = m_current_panel_widget;
        m_current_panel_widget = nullptr;
        delete widget;
    }

//    cout << "close_panel(): mid: " << m_current_panel_widget << endl;

    //  Now it's safe to destroy the instance.
    if (m_current_panel == nullptr){
        return;
    }

    try{
        m_current_panel->save_settings();
    }catch (...){}

    m_current_panel.reset();
    m_current_panel_descriptor.reset();
}

bool MainWindow::report_new_panel_intent(const PanelDescriptor& descriptor){
    //  No active panel. Proceed with panel change.
    if (m_current_panel == nullptr){
        return true;
    }

    //  Panel is already active. Don't change.
    if (&m_current_panel->descriptor() == &descriptor){
        m_current_panel->save_settings();
        return false;
    }

    return true;
}
void MainWindow::load_panel(
    std::shared_ptr<const PanelDescriptor> descriptor,
    std::unique_ptr<PanelInstance> panel
){
    if (m_panel_transition){
        global_logger_tagged().log(
            "Ignoring attempted panel change while existing one is still in progress.",
            COLOR_RED
        );
        return;
    }

    m_panel_transition = true;
    close_panel();

    //  Make new widget.
    try{
        check_new_version();
        m_current_panel_widget = panel->make_widget(*this, *this);
//        cout << "load_panel() = " << m_current_panel_widget << endl;
        m_current_panel_descriptor = std::move(descriptor);
        m_current_panel = std::move(panel);
        m_right_panel_layout->addWidget(m_current_panel_widget);
    }catch (...){
        if (m_current_panel_widget != nullptr){
            delete m_current_panel_widget;
        }
        m_panel_transition = false;
        throw;
    }
    m_panel_transition = false;
}
void MainWindow::on_busy(){
    if (m_program_list){
        m_program_list->setEnabled(false);
        m_settings->setEnabled(false);
    }
}
void MainWindow::on_idle(){
    if (m_program_list){
        m_program_list->setEnabled(true);
        m_settings->setEnabled(true);
    }
}


void MainWindow::on_config_value_changed(void* object){
    if (object == &GlobalSettings::instance().WINDOW_SIZE->WIDTH || object == &GlobalSettings::instance().WINDOW_SIZE->HEIGHT){
        QMetaObject::invokeMethod(this, [this]{
            if (!m_pending_resize){
                resize(
                    GlobalSettings::instance().WINDOW_SIZE->WIDTH,
                    GlobalSettings::instance().WINDOW_SIZE->HEIGHT
                );
            }
        });
    }else if (object == &GlobalSettings::instance().WINDOW_SIZE->X_POS || object == &GlobalSettings::instance().WINDOW_SIZE->Y_POS){
        QMetaObject::invokeMethod(this, [this]{
            if (!m_pending_move){
                move(
                    move_x_within_screen_bounds(GlobalSettings::instance().WINDOW_SIZE->X_POS),
                    move_y_within_screen_bounds(GlobalSettings::instance().WINDOW_SIZE->Y_POS)
                );
            }
        });        
    }
}
void MainWindow::sleep_suppress_state_changed(SleepSuppress new_state){
    QMetaObject::invokeMethod(this, [=, this]{
        switch (new_state){
        case PokemonAutomation::SleepSuppress::NONE:
            m_sleep_text->setText("Sleep Suppress: <font color=\"green\">None</font>");
//            m_sleep_box->setEnabled();
//            m_sleep_box->setCheckState(Qt::CheckState::Unchecked);
            break;
        case PokemonAutomation::SleepSuppress::NO_SLEEP:
            m_sleep_text->setText("Sleep Suppress: <font color=\"orange\">No Sleep</font>");
            break;
        case PokemonAutomation::SleepSuppress::SCREEN_ON:
            m_sleep_text->setText("Sleep Suppress: <font color=\"red\">Screen On</font>");
            break;
        }
    });
}




}

