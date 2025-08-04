/*  ML Label Images Overlay Manager
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Manages image overlay rendering for program LabelImages.
 *  Since ML_LabelImages.cpp is quite long, moved the overlay rendering code
 *  to this separate file.
 */

#ifndef PokemonAutomation_ML_LabelImagesOverlayManager_H
#define PokemonAutomation_ML_LabelImagesOverlayManager_H


#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"


namespace PokemonAutomation{
namespace ML{

class LabelImages;


class LabelImages_OverlayManager{
public:
    LabelImages_OverlayManager(LabelImages& proram);

    // clear all data to prepare for a new image to label
    void clear();

    // Initialize internal data related to source image size.
    // This must be called after LabelImages::source_image_width and source_image_height has been
    // set to the new size.
    // This must be called after clear() and before update_rendered_annotations().
    void set_image_size();

    // called whenever a change in annotation data happens to update the corresponding
    // rendering data
    void update_rendered_annotations();

private:
    LabelImages& m_program;

    VideoOverlaySet m_overlay_set;

    ImageRGB32 m_inclusion_point_icon_template;
    ImageRGB32 m_exclusion_point_icon_template;

    ImageRGB32 m_inclusion_point_icon;
    ImageRGB32 m_exclusion_point_icon;
};


}
}


#endif