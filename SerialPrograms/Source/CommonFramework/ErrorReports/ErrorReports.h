/*  Error Reports
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ErrorReports_H
#define PokemonAutomation_ErrorReports_H

#include <memory>
#include "Common/Cpp/Logger/AbstractLogger.h"
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


// Filename constants for error report components
extern const std::string& ERROR_LOGS_NAME;        // "Logs.log" - The log file in each error report
extern const std::string& ERROR_DUMP_NAME;        // "Minidump.dmp" - The minidump file if available
extern const std::string& ERROR_PATH_UNSENT;      // "ErrorReportsLocal" - Directory path for unsent error reports
extern const std::string& ERROR_PATH_SENT;        // "ErrorReportsSent" - Directory path for sent error reports



// Determines when error reports should be sent to the developers
enum class ErrorReportSendMode{
    SEND_AUTOMATICALLY,         // Send error reports immediately without user interaction
    PROMPT_WHEN_CONVENIENT,     // Ask user for confirmation before sending
    NEVER_SEND_ANYTHING,        // Never send error reports
};

// UI options for configuring error report behavior and content
class ErrorReportOption : public GroupOption{
public:
    ErrorReportOption();

    StaticTextOption DESCRIPTION;

    EnumDropdownOption<ErrorReportSendMode> SEND_MODE;  // When to send error reports

    BooleanCheckBoxOption SCREENSHOT;  // Include screenshot in error report
    BooleanCheckBoxOption VIDEO;       // Include video history leading up to error (if available)
    BooleanCheckBoxOption LOGS;        // Include recent log entries
    BooleanCheckBoxOption DUMPS;       // Include stack trace and minidump
    BooleanCheckBoxOption FILES;       // Include additional files
};



// Represents a complete error report that can be saved locally and sent to developers.
// Each report is stored in its own timestamped directory (e.g., ErrorReportsLocal/20250216-155318967416/)
// containing: Screenshot.png, Logs.log, Report.json, and optionally Video.mp4 and Minidump.dmp
class SendableErrorReport{
public:
    // Default constructor: Creates an empty error report with timestamp and directory
    SendableErrorReport();

    // Create a new error report from current error info.
    // - Creates a timestamped directory in ErrorReportsLocal/
    // - Generates Report.json with metadata
    // Parameters:
    //   logger: Logger for status messages during report creation
    //   info: Program metadata (name, ID, runtime)
    //   title: Human-readable title for the error (e.g., "UnexpectedSelectionArrow")
    //   messages: Additional key-value message pairs to include
    //   image: Screenshot to report
    //   stream_history: Video history session to report
    SendableErrorReport(
        Logger* logger,
        const ProgramInfo& info = ProgramInfo(),
        std::string title = "",
        std::vector<std::pair<std::string, std::string>> messages = {},
        const ImageViewRGB32& image = ImageViewRGB32(),
        const StreamHistorySession* stream_history = nullptr
    );

    // Load an existing error report from disk by reading its Report.json
    // Used to reload previously saved reports for sending
    // If it fails to read the saved report, it will just move the report
    // files to ErrorReportsSent/ folder.
    SendableErrorReport(std::string directory);

    // Get the full directory path where this error report is stored or to be stored
    // e.g. "ErrorReportsLocal/20250216-155318967416"
    const std::string& directory() const{
        return m_directory;
    }

    // Get the human-readable title of this error report
    const std::string& title() const{
        return m_title;
    }

    // Get the additional key-value message pairs included in this report
    const std::vector<std::pair<std::string, std::string>>& messages() const{
        return m_messages;
    }

    // Add an additional file to be included in this error report
    void add_file(std::string filename);

    // Save an error report JSON "Report.json" in `directory()`:
    // Also save the image to the error report directory.
    void save_report_json(Logger* logger) const;

    // Move this error report from ErrorReportsLocal/ to ErrorReportsSent/
    // Called after successfully sending a report to mark it as sent
    void move_to_sent();

    // Get a list of all pending (unsent) error report directories from ErrorReportsLocal/
    static std::vector<std::string> get_pending_reports();

    // Send single error report to the developers' server
    // Only functional in official builds (PA_OFFICIAL defined)
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


// Send multiple error reports asynchronously in the background
// Loads each report from disk and sends it to the developers' server
void send_reports(Logger& logger, const std::vector<std::string>& reports);

// Only functional in official builds (PA_OFFICIAL defined):
// Attempt to send all unsent error reports in ErrorReportsLocal/ by calling `send_reports()`.
// Behavior depends on GlobalSettings ERROR_REPORTS.SEND_MODE:
// - SEND_AUTOMATICALLY: Sends all reports immediately
// - PROMPT_WHEN_CONVENIENT: Shows dialog asking user for permission (if allow_prompt is true)
// - NEVER_SEND_ANYTHING: Does nothing
// Returns: AsyncTask handle for background sending operation, or nullptr if not sending
std::unique_ptr<AsyncTask> send_all_unsent_reports(Logger& logger, bool allow_prompt);


// Create and save a complete error report. This is the main entry point for error reporting.
// Workflow:
// 1. Creates a SendableErrorReport with all provided information
// 2. Saves it to ErrorReportsLocal/<timestamp>/ directory with:
//    - Screenshot image
//    - Logs.log (recent log entries)
//    - Report.json (metadata including title, messages, timestamps)
//    - Video.mp4 (if stream_history provided)
//    - Minidump.dmp (if available)
//    - other additional files
// 3. Only functional in official builds (PA_OFFICIAL defined): call `send_all_unsent_reports()`
//    to attempt to send all unsent error reports based on user settings
//
// Parameters:
//   logger: Logger for status messages (uses global logger if nullptr)
//   info: Program metadata (name, ID, start time for calculating runtime)
//   title: Human-readable error name/label (e.g., "UnexpectedSelectionArrow", "NoHatchEnd")
//   messages: Additional key-value diagnostic information
//   image: Screenshot to include in the report
//   stream_history: Video recording session to save (captures video leading up to error)
//   files: Additional file paths to include in the report
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
