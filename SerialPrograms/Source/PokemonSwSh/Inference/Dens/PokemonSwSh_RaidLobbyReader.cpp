/*  Raid Lobby Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *
 *      Determine if a raid is full and ready to start early.
 *
 */

#include "CommonFramework/Inference/ImageTools.h"
#include "PokemonSwSh_RaidLobbyReader.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


RaidLobbyReader::RaidLobbyReader(Logger& logger, VideoOverlay& overlay)
    : m_logger(logger)
    , m_checkbox0(overlay, 0.593, 0.337 + 0.0775*0, 0.034, 0.06)
    , m_checkbox1(overlay, 0.593, 0.337 + 0.0775*1, 0.034, 0.06)
    , m_checkbox2(overlay, 0.593, 0.337 + 0.0775*2, 0.034, 0.06)
    , m_checkbox3(overlay, 0.593, 0.337 + 0.0775*3, 0.034, 0.06)
    , m_spritebox0(overlay, 0.820, 0.337 + 0.0775*0, 0.034, 0.06)
    , m_spritebox1(overlay, 0.820, 0.337 + 0.0775*1, 0.034, 0.06)
    , m_spritebox2(overlay, 0.820, 0.337 + 0.0775*2, 0.034, 0.06)
    , m_spritebox3(overlay, 0.820, 0.337 + 0.0775*3, 0.034, 0.06)
{}

RaidLobbyState RaidLobbyReader::read(const QImage& screen){
    if (screen.isNull()){
        m_logger.log("RaidLobbyReader(): Screenshot failed.", COLOR_PURPLE);
        return RaidLobbyState();
    }

    QString str;

    FloatPixel average0 = image_average(extract_box(screen, m_checkbox0));
    FloatPixel average1 = image_average(extract_box(screen, m_checkbox1));
    FloatPixel average2 = image_average(extract_box(screen, m_checkbox2));
    FloatPixel average3 = image_average(extract_box(screen, m_checkbox3));
    double distance1 = euclidean_distance(average0, average1);
    double distance2 = euclidean_distance(average0, average2);
    double distance3 = euclidean_distance(average0, average3);
    str += "Ready = {";
    str += QString::number(0) + ", ";
    str += QString::number(distance1) + ", ";
    str += QString::number(distance2) + ", ";
    str += QString::number(distance3) + "}";

    double stddev0 = image_stddev(extract_box(screen, m_spritebox0)).sum();
    double stddev1 = image_stddev(extract_box(screen, m_spritebox1)).sum();
    double stddev2 = image_stddev(extract_box(screen, m_spritebox2)).sum();
    double stddev3 = image_stddev(extract_box(screen, m_spritebox3)).sum();
    str += ", Sprites = {";
    str += QString::number(stddev0) + ", ";
    str += QString::number(stddev1) + ", ";
    str += QString::number(stddev2) + ", ";
    str += QString::number(stddev3) + "}";

    m_logger.log("RaidLobbyReader(): " + str, COLOR_PURPLE);

    const double PLAYER_READY = 100;
    const double PLAYER_EXISTS = 20;

    RaidLobbyState state;
    state.valid = true;
    state.player0 = stddev0 < PLAYER_EXISTS
        ? RaidLobbySlot::EMPTY
        : RaidLobbySlot::NOT_READY;
    state.player1 = stddev1 < PLAYER_EXISTS
        ? RaidLobbySlot::EMPTY
        : distance1 > PLAYER_READY
            ? RaidLobbySlot::READY
            : RaidLobbySlot::NOT_READY;
    state.player2 = stddev2 < PLAYER_EXISTS
        ? RaidLobbySlot::EMPTY
        : distance2 > PLAYER_READY
            ? RaidLobbySlot::READY
            : RaidLobbySlot::NOT_READY;
    state.player3 = stddev3 < PLAYER_EXISTS
        ? RaidLobbySlot::EMPTY
        : distance3 > PLAYER_READY
            ? RaidLobbySlot::READY
            : RaidLobbySlot::NOT_READY;
    return state;
}


}
}
}


