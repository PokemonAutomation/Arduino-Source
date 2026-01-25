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

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


Logger& global_logger_raw(){
    auto get_log_filepath = [&](){
        QString application_name(QCoreApplication::applicationName());
        if (application_name.size() == 0){
            application_name = "SerialPrograms";
        }
        return USER_FILE_PATH() + (application_name + ".log").toStdString();
    };

    static FileWindowLogger logger(get_log_filepath(), LOG_HISTORY_LINES);
    return logger;
}


FileWindowLogger::~FileWindowLogger(){
    m_file_logger.remove_listener(*this);
}

FileWindowLogger::FileWindowLogger(const std::string& path, size_t max_queue_size)
    : m_file_logger(FileLoggerConfig{
        .file_path = path,
        .max_queue_size = max_queue_size,
        .max_file_size_bytes = 50 * 1024 * 1024,  // 50MB
        .last_log_max_lines = max_queue_size,
    })
{
    m_file_logger.add_listener(*this);
}

void FileWindowLogger::operator+=(FileWindowLoggerWindow& widget){
    std::lock_guard<std::mutex> lg(m_window_lock);
    m_windows.insert(&widget);
}

void FileWindowLogger::operator-=(FileWindowLoggerWindow& widget){
    std::lock_guard<std::mutex> lg(m_window_lock);
    m_windows.erase(&widget);
}

void FileWindowLogger::log(const std::string& msg, Color color){
    m_file_logger.log(msg, color);
}

void FileWindowLogger::log(std::string&& msg, Color color){
    m_file_logger.log(std::move(msg), color);
}

std::vector<std::string> FileWindowLogger::get_last() const{
    return m_file_logger.get_last();
}

void FileWindowLogger::on_log(const std::string& msg, Color color){
    // This is called from FileLogger's background thread.
    // Format the message for Qt display and send to all windows.
    std::lock_guard<std::mutex> lg(m_window_lock);
    if (!m_windows.empty()){
        QString str = to_window_str(msg, color);
        for (FileWindowLoggerWindow* window : m_windows){
            window->log(str);
        }
    }
}

QString FileWindowLogger::to_window_str(const std::string& msg, Color color){
    // Convert message to HTML for display in QTextEdit.
    // Replace spaces with &nbsp; and newlines with <br>.
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
    str += "</font>";

    return QString::fromStdString(str);
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
    this->setWindowIcon(QIcon(QString::fromStdString(RESOURCE_PATH() + "icon.png")));

    m_text->setReadOnly(true);
    m_text->setAcceptRichText(true);
    m_text->document()->setMaximumBlockCount(1000);

    connect(
        this, &FileWindowLoggerWindow::signal_log,
        m_text, [this](QString msg){
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
