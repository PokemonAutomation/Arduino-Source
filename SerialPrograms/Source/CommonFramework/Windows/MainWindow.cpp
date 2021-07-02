/*  Main Window UI
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QMenuBar>
//#include <QStatusBar>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include "CommonFramework/Globals.h"
#include "CommonFramework/PersistentSettings.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "PanelLists.h"
#include "ButtonDiagram.h"
#include "MainWindow.h"


//#include <Windows.h>

//#include <QJsonDocument>
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
    resize(PERSISTENT_SETTINGS().window_width, PERSISTENT_SETTINGS().window_height);
    centralwidget = new QWidget(this);
    centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
    setCentralWidget(centralwidget);
    menubar = new QMenuBar(this);
    menubar->setObjectName(QString::fromUtf8("menubar"));
    setMenuBar(menubar);
//    statusbar = new QStatusBar(this);
//    statusbar->setObjectName(QString::fromUtf8("statusbar"));
//    setStatusBar(statusbar);
    setWindowTitle(STRING_POKEMON + " Automation Serial Programs (" + VERSION + ")");

    QHBoxLayout* hbox = new QHBoxLayout(centralwidget);

    QVBoxLayout* left_layout = new QVBoxLayout();
    hbox->addLayout(left_layout, 0);


    QGroupBox* program_box = new QGroupBox("Program Select", centralwidget);
    left_layout->addWidget(program_box, 1);
    QVBoxLayout* program_layout = new QVBoxLayout(program_box);
    program_layout->setAlignment(Qt::AlignTop);

    m_program_list = new ProgramTabs(*this, *this);
    program_layout->addWidget(m_program_list);


    QGroupBox* support_box = new QGroupBox("Support (" + STRING_POKEMON + " Automation " + VERSION + ")", centralwidget);
    left_layout->addWidget(support_box);
    QVBoxLayout* support_layout = new QVBoxLayout(support_box);

    QHBoxLayout* support = new QHBoxLayout();
    support_layout->addLayout(support);
//    support->setMargin(0);

    QVBoxLayout* links = new QVBoxLayout();
    support->addLayout(links);

    {
        QLabel* github = new QLabel(support_box);
        links->addWidget(github);
        github->setText("<a href=\"" + ONLINE_DOC_URL + "/blob/master/README.md\">Online Documentation</a>");
        github->setTextFormat(Qt::RichText);
        github->setTextInteractionFlags(Qt::TextBrowserInteraction);
        github->setOpenExternalLinks(true);
    }
    {
        QLabel* discord = new QLabel(support_box);
        links->addWidget(discord);
        discord->setText("<a href=\"" + DISCORD_LINK_URL + "\">" + DISCORD_LINK + "</a>");
        discord->setTextFormat(Qt::RichText);
        discord->setTextInteractionFlags(Qt::TextBrowserInteraction);
        discord->setOpenExternalLinks(true);
    }
    {
        QLabel* github = new QLabel(support_box);
        links->addWidget(github);
        github->setText("<a href=\"" + PROJECT_GITHUB_URL + "\">" + PROJECT_GITHUB + "</a>");
//        github->setText("<a href=\"" + GITHUB_REPO + "\">GitHub Repository</a>");
        github->setTextFormat(Qt::RichText);
        github->setTextInteractionFlags(Qt::TextBrowserInteraction);
        github->setOpenExternalLinks(true);
    }
    {
        QLabel* about = new QLabel(support_box);
        links->addWidget(about);
        about->setText("<a href=\"" + PROJECT_GITHUB_URL + "\">" + "About this Program" + "</a>");
        about->setTextFormat(Qt::RichText);
        connect(
            about, &QLabel::linkActivated,
            this, [](const QString&){
                QMessageBox box;
                box.information(
                    nullptr,
                    "About",
                    STRING_POKEMON + " Automation Feedback Programs (" + VERSION + ")<br>" +
                    "Copyright: 2020 - 2021<br>" +
                    "<br>"
                    "Made by the " + STRING_POKEMON + " Automation Discord Server.<br>"
                    "<br>"
                    "This program uses Qt and dynamically links to unmodified Qt libraries under LGPL.<br>"
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
        this, [=](bool){
            keyboard_window->show();
        }
    );

    m_output_window.reset(new OutputWindow());
    QPushButton* output = new QPushButton("Output Window", support_box);
    buttons->addWidget(output);
    connect(
        output, &QPushButton::clicked,
        this, [=](bool){
            m_output_window->show();
            m_output_window->activateWindow();
        }
    );

#if 0
    QPushButton* about = new QPushButton("About", support_box);
    buttons->addWidget(about);
    connect(
        about, &QPushButton::clicked,
        this, [=](bool){
            QMessageBox box;
            box.information(
                nullptr,
                "About",
                STRING_POKEMON + " Automation Feedback Programs (" + VERSION + ")<br>" +
                "Copyright: 2020 - 2021<br>" +
                "<br>"
                "Made by the " + STRING_POKEMON + " Automation Discord Server.<br>"
                "<br>"
                "This program uses Qt and dynamically links to unmodified Qt libraries under LGPL.<br>"
            );
        }
    );
#endif

    QPushButton* settings = new QPushButton("Settings", support_box);
    m_settings = settings;
    buttons->addWidget(settings);
    connect(
        settings, &QPushButton::clicked,
        this, [=](bool){
            on_panel_construct(GlobalSettings_Descriptor::INSTANCE.make_panel());
        }
    );

    QVBoxLayout* right = new QVBoxLayout();
    m_right_panel_layout = right;
    hbox->addLayout(right, 4);
    right->setAlignment(Qt::AlignTop);


}
MainWindow::~MainWindow(){
    close_panel();
}



void MainWindow::open_output_window(){
    m_output_window->show();
}

void MainWindow::closeEvent(QCloseEvent* event){
    m_output_window->close();
    QMainWindow::closeEvent(event);
}
void MainWindow::resizeEvent(QResizeEvent* event){
//    PERSISTENT_SETTINGS().window_size = size();
    PERSISTENT_SETTINGS().window_width = width();
    PERSISTENT_SETTINGS().window_height = height();
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

    const std::string& identifier = m_current_panel->descriptor().identifier();
    PERSISTENT_SETTINGS().panels[identifier.c_str()] = m_current_panel->to_json();

    m_current_panel.reset();
}

void MainWindow::on_panel_construct(std::unique_ptr<PanelInstance> panel){
    close_panel();

    //  Make new widget.
    m_current_panel_widget = panel->make_widget(*this, *this);
    m_current_panel = std::move(panel);
    m_right_panel_layout->addWidget(m_current_panel_widget);
}
void MainWindow::on_busy(PanelInstance& panel){
    if (m_program_list){
        m_program_list->setEnabled(false);
        m_settings->setEnabled(false);
    }
}
void MainWindow::on_idle(PanelInstance& panel){
    if (m_program_list){
        m_program_list->setEnabled(true);
        m_settings->setEnabled(true);
    }
}





}

