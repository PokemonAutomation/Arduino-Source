/*  Debug Dumper
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QDir>
#include "DebugDumper.h"
#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/Logging/Logger.h"

namespace PokemonAutomation{

// create a folder with path ./DebugDumps/`folder_path`.
// The function will create all parent folders necessary to create the folder.
void create_debug_folder(const std::string& folder_path){
    QDir().mkdir(DEBUG_PATH().c_str());
    QDir(DEBUG_PATH().c_str()).mkpath(folder_path.c_str());
}

std::string dump_debug_image(
    Logger& logger,
    const std::string& path,
    const std::string& label,
    const ImageViewRGB32& image
){
    create_debug_folder(path);
    std::string full_path = DEBUG_PATH() + path + "/" + now_to_filestring() + "-" + label + ".png";
    logger.log("Saving debug image to: " + full_path, COLOR_YELLOW);
    image.save(full_path);
    return full_path;
}


}
