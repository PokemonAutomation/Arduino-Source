/*  Formatted Log Methods Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <format>
#include <string>
#include <vector>
#include "Common/Cpp/Color.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "AbstractLogger.h"
#include "TaggedLogger.h"
#include "FormattedLogMethods_Tests.h"

namespace PokemonAutomation{
namespace{



// A logger that keeps every line it gets, so a test can check each line's text,
// color and level.
class CaptureLogger : public Logger{
public:
    struct Line{
        std::string text;
        Color color;
        LogLevel level;
    };

    using Logger::log;

    virtual void log(const std::string& msg, Color color) override{
        lines.push_back({msg, color, LogLevel::Info});
    }
    virtual void log(const std::string& msg, Color color, LogLevel level) override{
        lines.push_back({msg, color, level});
    }

    std::vector<Line> lines;
};


// A class that is not a `Logger` but passes its lines to one, like `VideoStream` and
// `ProgramEnvironment`.
class ForwardingLogger : public FormattedLogMethods<ForwardingLogger>{
public:
    ForwardingLogger(Logger& logger)
        : m_logger(logger)
    {}

    void log(const std::string& msg, Color color, LogLevel level){
        m_logger.log(msg, color, level);
    }

private:
    Logger& m_logger;
};


// Return "" if `logger` got exactly one line, with the expected text, color and level.
// Otherwise return what differs, e.g. "level: expected warning, got info".
std::string check_one_line(
    const CaptureLogger& logger,
    const std::string& text, Color color, LogLevel level
){
    if (logger.lines.size() != 1){
        return std::format("expected 1 line, got {}", logger.lines.size());
    }
    const CaptureLogger::Line& line = logger.lines[0];
    if (line.text != text){
        return std::format("text: expected \"{}\", got \"{}\"", text, line.text);
    }
    if (line.color != color){
        return std::format("color: expected {}, got {}", color.to_string(), line.color.to_string());
    }
    if (line.level != level){
        return std::format(
            "level: expected {}, got {}",
            log_level_to_str(level), log_level_to_str(line.level)
        );
    }
    return "";
}



class Test_FormattedLogMethods_Format : public UnitTest{
public:
    Test_FormattedLogMethods_Format()
        : UnitTest("Common::FormattedLogMethods - Format arguments")
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        struct Case{
            const char* what;
            void (*log)(CaptureLogger& target);
            const char* expected;
        };
        const Case CASES[] = {
            {
                "string and float arguments",
                [](CaptureLogger& target){ target.info("Read {} at {:.2f}, {:.2f}", "gastly", 0.6276, 0.4426); },
                "Read gastly at 0.63, 0.44",
            },
            {
                "no arguments",
                [](CaptureLogger& target){ target.info("No arguments"); },
                "No arguments",
            },
            {
                "doubled braces",
                [](CaptureLogger& target){ target.info("{{x}} = {}", 5); },
                "{x} = 5",
            },
            {
                "std::string argument",
                [](CaptureLogger& target){ target.info("Hello, {}!", std::string("world")); },
                "Hello, world!",
            },
        };
        for (const Case& item : CASES){
            CaptureLogger capture;
            item.log(capture);
            std::string error = check_one_line(capture, item.expected, Color(), LogLevel::Info);
            if (!error.empty()){
                return UnitTestResult(std::format("{}: {}", item.what, error));
            }
        }
        return true;
    }
};



class Test_FormattedLogMethods_LevelsAndColors : public UnitTest{
public:
    Test_FormattedLogMethods_LevelsAndColors()
        : UnitTest("Common::FormattedLogMethods - Levels and colors")
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        //  For each level: the method without a color, and the method with one.
        struct Case{
            LogLevel level;
            void (*log)(CaptureLogger& target);
            void (*log_with_color)(CaptureLogger& target, Color color);
        };
        const Case CASES[] = {
            {
                LogLevel::Verbose,
                [](CaptureLogger& target){ target.verbose("line {}", 1); },
                [](CaptureLogger& target, Color color){ target.verbose(color, "line {}", 1); },
            },
            {
                LogLevel::Debug,
                [](CaptureLogger& target){ target.debug("line {}", 1); },
                [](CaptureLogger& target, Color color){ target.debug(color, "line {}", 1); },
            },
            {
                LogLevel::Info,
                [](CaptureLogger& target){ target.info("line {}", 1); },
                [](CaptureLogger& target, Color color){ target.info(color, "line {}", 1); },
            },
            {
                LogLevel::Warning,
                [](CaptureLogger& target){ target.warning("line {}", 1); },
                [](CaptureLogger& target, Color color){ target.warning(color, "line {}", 1); },
            },
            {
                LogLevel::Error,
                [](CaptureLogger& target){ target.error("line {}", 1); },
                [](CaptureLogger& target, Color color){ target.error(color, "line {}", 1); },
            },
            {
                LogLevel::Critical,
                [](CaptureLogger& target){ target.critical("line {}", 1); },
                [](CaptureLogger& target, Color color){ target.critical(color, "line {}", 1); },
            },
        };
        for (const Case& item : CASES){
            const char* level_name = log_level_to_str(item.level);
            {
                CaptureLogger capture;
                item.log(capture);
                std::string error = check_one_line(capture, "line 1", log_level_color(item.level), item.level);
                if (!error.empty()){
                    return UnitTestResult(std::format("{}() without a color: {}", level_name, error));
                }
            }
            {
                CaptureLogger capture;
                item.log_with_color(capture, COLOR_BLUE);
                std::string error = check_one_line(capture, "line 1", COLOR_BLUE, item.level);
                if (!error.empty()){
                    return UnitTestResult(std::format("{}() with COLOR_BLUE: {}", level_name, error));
                }
            }
        }
        return true;
    }
};



class Test_FormattedLogMethods_Wrappers : public UnitTest{
public:
    Test_FormattedLogMethods_Wrappers()
        : UnitTest("Common::FormattedLogMethods - Wrapping loggers")
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        //  `TaggedLogger` overrides `log()`. Its lines must still reach the base logger
        //  with their level, after the timestamp and tag.
        {
            CaptureLogger capture;
            TaggedLogger tagged(capture, "Test");
            tagged.warning("Retry {} of {}", 1, 3);
            if (capture.lines.size() != 1){
                return UnitTestResult(std::format("TaggedLogger: expected 1 line, got {}", capture.lines.size()));
            }
            const CaptureLogger::Line& line = capture.lines[0];
            const std::string expected_end = " - [Test]: Retry 1 of 3";
            if (!line.text.ends_with(expected_end)){
                return UnitTestResult(std::format(
                    "TaggedLogger: expected the text to end with \"{}\", got \"{}\"",
                    expected_end, line.text
                ));
            }
            if (line.color != log_level_color(LogLevel::Warning) || line.level != LogLevel::Warning){
                return UnitTestResult(std::format(
                    "TaggedLogger: expected warning in {}, got {} in {}",
                    log_level_color(LogLevel::Warning).to_string(),
                    log_level_to_str(line.level), line.color.to_string()
                ));
            }
        }

        //  A class that is not a `Logger` but passes its lines to one.
        {
            CaptureLogger capture;
            ForwardingLogger forwarding(capture);
            forwarding.error(COLOR_BLUE, "Failed {} times", 2);
            std::string error = check_one_line(capture, "Failed 2 times", COLOR_BLUE, LogLevel::Error);
            if (!error.empty()){
                return UnitTestResult("Forwarding class: " + error);
            }
        }
        return true;
    }
};



} // anonymous namespace



void add_tests_FormattedLogMethods(UnitTestDatabase& database){
    database.add<Test_FormattedLogMethods_Format>();
    database.add<Test_FormattedLogMethods_LevelsAndColors>();
    database.add<Test_FormattedLogMethods_Wrappers>();
}



}
