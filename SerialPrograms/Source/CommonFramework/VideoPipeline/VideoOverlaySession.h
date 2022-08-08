/*  Video Overlay Session
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_VideoPipeline_VideoOverlaySession_H
#define PokemonAutomation_VideoPipeline_VideoOverlaySession_H

#include <memory>
#include <vector>
#include <set>
#include <map>
#include "Common/Compiler.h"
#include "Common/Cpp/SpinLock.h"
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "VideoOverlay.h"

namespace PokemonAutomation{


class VideoOverlaySession : public VideoOverlay{
public:
    struct Box{
        ImageFloatBox box;
        Color color;
    };

public:
    struct Listener{
        virtual void box_update(const std::shared_ptr<const std::vector<Box>>& boxes){}
    };
    void add_listener(Listener& listener);
    void remove_listener(Listener& listener);

public:
    std::vector<Box> boxes() const;
    virtual void add_box(const ImageFloatBox& box, Color color) override;
    virtual void remove_box(const ImageFloatBox& box) override;

private:
    void push_box_update();

private:
    mutable SpinLock m_lock;
    std::map<const ImageFloatBox*, Color> m_boxes;

    std::set<Listener*> m_listeners;
};



}
#endif
