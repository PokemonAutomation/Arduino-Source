/*  Max Lair Detect Lobby
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Detect_Lobby_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Detect_Lobby_H

#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Inference/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{

#if 0
class LobbyReadyButtonDetector  : public VisualInferenceCallbackWithCommandStop{
public:
    LobbyReadyButtonDetector(VideoOverlay& overlay);

    bool detect(const QImage& screen);

    virtual bool on_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override final;


private:
    InferenceBoxScope m_box0;
    InferenceBoxScope m_box1;
};
#endif


#if 0
class NonLobbyDetector  : public VisualInferenceCallbackWithCommandStop{
public:
    NonLobbyDetector(VideoOverlay& overlay);

    bool detect(const QImage& screen);

    virtual bool on_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override final;


private:
    InferenceBoxScope m_pink;
    InferenceBoxScope m_white;
};
#endif


class LobbyDoneConnecting  : public VisualInferenceCallbackWithCommandStop{
public:
    LobbyDoneConnecting(VideoOverlay& overlay);

    bool detect(const QImage& screen);

    virtual bool on_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override final;


private:
    InferenceBoxScope m_box;
};


//  Detects when all joiners are readied up.
class LobbyAllReadyDetector : public VisualInferenceCallbackWithCommandStop{
public:
    LobbyAllReadyDetector(VideoOverlay& overlay, size_t consoles);

    bool detect(const QImage& screen);

    virtual bool on_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override final;


private:
    size_t m_consoles;
    InferenceBoxScope m_checkbox0;
    InferenceBoxScope m_checkbox1;
    InferenceBoxScope m_checkbox2;
    InferenceBoxScope m_checkbox3;
};



}
}
}
}
#endif
