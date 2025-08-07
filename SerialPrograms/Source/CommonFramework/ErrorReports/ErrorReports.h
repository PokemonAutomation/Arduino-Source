/*  Error Reports
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ErrorReports_H
#define PokemonAutomation_ErrorReports_H

#include <memory>
#include "Common/Cpp/AbstractLogger.h"
#include "Common/Cpp/Options/GroupOption.h"
#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/Notifications/ProgramInfo.h"

namespace PokemonAutomation{


class AsyncTask;
class StreamHistorySession;


extern const std::string& ERROR_LOGS_NAME;
extern const std::string& ERROR_DUMP_NAME;
extern const std::string& ERROR_PATH_UNSENT;
extern const std::string& ERROR_PATH_SENT;



enum class ErrorReportSendMode{
    SEND_AUTOMATICALLY,
    PROMPT_WHEN_CONVENIENT,
    NEVER_SEND_ANYTHING,
};

class ErrorReportOption : public GroupOption{
public:
    ErrorReportOption();

    StaticTextOption DESCRIPTION;

    EnumDropdownOption<ErrorReportSendMode> SEND_MODE;

    BooleanCheckBoxOption SCREENSHOT;
    BooleanCheckBoxOption VIDEO;
    BooleanCheckBoxOption LOGS;
    BooleanCheckBoxOption DUMPS;
    BooleanCheckBoxOption FILES;
};



class SendableErrorReport{
public:
    SendableErrorReport();

    //  Create new report from current stack.
    SendableErrorReport(
        Logger* logger,
        const ProgramInfo& info = ProgramInfo(),
        std::string title = "",
        std::vector<std::pair<std::string, std::string>> messages = {},
        const ImageViewRGB32& image = ImageViewRGB32(),
        const StreamHistorySession* stream_history = nullptr
    );

    //  Deserialize from existing report.
    SendableErrorReport(std::string directory);

    const std::string& directory() const{
        return m_directory;
    }
    const std::string& title() const{
        return m_title;
    }
    const std::vector<std::pair<std::string, std::string>>& messages() const{
        return m_messages;
    }

    void add_file(std::string filename);

    void save(Logger* logger) const;
    void move_to_sent();

    static std::vector<std::string> get_pending_reports();
    static void send(Logger& logger, std::shared_ptr<SendableErrorReport> report);

private:
    std::string m_timestamp;
    std::string m_directory;
    std::string m_processor;
    std::string m_program;
    std::string m_program_id;
    uint64_t m_program_runtime_millis = 0;
    std::string m_title;
    std::vector<std::pair<std::string, std::string>> m_messages;
    ImageRGB32 m_image_owner;
    ImageViewRGB32 m_image;
    std::string m_logs_name;
    std::string m_video_name;
    std::string m_dump_name;
    std::vector<std::string> m_files;
};


void send_reports(Logger& logger, const std::vector<std::string>& reports);
std::unique_ptr<AsyncTask> send_all_unsent_reports(Logger& logger, bool allow_prompt);


void report_error(
    Logger* logger = nullptr,
    const ProgramInfo& info = ProgramInfo(),
    std::string title = "",
    std::vector<std::pair<std::string, std::string>> messages = {},
    const ImageViewRGB32& image = ImageViewRGB32(),
    const StreamHistorySession* stream_history = nullptr,
    const std::vector<std::string>& files = {}
);







}

#endif
