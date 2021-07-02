/*  Message Pretty Printing
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include <set>
#include <map>
#include <sstream>
#include "Common/MessageProtocol.h"
#include "Common/Cpp/Exception.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonProgramIDs.h"
#include "MessageConverter.h"

namespace PokemonAutomation{


std::map<uint8_t, MessageConverter>& converter_map(){
    static std::map<uint8_t, MessageConverter> converters;
    return converters;
}
void register_message_converter(uint8_t type, MessageConverter converter){
    std::map<uint8_t, MessageConverter>& converters = converter_map();
    auto iter = converters.find(type);
    if (iter != converters.end()){
        PA_THROW_StringException("Duplicate message type.");
    }
    converters[type] = converter;
}


int register_message_converters_framework_errors(){
    register_message_converter(
        PABB_MSG_ERROR_READY,
        [](const std::string& body){
            std::stringstream ss;
            ss << "PABB_MSG_ERROR_READY - ";
            if (body.size() != 0){ ss << "(invalid size)" << std::endl; return ss.str(); }
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_ERROR_INVALID_MESSAGE,
        [](const std::string& body){
            std::stringstream ss;
            ss << "PABB_MSG_ERROR_INVALID_MESSAGE - ";
            if (body.size() != sizeof(pabb_MsgInfoInvalidMessage)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_MsgInfoInvalidMessage*)body.c_str();
            ss << "length = " << (unsigned)params->message_length;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_ERROR_CHECKSUM_MISMATCH,
        [](const std::string& body){
            std::stringstream ss;
            ss << "PABB_MSG_ERROR_CHECKSUM_MISMATCH - ";
            if (body.size() != sizeof(pabb_MsgInfoChecksumMismatch)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_MsgInfoChecksumMismatch*)body.c_str();
            ss << "length = " << (unsigned)params->message_length;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_ERROR_INVALID_TYPE,
        [](const std::string& body){
            std::stringstream ss;
            ss << "PABB_MSG_ERROR_INVALID_TYPE - ";
            if (body.size() != sizeof(pabb_MsgInfoInvalidType)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_MsgInfoInvalidType*)body.c_str();
            ss << "type = " << (unsigned)params->type;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_ERROR_INVALID_REQUEST,
        [](const std::string& body){
            std::stringstream ss;
            ss << "PABB_MSG_ERROR_INVALID_REQUEST - ";
            if (body.size() != sizeof(pabb_MsgInfoInvalidRequest)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_MsgInfoInvalidRequest*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_ERROR_MISSED_REQUEST,
        [](const std::string& body){
            std::stringstream ss;
            ss << "PABB_MSG_ERROR_MISSED_REQUEST - ";
            if (body.size() != sizeof(pabb_MsgInfoMissedRequest)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_MsgInfoMissedRequest*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_ERROR_COMMAND_DROPPED,
        [](const std::string& body){
            std::stringstream ss;
            ss << "PABB_MSG_ERROR_COMMAND_DROPPED - ";
            if (body.size() != sizeof(pabb_MsgInfoCommandDropped)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_MsgInfoCommandDropped*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_ERROR_WARNING,
        [](const std::string& body){
            std::stringstream ss;
            ss << "PABB_MSG_ERROR_WARNING - ";
            if (body.size() != sizeof(pabb_MsgInfoWARNING)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_MsgInfoWARNING*)body.c_str();
            ss << "error code = " << (unsigned)params->error_code;
            return ss.str();
        }
    );
    return 0;
}
int register_message_converters_framework_acks(){
    register_message_converter(
        PABB_MSG_ACK_COMMAND,
        [](const std::string& body){
            std::stringstream ss;
            ss << "PABB_MSG_ACK_COMMAND - ";
            if (body.size() != sizeof(pabb_MsgAckCommand)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_MsgAckCommand*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_ACK_REQUEST,
        [](const std::string& body){
            std::stringstream ss;
            ss << "PABB_MSG_ACK_REQUEST - ";
            if (body.size() != sizeof(pabb_MsgAckRequest)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_MsgAckRequest*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_ACK_REQUEST_I8,
        [](const std::string& body){
            std::stringstream ss;
            ss << "PABB_MSG_ACK_REQUEST_I8 - ";
            if (body.size() != sizeof(pabb_MsgAckRequestI8)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_MsgAckRequestI8*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", message = " << (unsigned)params->data;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_ACK_REQUEST_I16,
        [](const std::string& body){
            std::stringstream ss;
            ss << "PABB_MSG_ACK_REQUEST_I16 - ";
            if (body.size() != sizeof(pabb_MsgAckRequestI16)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_MsgAckRequestI16*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", message = " << (unsigned)params->data;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_ACK_REQUEST_I32,
        [](const std::string& body){
            std::stringstream ss;
            ss << "PABB_MSG_ACK_REQUEST_I32 - ";
            if (body.size() != sizeof(pabb_MsgAckRequestI32)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_MsgAckRequestI32*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", message = " << params->data;
            return ss.str();
        }
    );
    return 0;
}
int register_message_converters_framework_requests(){
    register_message_converter(
        PABB_MSG_SEQNUM_RESET,
        [](const std::string& body){
            std::stringstream ss;
            ss << "PABB_MSG_SEQNUM_RESET - ";
            if (body.size() != sizeof(pabb_MsgInfoSeqnumReset)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_MsgInfoSeqnumReset*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_REQUEST_PROTOCOL_VERSION,
        [](const std::string& body){
            std::stringstream ss;
            ss << "PABB_MSG_REQUEST_PROTOCOL_VERSION - ";
            if (body.size() != sizeof(pabb_MsgRequestProtocolVersion)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_MsgRequestProtocolVersion*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_REQUEST_PROGRAM_VERSION,
        [](const std::string& body){
            std::stringstream ss;
            ss << "PABB_MSG_REQUEST_PROGRAM_VERSION - ";
            if (body.size() != sizeof(pabb_MsgRequestProgramVersion)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_MsgRequestProgramVersion*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_REQUEST_PROGRAM_ID,
        [](const std::string& body){
            std::stringstream ss;
            ss << "PABB_MSG_REQUEST_PROGRAM_ID - ";
            if (body.size() != sizeof(pabb_MsgRequestProgramID)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_MsgRequestProgramID*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_REQUEST_CLOCK,
        [](const std::string& body){
            std::stringstream ss;
            ss << "PABB_MSG_REQUEST_CLOCK - ";
            if (body.size() != sizeof(pabb_system_clock)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_system_clock*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_REQUEST_COMMAND_FINISHED,
        [](const std::string& body){
            std::stringstream ss;
            ss << "PABB_MSG_REQUEST_COMMAND_FINISHED - ";
            if (body.size() != sizeof(pabb_MsgRequestCommandFinished)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_MsgRequestCommandFinished*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", seq_of_original_command = " << (unsigned)params->seq_of_original_command;
            ss << ", finish_time = " << params->finish_time;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_REQUEST_STOP,
        [](const std::string& body){
            std::stringstream ss;
            ss << "PABB_MSG_REQUEST_STOP - ";
            if (body.size() != sizeof(pabb_MsgRequestStop)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_MsgRequestStop*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_COMMAND_END_PROGRAM_CALLBACK,
        [](const std::string& body){
            std::stringstream ss;
            ss << "end_program_callback() - ";
            if (body.size() != sizeof(pabb_end_program_callback)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_end_program_callback*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_COMMAND_SET_LED_STATE,
        [](const std::string& body){
            std::stringstream ss;
            ss << "PABB_MSG_COMMAND_SET_LED_STATE - ";
            if (body.size() != sizeof(pabb_MsgCommandSetLeds)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_MsgCommandSetLeds*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", on = " << params->on;
            return ss.str();
        }
    );
    return 0;
}
int register_message_converters_custom_info(){
    register_message_converter(
        PABB_MSG_INFO_I32,
        [](const std::string& body){
            std::stringstream ss;
            ss << "PABB_MSG_INFO_I32 - ";
            if (body.size() != sizeof(pabb_MsgInfoI32)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_MsgInfoI32*)body.c_str();
            ss << "tag = " << (unsigned)params->tag;
            ss << ", data = " << params->data;
            return ss.str();
        }
    );
    return 0;
}
int register_message_converters_push_button_framework(){
    register_message_converter(
        PABB_MSG_COMMAND_PBF_WAIT,
        [](const std::string& body){
            std::stringstream ss;
            ss << "pbf_wait() - ";
            if (body.size() != sizeof(pabb_pbf_wait)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_pbf_wait*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", ticks = " << params->ticks;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_COMMAND_PBF_PRESS_BUTTON,
        [](const std::string& body){
            std::stringstream ss;
            ss << "pbf_press_button() - ";
            if (body.size() != sizeof(pabb_pbf_press_button)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_pbf_press_button*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", button = " << params->button;
            ss << ", hold_ticks = " << params->hold_ticks;
            ss << ", release_ticks = " << params->release_ticks;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_COMMAND_PBF_PRESS_DPAD,
        [](const std::string& body){
            std::stringstream ss;
            ss << "pbf_press_dpad() - ";
            if (body.size() != sizeof(pabb_pbf_press_dpad)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_pbf_press_dpad*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", dpad = " << (unsigned)params->dpad;
            ss << ", hold_ticks = " << params->hold_ticks;
            ss << ", release_ticks = " << params->release_ticks;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_COMMAND_PBF_MOVE_JOYSTICK_L,
        [](const std::string& body){
            std::stringstream ss;
            ss << "pbf_move_left_joystick() - ";
            if (body.size() != sizeof(pabb_pbf_move_joystick)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_pbf_move_joystick*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", x = " << (unsigned)params->x;
            ss << ", y = " << (unsigned)params->y;
            ss << ", hold_ticks = " << params->hold_ticks;
            ss << ", release_ticks = " << params->release_ticks;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_COMMAND_PBF_MOVE_JOYSTICK_R,
        [](const std::string& body){
            std::stringstream ss;
            ss << "pbf_move_right_joystick() - ";
            if (body.size() != sizeof(pabb_pbf_move_joystick)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_pbf_move_joystick*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", x = " << (unsigned)params->x;
            ss << ", y = " << (unsigned)params->y;
            ss << ", hold_ticks = " << params->hold_ticks;
            ss << ", release_ticks = " << params->release_ticks;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_COMMAND_MASH_BUTTON,
        [](const std::string& body){
            std::stringstream ss;
            ss << "pbf_mash_button() - ";
            if (body.size() != sizeof(pabb_pbf_mash_button)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_pbf_mash_button*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", button = " << params->button;
            ss << ", ticks = " << params->ticks;
            return ss.str();
        }
    );
    return 0;
}

int init_MessageLogger =
    register_message_converters_framework_errors() +
    register_message_converters_framework_acks() +
    register_message_converters_framework_requests() +
    register_message_converters_custom_info() +
    register_message_converters_push_button_framework();


std::string message_to_string(const BotBaseMessage& message){
    const std::map<uint8_t, MessageConverter>& converters = converter_map();
    auto iter = converters.find(message.type);
    if (iter == converters.end()){
        std::stringstream ss;
        ss << "Unknown Message Type " << (unsigned)message.type << ": length = " << message.body.size();
        return ss.str();
    }
    return iter->second(message.body);
}


std::string program_name(uint8_t id){
    switch (id){
    case PABB_PID_UNSPECIFIED:              return "Unspecified/Unknown";
    case PABB_PID_PABOTBASE_12KB:           return "PABotBase-12KB";
    case PABB_PID_PABOTBASE_31KB:           return "PABotBase-31KB";
    case PABB_PID_CUSTOM_PROGRAM:           return "Custom Program";
    case PABB_PID_SANDBOX:                  return "Sandbox";

    case PABB_PID_TURBOA:                   return "TurboA";
    case PABB_PID_MASS_RELEASE:             return "MassRelease";
    case PABB_PID_FAST_CODE_ENTRY:          return "FastCodeEntry";
    case PABB_PID_SURPRISE_TRADE:           return "SurpriseTrade";
    case PABB_PID_TRADE_BOT:                return "TradeBot";
    case PABB_PID_CLOTHING_BUYER:           return "ClothingBuyer";
    case PABB_PID_BALL_THROWER:             return "BallThrower";

    case PABB_PID_DATESPAM_WATT_FARMER:             return "DateSpam-WattFarmer";
    case PABB_PID_DATESPAM_BERRY_FARMER:            return "DateSpam-BerryFarmer";
    case PABB_PID_DATESPAM_LOTO_FARMER:             return "DateSpam-LotoFarmer";
    case PABB_PID_DATESPAM_STOW_ON_SIDE_FARMER:     return "DateSpam-StowOnSideFarmer";
    case PABB_PID_DATESPAM_DAILY_HIGHLIGHT_FARMER:  return "DateSpam-DailyHighlightFarmer";

    case PABB_PID_MULTIGAME_FOSSIL:                         return "MultiGameFossil";
    case PABB_PID_SHINYHUNT_REGI:                           return "ShinyHunt-Regi";
    case PABB_PID_SHINYHUNT_SWORDS_OF_JUSTICE:              return "ShinyHunt-SwordsOfJustice";
    case PABB_PID_SHINYHUNT_UNATTENDED_REGI:                return "ShinyHuntUnattended-Regi";
    case PABB_PID_SHINYHUNT_UNATTENDED_SWORDS_OF_JUSTICE:   return "ShinyHuntUnattended-SwordsOfJustice";
    case PABB_PID_SHINYHUNT_UNATTENDED_STRONG_SPAWN:        return "ShinyHuntUnattended-StrongSpawn";
    case PABB_PID_SHINYHUNT_UNATTENDED_REGIGIGAS:           return "ShinyHuntUnattended-Regigigas";
    case PABB_PID_SHINYHUNT_UNATTENDED_REGIGIGAS2:          return "ShinyHuntUnattended-Regigigas2";
    case PABB_PID_SHINYHUNT_UNATTENDED_IOA_TRADE:           return "ShinyHuntUnattended-IoATrade";

    case PABB_PID_BEAM_RESET:               return "BeamReset";
    case PABB_PID_EVENT_BEAM_FINDER:        return "EventBeamFinder";
    case PABB_PID_DAYSKIPPER_JPN:           return "DaySkipperJPN";
    case PABB_PID_DAYSKIPPER_EU:            return "DaySkipperEU";
    case PABB_PID_DAYSKIPPER_US:            return "DaySkipperUS";
    case PABB_PID_DAYSKIPPER_JPN_7P8K:      return "DaySkipperJPN-7.8k";

    case PABB_PID_FRIEND_DELETE:            return "FriendDelete";
    case PABB_PID_DEN_ROLLER:               return "DenRoller";
    case PABB_PID_AUTOHOST_ROLLING:         return "AutoHost-Rolling";
    case PABB_PID_AUTOHOST_MULTIGAME:       return "AutoHost-MultiGame";

    case PABB_PID_EGG_FETCHER2:             return "EggFetcher2";
    case PABB_PID_EGG_HATCHER:              return "EggHatcher";
    case PABB_PID_EGG_COMBINED2:            return "EggCombined2";
    case PABB_PID_EGG_SUPER_COMBINED2:      return "EggSuperCombined2";
    case PABB_PID_GOD_EGG_ITEM_DUPE:        return "GodEggItemDupe";
    case PABB_PID_GOD_EGG_DUPLICATION:      return "GodEggDuplication";

    default: return "Unknown ID";
    }
}




}

