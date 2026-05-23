#pragma once
#ifndef DPP_UTILITY_H
#define DPP_UTILITY_H

#include <dpp/dpp.h>
#include <atomic>
#include "Common/Cpp/Logging/AbstractLogger.h"

namespace PokemonAutomation{
namespace Integration{
namespace DppUtility{


class Utility{
public:
    Utility(){}

protected:
    static std::map<std::string, std::atomic_int> user_counts;

protected:
    void log(const std::string& message, const std::string& identity, const dpp::loglevel& ll);
    void get_user_counts(dpp::cluster& bot, const dpp::guild_create_t& event);
    int64_t get_value_from_input(const dpp::commandhandler& handler, const std::string& cmd, const std::string& input, std::string& out);
    int64_t sanitize_integer_input(const dpp::parameter_list_t& params, const uint8_t& index);
    uint16_t get_button(const uint16_t& bt);

private:
    Logger& dpp_logger();
};



}
}
}
#endif
