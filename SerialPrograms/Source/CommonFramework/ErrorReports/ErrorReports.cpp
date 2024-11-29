/*  Error Reports
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <iostream>
#include <QDir>
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "ProgramDumper.h"
#include "ErrorReports.h"

//  REMOVE
#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{



const std::string& ERROR_LOGS_NAME = "Logs.log";
const std::string& ERROR_DUMP_NAME = "Minidump.dmp";
const std::string& ERROR_PATH_UNSENT = "ErrorReportsLocal";
const std::string& ERROR_PATH_SENT = "ErrorReportsSent";



ErrorReportOption::ErrorReportOption()
    : GroupOption("Error Reports", LockMode::UNLOCK_WHILE_RUNNING, true)
    , DESCRIPTION(
        "Send error reports to the " + PROGRAM_NAME + " server to help them resolve issues and improve the program."
    )
    , SCREENSHOT(
        "<b>Include Screenshots:</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
    , LOGS(
        "<b>Include Logs:</b><br>Include the recent log leading up to the error.",
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
    , DUMPS(
        "<b>Include Dumps:</b><br>This saves stack-trace and related information.",
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
#if 0
    , FILES(
        "<b>Include Other Files:</b><br>Include other files that may be helpful for the developers.",
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
#endif
{
    PA_ADD_STATIC(DESCRIPTION);
    PA_ADD_OPTION(SCREENSHOT);
    PA_ADD_OPTION(LOGS);
    PA_ADD_OPTION(DUMPS);
//    PA_ADD_OPTION(FILES);
}


SendableErrorReport::SendableErrorReport()
    : m_timestamp(now_to_filestring())
    , m_directory(ERROR_PATH_UNSENT + "/" + m_timestamp + "/")
    , m_program(PreloadSettings::instance().DEVELOPER_MODE
        ? PROGRAM_NAME + " (" + PROGRAM_VERSION + "-dev)"
        : PROGRAM_NAME + " (" + PROGRAM_VERSION + ")"
    )
{
    QDir().mkdir(QString::fromStdString(m_directory));
}
SendableErrorReport::SendableErrorReport(
    Logger* logger,
    const ProgramInfo& info,
    std::string title,
    std::vector<std::pair<std::string, std::string>> messages,
    const ImageViewRGB32& image
)
    : SendableErrorReport()
{
    if (logger){
        logger->log("Compiling Error Report...");
    }else{
        std::cout << "Compiling Error Report..." << std::endl;
    }
    m_program_id = info.program_id;
    m_program_runtime_millis = std::chrono::duration_cast<Milliseconds>(current_time() - info.start_time).count();
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
        m_files.emplace_back(ERROR_LOGS_NAME);
    }
    if (program_dump(logger, m_directory + ERROR_DUMP_NAME)){
        m_files.emplace_back(ERROR_DUMP_NAME);
    }
}

SendableErrorReport::SendableErrorReport(std::string directory)
    : m_directory(std::move(directory))
{
    if (m_directory.back() != '/'){
        m_directory += '/';
    }

    try{
        m_image_owner = ImageRGB32(m_directory + "Image.png");
        m_image = m_image_owner;
    }catch (FileException&){}

    JsonValue json = load_json_file(m_directory + "Report.json");
    const JsonObject& obj = json.to_object_throw();
    m_timestamp = obj.get_string_throw("Timestamp");
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
        const JsonArray& files = obj.get_array_throw("Files");
        for (const JsonValue& file : files){
            m_files.emplace_back(file.to_string_throw());
        }
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

    m_image.save(m_directory + "Image.png");

    JsonArray array;
    for (const std::string& file : m_files){
        array.push_back(file);
    }
    report["Files"] = std::move(array);

    report.dump(m_directory + "Report.json");
}

#ifndef PA_OFFICIAL
bool SendableErrorReport::send(Logger& logger){
    return false;
}
#endif

void SendableErrorReport::move_to_sent(){
//    cout << "move_to_sent()" << endl;
    QDir().mkdir(QString::fromStdString(ERROR_PATH_SENT));

    std::string new_directory = ERROR_PATH_SENT + "/" + m_timestamp + "/";
//    cout << "old: " << m_directory << endl;
//    cout << "new: " << new_directory << endl;
    QDir().rename(
        QString::fromStdString(m_directory),
        QString::fromStdString(new_directory)
    );
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
void SendableErrorReport::send_reports(Logger& logger, const std::vector<std::string>& reports){
    for (const std::string& path : reports){
        try{
            SendableErrorReport report(path);
            report.send(logger);
        }catch (Exception& e){
            logger.log("Unable to send report: " + path + ", Message: " + e.to_str(), COLOR_RED);
        }catch (...){
            logger.log("Unable to send report: " + path, COLOR_RED);
        }
    }
}
void SendableErrorReport::send_all_unsent_reports(Logger& logger){
#ifdef PA_OFFICIAL
    if (GlobalSettings::instance().ERROR_REPORTS.enabled()){
        std::vector<std::string> reports = SendableErrorReport::get_pending_reports();
        global_logger_tagged().log("Found " + std::to_string(reports.size()) + " unsent error reports. Attempting to send...", COLOR_PURPLE);
        SendableErrorReport::send_reports(global_logger_tagged(), reports);
    }
#endif
}

void report_error(
    Logger* logger,
    const ProgramInfo& info,
    std::string title,
    std::vector<std::pair<std::string, std::string>> messages,
    const ImageViewRGB32& image,
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
            image
        );

        std::vector<std::string> full_file_paths;
        for (const std::string& file: files){
            full_file_paths.emplace_back(report.directory() + file);
        }

        report.save(logger);
    }

    SendableErrorReport::send_all_unsent_reports(*logger);
}




}
