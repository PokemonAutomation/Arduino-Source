/*  TeraTypeDetector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "PokemonSV_TeraTypeDetector.h"

#include <sstream>

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageMatch/ImageCropper.h"
#include "CommonFramework/ImageMatch/SilhouetteDictionaryMatcher.h"
#include "CommonFramework/ImageTools/ImageFilter.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

const std::array<std::string, NUM_TERA_TYPE> TERA_TYPE_NAMES = {
	"Bug",
	"Dark",
	"Dragon",
	"Electric",
	"Fairy",
	"Fighting",
	"Fire",
	"Flying",
	"Ghost",
	"Grass",
	"Ground",
	"Ice",
	"Normal",
	"Poison",
	"Psychic",
	"Rock",
	"Steel",
	"Water",
};

TeraType get_type(const std::string& name) {
	const auto it = std::find(begin(TERA_TYPE_NAMES), end(TERA_TYPE_NAMES), name);
	if (it == end(TERA_TYPE_NAMES)) {
		throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Unknown tera type name: " + name);
	}

	return TeraType(std::distance(TERA_TYPE_NAMES.begin(), it));
}

std::array<ImageRGB32, NUM_TERA_TYPE> BUILD_TERA_TYPE_ICONS() {
	const std::string image_folder_path = RESOURCE_PATH() + "PokemonSV/TeraTypes/";

	std::array<ImageRGB32, NUM_TERA_TYPE> ret;
	for (size_t i = 0; i < NUM_TERA_TYPE; i++) {
		const auto& type_name = TERA_TYPE_NAMES[i];
		const std::string image_path = image_folder_path + type_name + ".jpg";

		// Trim the image to remove 0-alpha boundaries.
		ImageRGB32 image(ImageMatch::trim_image_alpha(ImageRGB32(image_path)).copy());
		ret[i] = std::move(image);
	}

	return ret;
}
const std::array<ImageRGB32, NUM_TERA_TYPE>& ALL_TERA_TYPE_ICONS() {
	const static auto icons = BUILD_TERA_TYPE_ICONS();
	return icons;
}

ImageMatch::SilhouetteDictionaryMatcher BUILD_TERA_TYPE_ICON_MATCHER() {
	ImageMatch::SilhouetteDictionaryMatcher matcher;
	for (size_t i = 0; i < NUM_TERA_TYPE; i++) {
		matcher.add(TERA_TYPE_NAMES[i], ALL_TERA_TYPE_ICONS()[i]);
	}
	return matcher;
}
const ImageMatch::SilhouetteDictionaryMatcher& TERA_TYPE_ICON_MATCHER() {
    const static auto matcher = BUILD_TERA_TYPE_ICON_MATCHER();
    return matcher;
}

ImageRGB32 process_image(const ImageViewRGB32& image) {
	const ImageFloatBox box{0.787, 0.139, 0.073, 0.136};
    
    // Get a loose crop of the type icon
    ImageViewRGB32 cropped_image = extract_box_reference(image, box);

    // Get a tight crop:
    const ImagePixelBox tight_box = ImageMatch::enclosing_rectangle_with_pixel_filter(
        cropped_image,
        // The filter is a lambda function that returns true on black type icon pixels.
        [](Color pixel){
            return (uint32_t)pixel.red() + pixel.green() + pixel.blue() < 200;
        }
    );
    cropped_image = extract_box_reference(cropped_image, tight_box);

    // Replace the background orange or violet (outside the range from 0xff000000 to 0xff5f5f5f)
    // with 0-alpha white pixels.
    const bool replace_range = false;
    ImageRGB32 icon_image = filter_rgb32_range(
        cropped_image,
		0xff000000, 0xff5f5f5f, Color(0x00ffffff), replace_range
    );
	return icon_image;
}

TeraType detect_tera_type(Logger& logger, const ImageViewRGB32& screen) {
	ImageRGB32 icon_image = process_image(screen);

    const auto match_result = TERA_TYPE_ICON_MATCHER().match(icon_image, 100);
    std::ostringstream os;
    os << "Tera type icon match result: ";
    for(auto result : match_result.results){
        os << result.second << "(" << result.first << ") ";
    }
    logger.log(os.str(), COLOR_BLUE);

    return get_type(match_result.results.begin()->second);
}



// Take a screen of the tera card and name it Grass_in.jpg (for example)
// This will create an usable image for type template matching
void CREATE_ICONS_FROM_SCREEN() {
	const std::string image_folder_path = RESOURCE_PATH() + "PokemonSV/TeraTypes/";

	for (const auto& type_name : TERA_TYPE_NAMES) {
		const std::string image_path = image_folder_path + type_name + "_in.jpg";
		ImageRGB32 image(image_path);
		ImageRGB32 icon_image = process_image(image);
		icon_image.save(image_folder_path + type_name + ".jpg");
	}
}



}
}
}
