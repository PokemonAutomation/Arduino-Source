/*  Camera Selector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This class handles the state of the camera selection.
 *
 */

#ifndef PokemonAutomation_VideoPipeline_CameraOption_H
#define PokemonAutomation_VideoPipeline_CameraOption_H

#include "Common/Cpp/ImageResolution.h"
#include "CameraInfo.h"

namespace PokemonAutomation{

class JsonValue;



//  Handles the state of video selection.
class CameraOption{
    static const std::string JSON_CAMERA;
    static const std::string JSON_RESOLUTION;

public:
    CameraOption(Resolution p_default_resolution);


public:
    void load_json(const JsonValue& json);
    JsonValue to_json() const;


public:
    const Resolution default_resolution;
    CameraInfo info;
    Resolution current_resolution;
};





}
#endif
