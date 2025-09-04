/*  ML Image Annotation Display Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  This class represents the serializable state of an image display and label UI.
 * 
 *  This class is not thread-safe. To access internal state of an image display session
 *  in a thread-safe way, use ImageAnnotationDisplaySession.
 *
 */

#ifndef POKEMON_AUTOMATION_ML_ImageAnnotationDisplayOption_H
#define POKEMON_AUTOMATION_ML_ImageAnnotationDisplayOption_H

//#include "Common/Cpp/Color.h"
#include "CommonFramework/AudioPipeline/AudioOption.h"
//#include "CommonFramework/VideoPipeline/VideoSourceDescriptor.h"
#include "CommonFramework/VideoPipeline/VideoOverlayOption.h"
#include "Controllers/ControllerDescriptor.h"
//#include "NintendoSwitch/Options/NintendoSwitch_ModelType.h"

namespace PokemonAutomation{
    class ControllerRequirements;
namespace ML{


// options to load an image for image labeling purposes
class ImageAnnotationDisplayOption{
public:
    ImageAnnotationDisplayOption();
    ImageAnnotationDisplayOption(const JsonValue& json);

    void load_json(const JsonValue& json);
    JsonValue to_json() const;

    
public:
    std::string m_image_path;
    VideoOverlayOption m_overlay;
};





}
}
#endif
