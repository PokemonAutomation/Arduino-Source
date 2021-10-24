/*  Max Lair Detect Lobby
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Detect_Lobby_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Detect_Lobby_H

#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Inference/VisualInferenceCallback.h"
#include "CommonFramework/Inference/FrozenImageDetector.h"

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


#if 1
class NonLobbyDetector  : public VisualInferenceCallback{
public:
    NonLobbyDetector();

    bool detect(const QImage& screen);

    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override final;


private:
    ImageFloatBox m_pink;
    ImageFloatBox m_white;
};
#endif


class LobbyDoneConnecting  : public VisualInferenceCallback{
public:
    LobbyDoneConnecting();

    bool detect(const QImage& screen);

    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override final;


private:
    ImageFloatBox m_box;
    ImageFloatBox m_player0;
};


class LobbyJoinedDetector : public VisualInferenceCallback{
public:
    LobbyJoinedDetector(size_t consoles);

    size_t joined(
        const QImage& screen,
        std::chrono::system_clock::time_point timestamp
    );

    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override final;

private:
    size_t m_consoles;
    ImageFloatBox m_box0;
    ImageFloatBox m_box1;
    ImageFloatBox m_box2;
    ImageFloatBox m_box3;

    FrozenImageDetector m_player0;
    FrozenImageDetector m_player1;
    FrozenImageDetector m_player2;
    FrozenImageDetector m_player3;
};


//  Detects when all joiners are readied up.
class LobbyAllReadyDetector : public VisualInferenceCallback{
public:
    LobbyAllReadyDetector(
        size_t consoles,
        std::chrono::system_clock::time_point time_limit
    );

    bool detect(const QImage& screen);

    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override final;


private:
    size_t m_consoles;
    std::chrono::system_clock::time_point m_time_limit;
    ImageFloatBox m_checkbox0;
    ImageFloatBox m_checkbox1;
    ImageFloatBox m_checkbox2;
    ImageFloatBox m_checkbox3;
};



}
}
}
}
#endif
