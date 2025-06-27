/*  File and Window Logger
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QCoreApplication>
#include <QMenuBar>
#include <QDir>
#include "CommonFramework/Globals.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Windows/DpiScaler.h"
#include "CommonFramework/Windows/WindowTracker.h"
#include "CommonFramework/Windows/MainWindow.h"
#include "CommonFramework/Options/ResolutionOption.h"
#include "FileWindowLogger.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


Logger& global_logger_raw(){
    static FileWindowLogger logger(USER_FILE_PATH() + (QCoreApplication::applicationName() + ".log").toStdString());
    return logger;
}


void LastLogTracker::operator+=(std::string line){
    m_lines.emplace_back(std::move(line));
    while (m_lines.size() > m_max_lines){
        m_lines.pop_front();
    }
}
std::vector<std::string> LastLogTracker::snapshot() const{
    return std::vector<std::string>(m_lines.begin(), m_lines.end());
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
    , m_max_queue_size(LOG_HISTORY_LINES)
    , m_stopping(false)
{
    bool exists = m_file.exists();
    m_file.open(QIODevice::WriteOnly | QIODevice::Append);
    if (!exists){
        std::string bom = "\xef\xbb\xbf";
        m_file.write(bom.c_str(), bom.size());
    }

    m_thread = std::thread(&FileWindowLogger::thread_loop, this);
}
void FileWindowLogger::operator+=(FileWindowLoggerWindow& widget){
//    auto scope_check = m_sanitizer.check_scope();
    std::lock_guard<std::mutex> lg(m_lock);
    m_windows.insert(&widget);
}
void FileWindowLogger::operator-=(FileWindowLoggerWindow& widget){
//    auto scope_check = m_sanitizer.check_scope();
    std::lock_guard<std::mutex> lg(m_lock);
    m_windows.erase(&widget);
}

void FileWindowLogger::log(const std::string& msg, Color color){
//    auto scope_check = m_sanitizer.check_scope();
    std::unique_lock<std::mutex> lg(m_lock);
    m_last_log_tracker += msg;
    m_cv.wait(lg, [this]{ return m_queue.size() < m_max_queue_size; });
    m_queue.emplace_back(msg, color);
    m_cv.notify_all();
}
void FileWindowLogger::log(std::string&& msg, Color color){
//    auto scope_check = m_sanitizer.check_scope();
    std::unique_lock<std::mutex> lg(m_lock);
    m_last_log_tracker += msg;
    m_cv.wait(lg, [this]{ return m_queue.size() < m_max_queue_size; });
    m_queue.emplace_back(std::move(msg), color);
    m_cv.notify_all();
}
std::vector<std::string> FileWindowLogger::get_last() const{
//    auto scope_check = m_sanitizer.check_scope();
    std::unique_lock<std::mutex> lg(m_lock);
    return m_last_log_tracker.snapshot();
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
//    auto scope_check = m_sanitizer.check_scope();
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
//    auto scope_check = m_sanitizer.check_scope();
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
    uint32_t width = GlobalSettings::instance().LOG_WINDOW_SIZE->WIDTH;
    uint32_t height = GlobalSettings::instance().LOG_WINDOW_SIZE->HEIGHT;
    resize(scale_dpi_width(width), scale_dpi_height(height));
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

    GlobalSettings::instance().LOG_WINDOW_SIZE->WIDTH.add_listener(*this);
    GlobalSettings::instance().LOG_WINDOW_SIZE->HEIGHT.add_listener(*this);
    GlobalSettings::instance().LOG_WINDOW_SIZE->X_POS.add_listener(*this);
    GlobalSettings::instance().LOG_WINDOW_SIZE->Y_POS.add_listener(*this);  

    m_logger += *this;
    log("================================================================================");
    log("<b>Window Startup...</b>");
    log("Current path: " + QDir::currentPath());
    log("Executable path: " + qApp->applicationDirPath());
    log(QString::fromStdString("Program setting folder: " + SETTINGS_PATH()));
    log(QString::fromStdString("Program resources folder: " + RESOURCE_PATH()));
    add_window(*this);
}
FileWindowLoggerWindow::~FileWindowLoggerWindow(){
    remove_window(*this);
    m_logger -= *this;
    GlobalSettings::instance().LOG_WINDOW_SIZE->WIDTH.remove_listener(*this);
    GlobalSettings::instance().LOG_WINDOW_SIZE->HEIGHT.remove_listener(*this);
    GlobalSettings::instance().LOG_WINDOW_SIZE->X_POS.remove_listener(*this);
    GlobalSettings::instance().LOG_WINDOW_SIZE->Y_POS.remove_listener(*this);      
}

void FileWindowLoggerWindow::log(QString msg){
//    cout << "FileWindowLoggerWindow::log(): " << msg.toStdString() << endl;
    emit signal_log(msg);
}

void FileWindowLoggerWindow::resizeEvent(QResizeEvent* event){
    m_pending_resize = true;
    GlobalSettings::instance().LOG_WINDOW_SIZE->WIDTH.set(width());
    GlobalSettings::instance().LOG_WINDOW_SIZE->HEIGHT.set(height());
    m_pending_resize = false;
}

void FileWindowLoggerWindow::moveEvent(QMoveEvent* event){
    m_pending_move = true;    
    GlobalSettings::instance().LOG_WINDOW_SIZE->X_POS.set(x());
    GlobalSettings::instance().LOG_WINDOW_SIZE->Y_POS.set(y());
    m_pending_move = false;
}

void FileWindowLoggerWindow::on_config_value_changed(void* object){
    if (object == &GlobalSettings::instance().LOG_WINDOW_SIZE->WIDTH || object == &GlobalSettings::instance().LOG_WINDOW_SIZE->HEIGHT){
        QMetaObject::invokeMethod(this, [this]{
            if (!m_pending_resize){
                resize(
                    GlobalSettings::instance().LOG_WINDOW_SIZE->WIDTH,
                    GlobalSettings::instance().LOG_WINDOW_SIZE->HEIGHT
                );
            }
        });
    }else if (object == &GlobalSettings::instance().LOG_WINDOW_SIZE->X_POS || object == &GlobalSettings::instance().LOG_WINDOW_SIZE->Y_POS){
        QMetaObject::invokeMethod(this, [this]{
            if (!m_pending_move){
                move(
                    move_x_within_screen_bounds(GlobalSettings::instance().LOG_WINDOW_SIZE->X_POS),
                    move_y_within_screen_bounds(GlobalSettings::instance().LOG_WINDOW_SIZE->Y_POS)
                );
            }
        });        
    }
}




















}
