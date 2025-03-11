/*  Raid Lobby Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *
 *      Determine if a raid is full and ready to start early.
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_RaidLobbyReader_H
#define PokemonAutomation_PokemonSwSh_RaidLobbyReader_H

#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


enum class RaidLobbySlot{
    EMPTY,
    NOT_READY,
    READY,
};

struct RaidLobbyState{
    bool valid = false;
    RaidLobbySlot player0 = RaidLobbySlot::EMPTY;
    RaidLobbySlot player1 = RaidLobbySlot::EMPTY;
    RaidLobbySlot player2 = RaidLobbySlot::EMPTY;
    RaidLobbySlot player3 = RaidLobbySlot::EMPTY;

    bool raid_is_full() const{
        return
            player0 != RaidLobbySlot::EMPTY &&
            player1 != RaidLobbySlot::EMPTY &&
            player2 != RaidLobbySlot::EMPTY &&
            player3 != RaidLobbySlot::EMPTY;
    }
    bool raiders_are_ready() const{
        return
            player1 != RaidLobbySlot::NOT_READY &&
            player2 != RaidLobbySlot::NOT_READY &&
            player3 != RaidLobbySlot::NOT_READY;
    }

    size_t raiders() const{
        size_t count = 0;
        if (player1 != RaidLobbySlot::EMPTY) count++;
        if (player2 != RaidLobbySlot::EMPTY) count++;
        if (player3 != RaidLobbySlot::EMPTY) count++;
        return count;
    }
};


class RaidLobbyReader{
public:
    RaidLobbyReader(Logger& logger, VideoOverlay& overlay);

    RaidLobbyState read(const ImageViewRGB32& screen);

private:
    Logger& m_logger;
    OverlayBoxScope m_checkbox0;
    OverlayBoxScope m_checkbox1;
    OverlayBoxScope m_checkbox2;
    OverlayBoxScope m_checkbox3;
    OverlayBoxScope m_spritebox0;
    OverlayBoxScope m_spritebox1;
    OverlayBoxScope m_spritebox2;
    OverlayBoxScope m_spritebox3;
};


}
}
}
#endif

