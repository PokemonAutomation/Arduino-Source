/*  Text Window for Output Logging
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QCoreApplication>
#include <QMenuBar>
//#include <QStatusBar>
#include "CommonFramework/PersistentSettings.h"
#include "OutputWindow.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{



TaggedLoggerOld::TaggedLoggerOld(OutputWindow& window, QString tag)
    : m_tag(std::move(tag))
{
    connect(
        this, &TaggedLoggerOld::signal_log,
        &window, &OutputWindow::log
    );
}
void TaggedLoggerOld::log(const char* msg, QColor color){
    QString body =
        QString::fromStdString(PokemonAutomation::current_time()) +
        " - [" + m_tag + "]: " +
        msg;
//    cout << color.toUtf8().data() << " - " << body.toUtf8().data() << endl;
    signal_log(std::move(body), color);
}
void TaggedLoggerOld::log(const std::string& msg, QColor color){
    QString body =
        QString::fromStdString(PokemonAutomation::current_time()) +
        " - [" + m_tag + "]: " +
        QString::fromStdString(msg);
//    cout << color.toUtf8().data() << " - " << body.toUtf8().data() << endl;
    signal_log(std::move(body), color);
}
void TaggedLoggerOld::log(const QString& msg, QColor color){
    QString body =
        QString::fromStdString(PokemonAutomation::current_time()) +
        " - [" + m_tag + "]: " +
        msg;
//    cout << color.toUtf8().data() << " - " << body.toUtf8().data() << endl;
    signal_log(std::move(body), color);
}


SerialLoggerOld::SerialLoggerOld(OutputWindow& window, QString tag)
    : TaggedLoggerOld(window, std::move(tag))
    , PokemonAutomation::MessageLogger(PERSISTENT_SETTINGS().log_everything)
{}
void SerialLoggerOld::log(std::string msg){
    TaggedLoggerOld::log(msg, "green");
}


OutputWindow::OutputWindow(QWidget* parent)
    : QMainWindow(parent)
{
    if (objectName().isEmpty()){
        setObjectName(QString::fromUtf8("TextWindow"));
    }
    resize(800, 600);
    m_text = new QTextEdit(this);
    m_text->setObjectName(QString::fromUtf8("centralwidget"));
    setCentralWidget(m_text);
    m_menubar = new QMenuBar(this);
    m_menubar->setObjectName(QString::fromUtf8("menubar"));
    setMenuBar(m_menubar);
//    m_statusbar = new QStatusBar(this);
//    m_statusbar->setObjectName(QString::fromUtf8("statusbar"));
//    setStatusBar(m_statusbar);
    setWindowTitle("Program Output");

    m_text->setReadOnly(true);
    m_text->setAcceptRichText(true);
    m_text->document()->setMaximumBlockCount(1000);
    m_default_color = m_text->textColor();

    m_log_file.setFileName(QCoreApplication::applicationFilePath() + ".log");
    bool exists = m_log_file.exists();
    m_log_file.open(QIODevice::WriteOnly | QIODevice::Append);
    if (!exists){
        std::string bom = "\xef\xbb\xbf";
        m_log_file.write(bom.c_str(), bom.size());
    }

    log("================================================================================", "");
    log(
        QString::fromStdString(PokemonAutomation::current_time()) +
        " - [Application]: " +
        "<b>Application Startup...</b>",
        ""
    );
}
OutputWindow::~OutputWindow(){
    m_log_file.close();
}

void OutputWindow::log(QString msg, QColor color){
    //  Replace all newlines with:
    //      <br>    for the output window.
    //      \r\n    for the log file.

    QString window_str = "<font color=\"" + (color.isValid() ? color.name() : m_default_color.name()) + "\">";
    QString file_str;
    bool pending_carrage_return = false;
    for (QChar ch : msg){
        if (pending_carrage_return && ch == '\n'){
            window_str += "<br>";
            file_str += "\r\n";
            pending_carrage_return = false;
            continue;
        }
        pending_carrage_return = false;
        if (ch == '\n'){
            window_str += "<br>";
            file_str += "\r\n";
            continue;
        }
        window_str += ch;
        file_str += ch;
    }

    if (file_str.back() == "\n"){
        window_str.resize(window_str.size() - 4);
        file_str.resize(file_str.size() - 2);
    }

    window_str += "</font>";
    m_text->append(window_str);

    msg += "\r\n";
    m_log_file.write(msg.toUtf8().data());
    m_log_file.flush();
}




}
