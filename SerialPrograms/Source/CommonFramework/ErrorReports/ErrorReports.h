/*  Error Reports
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ErrorReports_H
#define PokemonAutomation_ErrorReports_H

#include "Common/Cpp/AbstractLogger.h"
#include "Common/Cpp/Options/GroupOption.h"
#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/Notifications/ProgramInfo.h"

namespace PokemonAutomation{


extern const std::string& ERROR_LOGS_NAME;
extern const std::string& ERROR_DUMP_NAME;
extern const std::string& ERROR_PATH_UNSENT;
extern const std::string& ERROR_PATH_SENT;



class ErrorReportOption : public GroupOption{
public:
    ErrorReportOption();

    StaticTextOption DESCRIPTION;
    BooleanCheckBoxOption SCREENSHOT;
    BooleanCheckBoxOption LOGS;
    BooleanCheckBoxOption DUMPS;
//    BooleanCheckBoxOption FILES;
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
        const ImageViewRGB32& image = ImageViewRGB32()
    );

    //  Deserialize from existing report.
    SendableErrorReport(std::string directory);

    const std::string& directory() const{
        return m_directory;
    }

    void add_file(std::string filename);

    void save(Logger* logger) const;
    bool send(Logger& logger);
    void move_to_sent();

    static std::vector<std::string> get_pending_reports();
    static void send_reports(Logger& logger, const std::vector<std::string>& reports);
    static void send_all_unsent_reports(Logger& logger);

private:
    std::string m_timestamp;
    std::string m_directory;
    std::string m_program;
    std::string m_program_id;
    uint64_t m_program_runtime_millis;
    std::string m_title;
    std::vector<std::pair<std::string, std::string>> m_messages;
    ImageRGB32 m_image_owner;
    ImageViewRGB32 m_image;
    std::vector<std::string> m_files;
};



void report_error(
    Logger* logger = nullptr,
    const ProgramInfo& info = ProgramInfo(),
    std::string title = "",
    std::vector<std::pair<std::string, std::string>> messages = {},
    const ImageViewRGB32& image = ImageViewRGB32(),
    const std::vector<std::string>& files = {}
);


}

#endif
