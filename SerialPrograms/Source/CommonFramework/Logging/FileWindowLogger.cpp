/*  File and Window Logger
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QCoreApplication>
#include <QMenuBar>
#include "CommonFramework/Windows/DpiScaler.h"
#include "CommonFramework/Windows/WindowTracker.h"
#include "FileWindowLogger.h"

namespace PokemonAutomation{


Logger& global_logger_raw(){
    static FileWindowLogger logger((QCoreApplication::applicationName() + ".log").toStdString());
    return logger;
}


FileWindowLogger::FileWindowLogger(const std::string& path)
    : m_file(QString::fromStdString(path))
{
    bool exists = m_file.exists();
    m_file.open(QIODevice::WriteOnly | QIODevice::Append);
    if (!exists){
        std::string bom = "\xef\xbb\xbf";
        m_file.write(bom.c_str(), bom.size());
    }
}
void FileWindowLogger::operator+=(FileWindowLoggerWindow& widget){
    std::lock_guard<std::mutex> lg(m_lock);
    m_windows.insert(&widget);
}
void FileWindowLogger::operator-=(FileWindowLoggerWindow& widget){
    std::lock_guard<std::mutex> lg(m_lock);
    m_windows.erase(&widget);
}

void FileWindowLogger::log(const char* msg, Color color){
    log(std::string(msg), color);
}
void FileWindowLogger::log(const std::string& msg, Color color){
    std::string line = normalize_newlines(msg);
    std::lock_guard<std::mutex> lg(m_lock);
    {
        if (!m_windows.empty()){
            QString str = to_window_str(line, color);
            for (FileWindowLoggerWindow* window : m_windows){
                window->log(str);
            }
        }
    }
    {
        m_file.write(to_file_str(msg).c_str());
        m_file.flush();
    }
}


std::string FileWindowLogger::normalize_newlines(const std::string& msg){
    std::string str;
    size_t index = 0;

    while (true){
        auto pos = msg.find("\r\n", index);
        if (pos == std::string::npos){
            str += msg.substr(index, pos);
            break;
        }else{
            str += msg.substr(index, pos);
            str += "\n";
            index = pos + 2;
        }
    }

    if (!str.empty() && str.back() == '\n'){
        str.pop_back();
    }

    return str;
}
std::string FileWindowLogger::to_file_str(const std::string& msg){
    //  Replace all newlines with:
    //      <br>    for the output window.
    //      \r\n    for the log file.

    std::string str;
    for (char ch : msg){
        if (ch == '\n'){
            str += "\r\n";
            continue;
        }
        str += ch;
    }
    str += "\r\n";

    return str;
}
QString FileWindowLogger::to_window_str(const std::string& msg, Color color){
    //  Replace all newlines with:
    //      <br>    for the output window.
    //      \r\n    for the log file.

    std::string str;
    if (color){
        str += "<font color=\"" + QColor((uint32_t)color).name().toStdString() + "\">";
    }else{
        str += "<font>";
    }
    for (char ch : msg){
        if (ch == ' '){
            str += "&nbsp;";
            continue;
        }
        if (ch == '\n'){
            str += "<br>";
            continue;
        }
        str += ch;
    }
//    if (color){
        str += "</font>";
//    }

    return QString::fromStdString(str);
}


FileWindowLoggerWindow::FileWindowLoggerWindow(FileWindowLogger& logger, QWidget* parent)
    : QMainWindow(parent)
    , m_logger(logger)
{
    if (objectName().isEmpty()){
        setObjectName(QString::fromUtf8("TextWindow"));
    }
    resize(scale_dpi_width(1200), scale_dpi_height(600));
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

    connect(
        this, &FileWindowLoggerWindow::signal_log,
        m_text, &QTextEdit::append
    );

    m_logger += *this;
    log("================================================================================");
    log("<b>Window Startup...</b>");
    add_window(*this);
}
FileWindowLoggerWindow::~FileWindowLoggerWindow(){
    remove_window(*this);
    m_logger -= *this;
}

void FileWindowLoggerWindow::log(const QString& msg){
    emit signal_log(msg);
}




















}
