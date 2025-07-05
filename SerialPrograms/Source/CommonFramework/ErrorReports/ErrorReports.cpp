/*  Error Reports
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <iostream>
#include <QDir>
#include <QMessageBox>
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Concurrency/AsyncDispatcher.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/GlobalServices.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Environment/Environment.h"
#include "CommonFramework/Options/Environment/ThemeSelectorOption.h"
#include "CommonFramework/Recording/StreamHistorySession.h"
#include "ProgramDumper.h"
#include "ErrorReports.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



const std::string& ERROR_LOGS_NAME = "Logs.log";
const std::string& ERROR_DUMP_NAME = "Minidump.dmp";
const std::string& ERROR_PATH_UNSENT = "ErrorReportsLocal";
const std::string& ERROR_PATH_SENT = "ErrorReportsSent";



ErrorReportOption::ErrorReportOption()
    : GroupOption(
        "Error Reports",
        LockMode::UNLOCK_WHILE_RUNNING,
        GroupOption::EnableMode::ALWAYS_ENABLED, true
    )
    , DESCRIPTION(
        "Send error reports to the " + PROGRAM_NAME + " server to help them resolve issues and improve the program."
    )
    , SEND_MODE(
        "<b>When to Send Error Reports:</b>",
        {
            {ErrorReportSendMode::SEND_AUTOMATICALLY,       "automatic",    "Send automatically."},
            {ErrorReportSendMode::PROMPT_WHEN_CONVENIENT,   "prompt",       "Prompt when convenient."},
            {ErrorReportSendMode::NEVER_SEND_ANYTHING,      "never",        "Never send error reports."},
        },
        LockMode::UNLOCK_WHILE_RUNNING,
        IS_BETA_VERSION
            ? ErrorReportSendMode::SEND_AUTOMATICALLY
            : ErrorReportSendMode::PROMPT_WHEN_CONVENIENT
    )
    , SCREENSHOT(
        "<b>Include Screenshots:</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
    , VIDEO(
        "<b>Include Video:</b><br>Include a video leading up to the error. (if available)",
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
    , LOGS(
        "<b>Include Logs:</b><br>Include the recent log leading up to the error.",
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
    , DUMPS(
        "<b>Include Dumps:</b><br>Include stack-trace and related information.",
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
    , FILES(
        "<b>Include Other Files:</b><br>Include other files that may be helpful for the developers.",
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
{
    PA_ADD_STATIC(DESCRIPTION);
    PA_ADD_OPTION(SEND_MODE);
    PA_ADD_OPTION(SCREENSHOT);
    PA_ADD_OPTION(VIDEO);
    PA_ADD_OPTION(LOGS);
    PA_ADD_OPTION(DUMPS);
    if (PreloadSettings::instance().DEVELOPER_MODE){
        PA_ADD_OPTION(FILES);
    }
}


SendableErrorReport::SendableErrorReport()
    : m_timestamp(now_to_filestring())
    , m_directory(ERROR_PATH_UNSENT + "/" + m_timestamp + "/")
    , m_processor(get_processor_name())
    , m_program(PreloadSettings::instance().DEVELOPER_MODE
        ? PROGRAM_NAME + " (" + PROGRAM_VERSION + "-dev)"
        : PROGRAM_NAME + " (" + PROGRAM_VERSION + ")"
    )
    , m_program_runtime_millis(0)
    , m_dump_name(ERROR_DUMP_NAME)
{
    QDir().mkpath(QString::fromStdString(m_directory));
}
SendableErrorReport::SendableErrorReport(
    Logger* logger,
    const ProgramInfo& info,
    std::string title,
    std::vector<std::pair<std::string, std::string>> messages,
    const ImageViewRGB32& image,
    const StreamHistorySession* stream_history
)
    : SendableErrorReport()
{
    if (logger){
        logger->log("Compiling Error Report...");
    }else{
        std::cout << "Compiling Error Report..." << std::endl;
    }
    m_program_id = info.program_id;
    if (info.start_time != WallClock::min()){
        m_program_runtime_millis = std::chrono::duration_cast<Milliseconds>(current_time() - info.start_time).count();
    }
    m_title = std::move(title);
    m_messages = std::move(messages);
    m_image = image;
    {
        std::string log;
        for (const std::string& line : global_logger_raw().get_last()){
            log += line;
            log += "\r\n";
        }
        QFile file(QString::fromStdString(m_directory + ERROR_LOGS_NAME));
        bool exists = file.exists();
        file.open(QIODevice::WriteOnly);
        if (!exists){
            std::string bom = "\xef\xbb\xbf";
            file.write(bom.c_str(), bom.size());
        }
        file.write(log.c_str());
        file.flush();
        m_logs_name = ERROR_LOGS_NAME;
    }
    if (stream_history){
        if (stream_history->save(m_directory + "Video.mp4")){
            m_video_name = "Video.mp4";
        }
    }
    if (program_dump(logger, m_directory + ERROR_DUMP_NAME)){
        m_dump_name = ERROR_DUMP_NAME;
    }
}

SendableErrorReport::SendableErrorReport(std::string directory)
    : m_directory(std::move(directory))
{
    if (m_directory.back() != '/'){
        m_directory += '/';
    }

    JsonValue json = load_json_file(m_directory + "Report.json");
    const JsonObject& obj = json.to_object_throw();
    m_timestamp = obj.get_string_throw("Timestamp");

    //  If we error from this point on, we'll just move it to the sent folder.
    try{
        m_processor = obj.get_string_throw("Processor");
        m_program = obj.get_string_throw("Program");
        m_program_id = obj.get_string_throw("ProgramID");
        m_program_runtime_millis = obj.get_integer_throw("ElapsedTimeMillis");
        m_title = obj.get_string_throw("Title");
        {
            const JsonArray& messages = obj.get_array_throw("Messages");
            for (const JsonValue& message : messages){
                const JsonArray& item = message.to_array_throw();
                if (item.size() != 2){
                    throw ParseException("Expected 2 values for message.");
                }
                m_messages.emplace_back(
                    item[0].to_string_throw(),
                    item[1].to_string_throw()
                );
            }
        }
        {
            const std::string* image_name = obj.get_string("Screenshot");
            if (image_name){
                try{
                    m_image_owner = ImageRGB32(m_directory + *image_name);
                    m_image = m_image_owner;
                }catch (FileException&){}
            }
        }
        {
            const std::string* video_name = obj.get_string("Video");
            if (video_name){
                m_video_name = *video_name;
            }
        }
        {
            const std::string* dump_name = obj.get_string("Dump");
            if (dump_name){
                m_dump_name = *dump_name;
            }
        }
        {
            const std::string* logs_name = obj.get_string("Logs");
            if (logs_name){
                m_logs_name = *logs_name;
            }
        }
        {
            const JsonArray& files = obj.get_array_throw("Files");
            for (const JsonValue& file : files){
                m_files.emplace_back(file.to_string_throw());
            }
        }
    }catch (...){
        move_to_sent();
        throw;
    }
}
void SendableErrorReport::add_file(std::string filename){
    m_files.emplace_back(std::move(filename));
}

void SendableErrorReport::save(Logger* logger) const{
    if (logger){
        logger->log("Saving Error Report...");
    }else{
        std::cout << "Saving Error Report..." << std::endl;
    }

    JsonObject report;

    report["Timestamp"] = m_timestamp;
    report["Processor"] = m_processor;
    report["Program"] = m_program;
    report["ProgramID"] = m_program_id;
    report["ElapsedTimeMillis"] = m_program_runtime_millis;
    report["Title"] = m_title;
    {
        JsonArray messages;
        for (const std::pair<std::string, std::string>& item : m_messages){
            JsonArray array;
            array.push_back(item.first);
            array.push_back(item.second);
            messages.push_back(std::move(array));
        }
        report["Messages"] = std::move(messages);
    }
    if (m_image){
        //  4k .png images are too big for current Discord limits.
        std::string extension = m_image.width() > 1920
            ? ".jpg"
            : ".png";
        if (m_image.save(m_directory + "Screenshot" + extension)){
            report["Screenshot"] = "Screenshot" + extension;
        }
    }
    if (!m_video_name.empty()){
        report["Video"] = m_video_name;
    }
    if (!m_dump_name.empty()){
        report["Dump"] = m_dump_name;
    }
    if (!m_logs_name.empty()){
        report["Logs"] = m_logs_name;
    }

    JsonArray array;
    for (const std::string& file : m_files){
        array.push_back(file);
    }
    report["Files"] = std::move(array);

    report.dump(m_directory + "Report.json");
}

void SendableErrorReport::move_to_sent(){
//    cout << "move_to_sent()" << endl;
    QDir().mkdir(QString::fromStdString(ERROR_PATH_SENT));

    std::string new_directory = ERROR_PATH_SENT + "/" + m_timestamp + "/";
//    cout << "old: " << m_directory << endl;
//    cout << "new: " << new_directory << endl;
    bool success = QDir().rename(
        QString::fromStdString(m_directory),
        QString::fromStdString(new_directory)
    );
    if (success){
        global_logger_tagged().log("Moved error report " + m_timestamp + ".");
    }else{
        global_logger_tagged().log("Unable to move error report " + m_timestamp + ".", COLOR_RED);
    }
    m_directory = std::move(new_directory);
}


std::vector<std::string> SendableErrorReport::get_pending_reports(){
    std::vector<std::string> ret;
    QDir dir(QString::fromStdString(ERROR_PATH_UNSENT));
    dir.setFilter(QDir::Filter::AllDirs);
    QFileInfoList list = dir.entryInfoList();
    for (const auto& item : list){
        std::string path = item.filePath().toStdString();
        if (path.back() == '.'){
            continue;
        }
        ret.emplace_back(std::move(path));
    }
    return ret;
}

#ifndef PA_OFFICIAL
void SendableErrorReport::send(Logger& logger, std::shared_ptr<SendableErrorReport> report){}
#endif



//  Send all the reports. This function will return early and all the reports
//  will be sent asynchronously in the background.
void send_reports(Logger& logger, const std::vector<std::string>& reports){
    for (const std::string& path : reports){
        try{
//            static int c = 0;
//            cout << "Sending... " << c++ << endl;
            //  std:::shared_ptr because it needs to take ownership for
            //  destruction at a later time.
            SendableErrorReport::send(logger, std::make_shared<SendableErrorReport>(path));
        }catch (Exception& e){
            logger.log("Unable to send report: " + path + ", Message: " + e.to_str(), COLOR_RED);
        }catch (...){
            logger.log("Unable to send report: " + path, COLOR_RED);
        }
    }
}
std::unique_ptr<AsyncTask> send_all_unsent_reports(Logger& logger, bool allow_prompt){
#ifdef PA_OFFICIAL
    ErrorReportSendMode mode = GlobalSettings::instance().ERROR_REPORTS->SEND_MODE;
    if (mode == ErrorReportSendMode::NEVER_SEND_ANYTHING){
        return nullptr;
    }

    std::vector<std::string> reports = SendableErrorReport::get_pending_reports();
    global_logger_tagged().log("Found " + std::to_string(reports.size()) + " unsent error reports.", COLOR_PURPLE);

    if (reports.empty()){
        return nullptr;
    }

    if (mode == ErrorReportSendMode::PROMPT_WHEN_CONVENIENT){
        if (!allow_prompt){
            return nullptr;
        }
        QMessageBox box;
        QMessageBox::StandardButton button = box.information(
            nullptr,
            "Error Reporting",
            QString::fromStdString(
                (reports.size() == 1
                    ? "There is " + tostr_u_commas(reports.size()) + " error report.<br><br>"
                      "Would you like to help make this program better by sending it to the developers?<br><br>"
                    : "There are " + tostr_u_commas(reports.size()) + " error reports.<br><br>"
                      "Would you like to help make this program better by sending them to the developers?<br><br>"
                ) +
                make_text_url(ERROR_PATH_UNSENT, "View unsent error reports.")// + "<br><br>"
//                "(You can change )"
            ),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::StandardButton::Yes
        );
        if (button != QMessageBox::StandardButton::Yes){
            return nullptr;
        }
    }

    global_logger_tagged().log("Attempting to send " + std::to_string(reports.size()) + " error reports.", COLOR_PURPLE);

    return global_async_dispatcher().dispatch([reports = std::move(reports)]{
        send_reports(global_logger_tagged(), reports);
    });
#else
    return nullptr;
#endif
}

void report_error(
    Logger* logger,
    const ProgramInfo& info,
    std::string title,
    std::vector<std::pair<std::string, std::string>> messages,
    const ImageViewRGB32& image,
    const StreamHistorySession* stream_history,
    const std::vector<std::string>& files
){
    if (logger == nullptr){
        logger = &global_logger_tagged();
    }

    {
        SendableErrorReport report(
            logger,
            info,
            std::move(title),
            std::move(messages),
            image,
            stream_history
        );

        std::vector<std::string> full_file_paths;
        for (const std::string& file: files){
            full_file_paths.emplace_back(report.directory() + file);
        }

        report.save(logger);
    }

    send_all_unsent_reports(*logger, false);
}




}
