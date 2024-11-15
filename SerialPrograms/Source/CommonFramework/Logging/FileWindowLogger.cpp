/*  File and Window Logger
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QCoreApplication>
#include <QMenuBar>
#include <QDir>
#include <CommonFramework/Globals.h>
#include "CommonFramework/Windows/DpiScaler.h"
#include "CommonFramework/Windows/WindowTracker.h"
#include "FileWindowLogger.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


Logger& global_logger_raw(){
    static FileWindowLogger logger(USER_FILE_PATH() + (QCoreApplication::applicationName() + ".log").toStdString());
    return logger;
}


FileWindowLogger::~FileWindowLogger(){
    {
        std::lock_guard<std::mutex> lg(m_lock);
        m_stopping = true;
        m_cv.notify_all();
    }
    m_thread.join();
}
FileWindowLogger::FileWindowLogger(const std::string& path)
    : m_file(QString::fromStdString(path))
    , m_max_queue_size(10000)
    , m_stopping(false)
    , m_thread(&FileWindowLogger::thread_loop, this)
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

void FileWindowLogger::log(const std::string& msg, Color color){
    std::unique_lock<std::mutex> lg(m_lock);
    m_cv.wait(lg, [this]{ return m_queue.size() < m_max_queue_size; });
    m_queue.emplace_back(msg, color);
    m_cv.notify_all();
}
void FileWindowLogger::log(std::string&& msg, Color color){
    std::unique_lock<std::mutex> lg(m_lock);
    m_cv.wait(lg, [this]{ return m_queue.size() < m_max_queue_size; });
    m_queue.emplace_back(std::move(msg), color);
    m_cv.notify_all();
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
void FileWindowLogger::internal_log(const std::string& msg, Color color){
    std::string line = normalize_newlines(msg);
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
void FileWindowLogger::thread_loop(){
    std::unique_lock<std::mutex> lg(m_lock);
    while (true){
        m_cv.wait(lg, [&]{
            return m_stopping || !m_queue.empty();
        });
        if (m_stopping){
            break;
        }
        auto& item = m_queue.front();
        std::string msg = std::move(item.first);
        Color color = item.second;
        m_queue.pop_front();

        lg.unlock();
        internal_log(msg, color);
        lg.lock();

        if (m_queue.size() <= m_max_queue_size / 2){
            m_cv.notify_all();
        }
    }
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
        m_text, [this](QString msg){
//            cout << "signal_log(): " << msg.toStdString() << endl;
            m_text->append(msg);
        }
    );

    m_logger += *this;
    log("================================================================================");
    log("<b>Window Startup...</b>");
    log("Current path: " + QDir::currentPath());
    log("Executable path: " + qApp->applicationDirPath());
    add_window(*this);
}
FileWindowLoggerWindow::~FileWindowLoggerWindow(){
    remove_window(*this);
    m_logger -= *this;
}

void FileWindowLoggerWindow::log(QString msg){
//    cout << "FileWindowLoggerWindow::log(): " << msg.toStdString() << endl;
    emit signal_log(msg);
}




















}
