/*  Object Name Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonHome_HomeApplicationDetector_H
#define PokemonAutomation_PokemonHome_HomeApplicationDetector_H

#include <vector>
#include "Common/Cpp/Color.h"
#include "Common/Cpp/Containers/FixedLimitVector.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{

class VideoOverlaySet;
class VideoOverlay;
class OverlayBoxScope;

namespace NintendoSwitch{
namespace PokemonHome{


class HomeApplicationDetector : public StaticScreenDetector{
public:
    HomeApplicationDetector(Color color);
    virtual ~HomeApplicationDetector();
    
    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;
    
    std::vector<ImageFloatBox> detect_all(const ImageViewRGB32& screen) const;
    
protected:
    Color m_color;
    ImageFloatBox m_box;
};



class HomeApplicationWatcher : public VisualInferenceCallback{
public:
    HomeApplicationWatcher(Color color);
    virtual ~HomeApplicationWatcher();
    
    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;
    
    
protected:
    HomeApplicationDetector m_detector;
    FixedLimitVector<OverlayBoxScope> m_hits;
};



class HomeTitleScreenDetector : public StaticScreenDetector{
public:
    HomeTitleScreenDetector(Color color);
    virtual ~HomeTitleScreenDetector();

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

    std::vector<ImageFloatBox> detect_all(const ImageViewRGB32& screen) const;

protected:
    Color m_color;
    ImageFloatBox m_box;
};



class HomeTitleScreenWatcher : public VisualInferenceCallback{
public:
    HomeTitleScreenWatcher(Color color);
    virtual ~HomeTitleScreenWatcher();

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;


protected:
    HomeTitleScreenDetector m_detector;
    FixedLimitVector<OverlayBoxScope> m_hits;
};



class HomeMainMenuDetector : public StaticScreenDetector{
public:
    HomeMainMenuDetector(Color color);
    virtual ~HomeMainMenuDetector();

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

    std::vector<ImageFloatBox> detect_all(const ImageViewRGB32& screen) const;

protected:
    Color m_color;
    ImageFloatBox m_box;
};



class HomeMainMenuWatcher : public VisualInferenceCallback{
public:
    HomeMainMenuWatcher(Color color);
    virtual ~HomeMainMenuWatcher();

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;


protected:
    HomeMainMenuDetector m_detector;
    FixedLimitVector<OverlayBoxScope> m_hits;
};



class HomeGameSelectDetector : public StaticScreenDetector{
public:
    HomeGameSelectDetector(Color color);
    virtual ~HomeGameSelectDetector();

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

    std::vector<ImageFloatBox> detect_all(const ImageViewRGB32& screen) const;

protected:
    Color m_color;
    ImageFloatBox m_box;
};



class HomeGameSelectWatcher : public VisualInferenceCallback{
public:
    HomeGameSelectWatcher(Color color);
    virtual ~HomeGameSelectWatcher();

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;


protected:
    HomeGameSelectDetector m_detector;
    FixedLimitVector<OverlayBoxScope> m_hits;
};


class HomeListViewDetector : public StaticScreenDetector{
public:
    HomeListViewDetector(Color color);
    virtual ~HomeListViewDetector();

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

    std::vector<ImageFloatBox> detect_all(const ImageViewRGB32& screen) const;

protected:
    Color m_color;
    ImageFloatBox m_box;
};



class HomeListViewWatcher : public VisualInferenceCallback{
public:
    HomeListViewWatcher(Color color);
    virtual ~HomeListViewWatcher();

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;


protected:
    HomeListViewDetector m_detector;
    FixedLimitVector<OverlayBoxScope> m_hits;
};


class HomeSummaryViewDetector : public StaticScreenDetector{
public:
    HomeSummaryViewDetector(Color color);
    virtual ~HomeSummaryViewDetector();

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

    std::vector<ImageFloatBox> detect_all(const ImageViewRGB32& screen) const;

protected:
    Color m_color;
    ImageFloatBox m_box;
};



class HomeSummaryViewWatcher : public VisualInferenceCallback{
public:
    HomeSummaryViewWatcher(Color color);
    virtual ~HomeSummaryViewWatcher();

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;


protected:
    HomeSummaryViewDetector m_detector;
    FixedLimitVector<OverlayBoxScope> m_hits;
};


class HomeMarkingsViewDetector : public StaticScreenDetector{
public:
    HomeMarkingsViewDetector(Color color);
    virtual ~HomeMarkingsViewDetector();

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

    std::vector<ImageFloatBox> detect_all(const ImageViewRGB32& screen) const;

protected:
    Color m_color;
    ImageFloatBox m_box;
};



class HomeMarkingsViewWatcher : public VisualInferenceCallback{
public:
    HomeMarkingsViewWatcher(Color color);
    virtual ~HomeMarkingsViewWatcher();

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;


protected:
    HomeMarkingsViewDetector m_detector;
    FixedLimitVector<OverlayBoxScope> m_hits;
};


class HomeBoxViewDetector : public StaticScreenDetector{
public:
    HomeBoxViewDetector(Color color);
    virtual ~HomeBoxViewDetector();

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

    std::vector<ImageFloatBox> detect_all(const ImageViewRGB32& screen) const;

protected:
    Color m_color;
    ImageFloatBox m_box;
};



class HomeBoxViewWatcher : public VisualInferenceCallback{
public:
    HomeBoxViewWatcher(Color color);
    virtual ~HomeBoxViewWatcher();

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;


protected:
    HomeBoxViewDetector m_detector;
    FixedLimitVector<OverlayBoxScope> m_hits;
};


}
}
}
#endif
