/*  Debug Dumper
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Filesystem/Filesystem.h"
#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "DebugDumper.h"

namespace PokemonAutomation{

std::string dump_debug_image(
    Logger& logger,
    const std::string& path,
    const std::string& label,
    const ImageViewRGB32& image
){
    std::string full_path = DEBUG_PATH() + path + "/" + now_to_filestring() + "-" + label + ".png";
    logger.log("Save debug image: " + full_path, COLOR_YELLOW);
    image.save(full_path);
    return full_path;
}


}
