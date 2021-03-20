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
#include "CommonFramework/Panels/RightPanel.h"
#include "CommonFramework/Widgets/ProgramList.h"
#include "CommonFramework/Widgets/SettingList.h"
#include "ButtonDiagram.h"
#include "MainWindow.h"

namespace PokemonAutomation{


MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_right_panel_widget(nullptr)
{
    if (objectName().isEmpty()){
        setObjectName(QString::fromUtf8("MainWindow"));
    }
    resize(settings.window_size.width(), settings.window_size.height());
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
    hbox->addLayout(left_layout, 2);


    QGroupBox* program_box = new QGroupBox("Program List", centralwidget);
    left_layout->addWidget(program_box, 1);
    QVBoxLayout* program_layout = new QVBoxLayout(program_box);
    program_layout->setAlignment(Qt::AlignTop);
    m_program_list = new ProgramListUI(*this);
    program_layout->addWidget(m_program_list);

    QGroupBox* setting_box = new QGroupBox("Global Settings", centralwidget);
    left_layout->addWidget(setting_box, 0);
    QVBoxLayout* setting_layout = new QVBoxLayout(setting_box);
    setting_layout->setAlignment(Qt::AlignTop);
    m_setting_list = new SettingListUI(*this);
    setting_layout->addWidget(m_setting_list);

#if 0
    int width = std::max(
        m_program_list->text_width(),
        m_settings_list->text_width()
    );
#else
        int width = 250;
#endif
    program_box->setMaximumWidth(width);
    setting_box->setMaximumWidth(width);

    QGroupBox* support_box = new QGroupBox("Support", centralwidget);
    left_layout->addWidget(support_box);
    QVBoxLayout* support_layout = new QVBoxLayout(support_box);

    QHBoxLayout* support = new QHBoxLayout();
    support_layout->addLayout(support);
//    support->setMargin(0);

    QVBoxLayout* links = new QVBoxLayout();
    support->addLayout(links);

    {
        QLabel* discord = new QLabel(support_box);
        links->addWidget(discord);
        discord->setText("<a href=\"" + DISCORD + "\">Discord Server</a>");
        discord->setTextFormat(Qt::RichText);
        discord->setTextInteractionFlags(Qt::TextBrowserInteraction);
        discord->setOpenExternalLinks(true);
    }
    {
        QLabel* github = new QLabel(support_box);
        links->addWidget(github);
        github->setText("<a href=\"" + GITHUB_REPO + "/blob/master/Documentation/README.md\">Online Documentation</a>");
        github->setTextFormat(Qt::RichText);
        github->setTextInteractionFlags(Qt::TextBrowserInteraction);
        github->setOpenExternalLinks(true);
    }
    {
        QLabel* github = new QLabel(support_box);
        links->addWidget(github);
        github->setText("<a href=\"" + GITHUB_REPO + "\">GitHub Repository</a>");
        github->setTextFormat(Qt::RichText);
        github->setTextInteractionFlags(Qt::TextBrowserInteraction);
        github->setOpenExternalLinks(true);
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

    QVBoxLayout* right = new QVBoxLayout();
    m_right_panel_layout = right;
    hbox->addLayout(right, 4);
    right->setAlignment(Qt::AlignTop);


}
MainWindow::~MainWindow(){
//    cout << "~MainWindow()" << endl;
    if (m_right_panel_widget != nullptr){
        m_right_panel_layout->removeWidget(m_right_panel_widget);
        delete m_right_panel_widget;
        m_right_panel_widget = nullptr;
    }
}


void MainWindow::left_panel_enabled(bool enabled){
    m_program_list->setEnabled(enabled);
    m_setting_list->setEnabled(enabled);
}
void MainWindow::change_panel(RightPanel& factory){
    if (m_right_panel_widget != nullptr){
        m_right_panel_layout->removeWidget(m_right_panel_widget);
        delete m_right_panel_widget;
        m_right_panel_widget = nullptr;
    }
    m_right_panel_widget = factory.make_ui(*this);
    m_right_panel_layout->addWidget(m_right_panel_widget);
}
void MainWindow::open_output_window(){
    m_output_window->show();
}

void MainWindow::closeEvent(QCloseEvent *event){
    m_output_window->close();
    QMainWindow::closeEvent(event);
}
void MainWindow::resizeEvent(QResizeEvent* event){
    settings.window_size = size();
}


}

