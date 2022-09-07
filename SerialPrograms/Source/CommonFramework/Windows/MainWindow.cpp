/*  Main Window UI
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QMenuBar>
//#include <QStatusBar>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include "Common/Compiler.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/PersistentSettings.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Logging/FileWindowLogger.h"
#include "PanelLists.h"
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
//    QSize window_size = PERSISTENT_SETTINGS().window_size;
    resize(GlobalSettings::instance().WINDOW_SIZE.WIDTH, GlobalSettings::instance().WINDOW_SIZE.HEIGHT);
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


    QGroupBox* program_box = new QGroupBox("Program Select", centralwidget);
    left_layout->addWidget(program_box, 1);
    QVBoxLayout* program_layout = new QVBoxLayout(program_box);
    program_layout->setAlignment(Qt::AlignTop);

    m_program_list = new ProgramTabs(*this, *this);
    program_layout->addWidget(m_program_list);


    QGroupBox* support_box = new QGroupBox(
        QString::fromStdString(PROGRAM_NAME + " " + PROGRAM_VERSION + " (" + PA_ARCH_STRING + ")"),
        centralwidget
    );
    left_layout->addWidget(support_box);
    QVBoxLayout* support_layout = new QVBoxLayout(support_box);

    QHBoxLayout* support = new QHBoxLayout();
    support_layout->addLayout(support);
//    support->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout* links = new QVBoxLayout();
    support->addLayout(links);

    {
        QLabel* github = new QLabel(support_box);
        links->addWidget(github);
        github->setText(QString::fromStdString(make_text_url(ONLINE_DOC_URL, "Online Documentation")));
        github->setTextFormat(Qt::RichText);
        github->setTextInteractionFlags(Qt::TextBrowserInteraction);
        github->setOpenExternalLinks(true);
    }
    {
        QLabel* discord = new QLabel(support_box);
        links->addWidget(discord);
        discord->setText(QString::fromStdString(make_text_url(DISCORD_LINK_URL, DISCORD_LINK)));
        discord->setTextFormat(Qt::RichText);
        discord->setTextInteractionFlags(Qt::TextBrowserInteraction);
        discord->setOpenExternalLinks(true);
    }
    {
        QLabel* github = new QLabel(support_box);
        links->addWidget(github);
        github->setText(QString::fromStdString(make_text_url(PROJECT_GITHUB_URL, PROJECT_GITHUB)));
//        github->setText("<a href=\"" + GITHUB_REPO + "\">GitHub Repository</a>");
        github->setTextFormat(Qt::RichText);
        github->setTextInteractionFlags(Qt::TextBrowserInteraction);
        github->setOpenExternalLinks(true);
    }
    {
        QLabel* about = new QLabel(support_box);
        links->addWidget(about);
        about->setText(QString::fromStdString(make_text_url(PROJECT_GITHUB_URL, "About this Program")));
        about->setTextFormat(Qt::RichText);
        connect(
            about, &QLabel::linkActivated,
            this, [](const QString&){
                QMessageBox box;
                box.information(
                    nullptr,
                    "About",
                    QString::fromStdString(
                        PROGRAM_NAME + " Computer-Control Programs (" + PROGRAM_VERSION + ")<br>" +
                        "Copyright: 2020 - 2022<br>" +
                        "<br>"
                        "Made by the " + PROGRAM_NAME + " Discord Server.<br>"
                        "<br>"
                        "This program uses Qt and dynamically links to unmodified Qt libraries under LGPL.<br>"
                    )
                );
            }
        );
    }

    QVBoxLayout* buttons = new QVBoxLayout();
    support->addLayout(buttons);

    QPushButton* keyboard = new QPushButton("Keyboard Layout", support_box);
    buttons->addWidget(keyboard);
    QMainWindow* keyboard_window = new ButtonDiagram(*support_box);
    connect(
        keyboard, &QPushButton::clicked,
        this, [keyboard_window](bool){
            keyboard_window->show();
        }
    );

    m_output_window.reset(new FileWindowLoggerWindow((FileWindowLogger&)global_logger_raw()));
    QPushButton* output = new QPushButton("Output Window", support_box);
    buttons->addWidget(output);
    connect(
        output, &QPushButton::clicked,
        this, [this](bool){
            m_output_window->show();
            m_output_window->raise(); // bring the window to front on macOS
            m_output_window->activateWindow(); // bring the window to front on Windows
        }
    );

    QPushButton* settings = new QPushButton("Settings", support_box);
    m_settings = settings;
    buttons->addWidget(settings);
    connect(
        settings, &QPushButton::clicked,
        this, [this](bool){
            if (report_new_panel_intent(GlobalSettings_Descriptor::INSTANCE)){
                load_panel(GlobalSettings_Descriptor::INSTANCE.make_panel());
            }
        }
    );

    QVBoxLayout* right = new QVBoxLayout();
    m_right_panel_layout = right;
    hbox->addLayout(right, 4);
    right->setAlignment(Qt::AlignTop);

    // Load the program panel specified in the persistent setting.
    m_program_list->load_persistent_panel();

//    cout << "Done constructing" << endl;
}
MainWindow::~MainWindow(){
    close_panel();
}


void MainWindow::closeEvent(QCloseEvent* event){
    m_output_window->close();
    QMainWindow::closeEvent(event);
}
void MainWindow::resizeEvent(QResizeEvent* event){
    GlobalSettings::instance().WINDOW_SIZE.WIDTH.set(width());
    GlobalSettings::instance().WINDOW_SIZE.HEIGHT.set(height());
}

void MainWindow::close_panel(){
    //  Must destroy the widget first since it references the instance.
    if (m_current_panel_widget != nullptr){
        m_right_panel_layout->removeWidget(m_current_panel_widget);
        delete m_current_panel_widget;
        m_current_panel_widget = nullptr;
    }

    //  Now it's safe to destroy the instance.
    if (m_current_panel == nullptr){
        return;
    }

    m_current_panel->save_settings();
#if 0
    const std::string& identifier = m_current_panel->descriptor().identifier();
    if (!identifier.empty()){
        PERSISTENT_SETTINGS().panels[QString::fromStdString(identifier)] = m_current_panel->to_json();
    }
#endif

    m_current_panel.reset();
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
void MainWindow::load_panel(std::unique_ptr<PanelInstance> panel){
    close_panel();

    //  Make new widget.
    m_current_panel_widget = panel->make_widget(*this, *this);
    m_current_panel = std::move(panel);
    m_right_panel_layout->addWidget(m_current_panel_widget);
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





}

