/*  BBQ Diagnostic Logger
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <sstream>
#include <string_view>
#include "Common/Cpp/PrettyPrint.h"
#include "PokemonSV_BBQLogger.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


namespace{

constexpr std::string_view UNKNOWN = "unknown";

std::string_view quest_label(BBQuests quest){
    const std::string& slug = BBQuests_enum_to_string(quest);
    return slug.empty() ? UNKNOWN : std::string_view(slug);
}

void append_elapsed(std::ostream& out, std::string_view field, BBQAttemptLog::Clock::duration elapsed){
    const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
    out << ' ' << field << "_ms=" << milliseconds.count()
        << ' ' << field << "=\"" << duration_to_string(milliseconds) << '\"';
}

}


void BBQLogger::board(bool panel_detected, const std::vector<BBQuests>& quests){
    std::ostringstream out;
    out << "BBQ event=board panel_detected=" << panel_detected << " quests=";
    bool first = true;
    for (BBQuests quest : quests){
        if (!first) out << ',';
        first = false;
        out << quest_label(quest);
    }
    m_logger.log(out.str());
}

void BBQLogger::battle_entry_timeout(BBQuests quest){
    std::ostringstream out;
    out << "BBQ quest=" << quest_label(quest) << " event=battle_entry outcome=timeout";
    m_logger.log(out.str());
}

void BBQLogger::retry_exhausted(BBQuests quest){
    std::ostringstream out;
    out << "BBQ quest=" << quest_label(quest) << " event=retry_exhausted recovery=skip_current_batch";
    m_logger.log(out.str());
}


BBQAttemptLog::BBQAttemptLog(
    Logger& logger, BBQuests quest,
    unsigned attempt, unsigned limit,
    Clock::time_point start
)
    : m_logger(logger), m_start(start), m_route_end(start)
{
    std::ostringstream out;
    out << "BBQ quest=" << quest_label(quest) << " attempt=" << attempt << " limit=" << limit;
    m_attempt_description = out.str();
    out << " event=start";
    m_logger.log(out.str());
}

void BBQAttemptLog::route_return(Clock::time_point end){
    m_route_end = end;
    std::ostringstream out;
    out << m_attempt_description << " event=route_return";
    append_elapsed(out, "elapsed", end - m_start);
    m_logger.log(out.str());
}

void BBQAttemptLog::result(bool completed, Clock::time_point end){
    std::ostringstream out;
    out << m_attempt_description << " event=result outcome="
        << (completed ? "absent_from_board" : "still_on_board");
    append_elapsed(out, "verification", end - m_route_end);
    append_elapsed(out, "total", end - m_start);
    m_logger.log(out.str());
}


}
}
}
