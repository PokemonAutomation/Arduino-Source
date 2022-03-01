/*  Max Lair Detect Lobby
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Detect_Lobby_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Detect_Lobby_H

#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"
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
class LobbyDetector  : public VisualInferenceCallback{
public:
    LobbyDetector(bool invert);

    bool detect(const QImage& screen);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override final;


private:
    bool m_invert;
    ImageFloatBox m_pink;
    ImageFloatBox m_white;
};
#endif


class LobbyDoneConnecting  : public VisualInferenceCallback{
public:
    LobbyDoneConnecting();

    bool detect(const QImage& screen);

    virtual void make_overlays(VideoOverlaySet& items) const override;
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
    LobbyJoinedDetector(size_t consoles, bool invert);

    size_t joined(
        const QImage& screen,
        std::chrono::system_clock::time_point timestamp
    );

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override final;

private:
    size_t m_consoles;
    bool m_invert;

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
class LobbyReadyDetector : public VisualInferenceCallback{
public:
    LobbyReadyDetector();

    size_t ready_players(const QImage& screen);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override final;
    virtual bool detect(const QImage& screen) = 0;


private:
    ImageFloatBox m_checkbox0;
    ImageFloatBox m_checkbox1;
    ImageFloatBox m_checkbox2;
    ImageFloatBox m_checkbox3;
};

class LobbyMinReadyDetector : public LobbyReadyDetector{
public:
    LobbyMinReadyDetector(size_t consoles, bool invert);
    virtual bool detect(const QImage& screen) override;
private:
    size_t m_consoles;
    bool m_invert;
};
class LobbyAllReadyDetector : public LobbyReadyDetector{
public:
    LobbyAllReadyDetector(size_t consoles);
    virtual bool detect(const QImage& screen) override;
private:
    size_t m_consoles;
};



}
}
}
}
#endif
