/*  Let's Go HP Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_LetsGoHpReader_H
#define PokemonAutomation_PokemonSV_LetsGoHpReader_H

#include <deque>
#include "Common/Cpp/Color.h"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{
    class Logger;
namespace NintendoSwitch{
namespace PokemonSV{


class LetsGoHpWatcher : public VisualInferenceCallback{
public:
    LetsGoHpWatcher(Color color);

    void clear();
    double last_known_value() const;

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    Color m_color;
    ImageFloatBox m_box;
//    std::atomic<double> m_last_known_value;

    mutable SpinLock m_lock;
    std::deque<std::pair<WallClock, double>> m_history;
};




}
}
}
#endif
