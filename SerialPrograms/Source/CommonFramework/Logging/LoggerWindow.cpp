/*  File and Window Logger
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QCoreApplication>
#include <QTimer>
#include <QMenuBar>
#include <QDir>
#include "Common/Cpp/Logging/LastLogTracker.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Windows/DpiScaler.h"
#include "CommonFramework/Windows/WindowTracker.h"
#include "CommonFramework/Windows/MainWindow.h"
#include "CommonFramework/Options/ResolutionOption.h"
#include "LoggerWindow.h"

namespace PokemonAutomation{


void LoggerWindow::log(const std::string& msg, Color color){
    QString new_lines = to_window_str(msg, color);

    bool pending_update = false;
    {
        WriteSpinLock lg(m_lock, nullptr);
        pending_update = m_pending_update;
        m_pending_update = true;

        QString* qstr = m_pending.try_push_back();
        if (qstr == nullptr){
            m_pending.pop_front();
            qstr = &m_pending.push_back();
        }
        *qstr = std::move(new_lines);

        WallClock now = current_time();
        WallClock threshold = now - WINDOW_SIZE;
        while (!m_timestamps.empty() && m_timestamps.front() < threshold){
            m_timestamps.pop_front();
        }

        m_timestamps.try_push_back(now);

//        printf("Pending = %d, Later = %d\n", pending_update, update_later);
    }

    if (pending_update){
        return;
    }

    QMetaObject::invokeMethod(this, [this]{
        update();
    }, Qt::QueuedConnection);
}

QString LoggerWindow::to_window_str(const std::string& msg, Color color){
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


LoggerWindow::LoggerWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_timestamps(MAX_LOGS_PER_WINDOW)
    , m_pending(MAX_LINES)
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
    m_text->document()->setMaximumBlockCount(MAX_LINES);

    connect(
        this, &LoggerWindow::signal_log,
        m_text, [this](){
            update();
        }
    );

    GlobalSettings::instance().LOG_WINDOW_SIZE->WIDTH.add_listener(*this);
    GlobalSettings::instance().LOG_WINDOW_SIZE->HEIGHT.add_listener(*this);
    GlobalSettings::instance().LOG_WINDOW_SIZE->X_POS.add_listener(*this);
    GlobalSettings::instance().LOG_WINDOW_SIZE->Y_POS.add_listener(*this);

    for (const LogLine& item : global_last_log_history().get_recent(MAX_LINES)){
        log(item.text, item.color);
    }

    log("================================================================================");
    log("<b>Window Startup...</b>");
    add_window(*this);
}

LoggerWindow::~LoggerWindow(){
    remove_window(*this);
    GlobalSettings::instance().LOG_WINDOW_SIZE->WIDTH.remove_listener(*this);
    GlobalSettings::instance().LOG_WINDOW_SIZE->HEIGHT.remove_listener(*this);
    GlobalSettings::instance().LOG_WINDOW_SIZE->X_POS.remove_listener(*this);
    GlobalSettings::instance().LOG_WINDOW_SIZE->Y_POS.remove_listener(*this);
}

void LoggerWindow::update(){
    for (size_t c = 0; c < MAX_LOGS_PER_WINDOW; c++){
        QString line;
        bool full;
        {
            WriteSpinLock lg(m_lock, nullptr);
            if (m_pending.empty()){
                m_pending_update = false;
                return;
            }
            full = m_pending.full();
            line = std::move(m_pending.front());
            m_pending.pop_front();
        }
        if (full){
            m_text->clear();
        }
        m_text->append(line);
    }
    QTimer::singleShot(WINDOW_SIZE.count(), this, [this](){
        emit signal_log();
    });
}


void LoggerWindow::resizeEvent(QResizeEvent* event){
    m_pending_resize = true;
    GlobalSettings::instance().LOG_WINDOW_SIZE->WIDTH.set(width());
    GlobalSettings::instance().LOG_WINDOW_SIZE->HEIGHT.set(height());
    m_pending_resize = false;
}

void LoggerWindow::moveEvent(QMoveEvent* event){
    m_pending_move = true;
    GlobalSettings::instance().LOG_WINDOW_SIZE->X_POS.set(x());
    GlobalSettings::instance().LOG_WINDOW_SIZE->Y_POS.set(y());
    m_pending_move = false;
}

void LoggerWindow::on_config_value_changed(void* object){
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
