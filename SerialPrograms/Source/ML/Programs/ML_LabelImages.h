/*  ML Label Images
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ML_LabelImages_H
#define PokemonAutomation_ML_LabelImages_H

#include "Common/Cpp/Options/BatchOption.h"
#include "Common/Cpp/Options/FloatingPointOption.h"
#include "CommonFramework/Panels/PanelInstance.h"
#include "CommonFramework/Panels/UI/PanelWidget.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "Pokemon/Options/Pokemon_HomeSpriteSelectOption.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SwitchSystemOption.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SwitchSystemSession.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include <QGraphicsScene>
#include "ML/DataLabeling/SegmentAnythingModel.h"

class QGraphicsView;
class QGraphicsPixmapItem;

namespace PokemonAutomation{


class ConfigWidget;
namespace NintendoSwitch{
    class SwitchSystemWidget;
}
struct OverlayImage;


namespace ML{


class LabelImages_Descriptor : public PanelDescriptor{
public:
    LabelImages_Descriptor();
};

// label image program
class LabelImages : public PanelInstance{
public:
    LabelImages(const LabelImages_Descriptor& descriptor);
    virtual QWidget* make_widget(QWidget& parent, PanelHolder& holder) override;

public:
    //  Serialization
    virtual void from_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

private:
    friend class LabelImages_Widget;
    friend class DrawnBoundingBox;
    // switch control options like what micro-controller 
    // and what video source to use
    NintendoSwitch::SwitchSystemOption m_switch_control_option;
    // the group option that holds rest of the options defined below:
    BatchOption m_options;

    FloatingPointOption X;
    FloatingPointOption Y;
    FloatingPointOption WIDTH;
    FloatingPointOption HEIGHT;
    Pokemon::HomeSpriteSelectCell FORM_LABEL;

    size_t source_image_height = 0;
    size_t source_image_width = 0;
    std::vector<float> m_image_embedding;
    std::vector<bool> m_output_boolean_mask;

    ImageRGB32 m_mask_image;
    std::unique_ptr<OverlayImage> m_overlay_image;
    SAMSession m_sam_session;
};


class DrawnBoundingBox : public ConfigOption::Listener, public VideoOverlay::MouseListener{
public:
    ~DrawnBoundingBox();
    DrawnBoundingBox(LabelImages& parent, VideoOverlay& overlay);
    virtual void on_config_value_changed(void* object) override;
    virtual void on_mouse_press(double x, double y) override;
    virtual void on_mouse_release(double x, double y) override;
    virtual void on_mouse_move(double x, double y) override;

private:
    void detach();

private:
    LabelImages& m_program;
    VideoOverlay& m_overlay;
    VideoOverlaySet m_overlay_set;
    std::mutex m_lock;

    std::optional<std::pair<double, double>> m_mouse_start;
};


class LabelImages_Widget : public PanelWidget{
public:
    ~LabelImages_Widget();
    LabelImages_Widget(
        QWidget& parent,
        LabelImages& instance,
        PanelHolder& holder
    );

private:
    LabelImages& m_program;
    NintendoSwitch::SwitchSystemSession m_session;
    NintendoSwitch::SwitchSystemWidget* m_switch_widget;
    DrawnBoundingBox m_drawn_box;
    ConfigWidget* m_option_widget;

    std::unique_ptr<ImageRGB32> m_image_mask;
    std::unique_ptr<OverlayImage> m_overlay_image;
};




}
}
#endif

