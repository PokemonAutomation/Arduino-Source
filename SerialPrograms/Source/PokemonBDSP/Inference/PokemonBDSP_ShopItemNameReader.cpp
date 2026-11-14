/*  Shop Item Name Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonTools/OCR/OCR_Routines.h"
#include "PokemonBDSP_ShopItemNameReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


ShopItemNameReader::ShopItemNameReader()
    : SmallDictionaryMatcher([](){
        // Reuse shared item categories and Ball/Berry names, plus BDSP TMs.
        JsonObject dictionary;
        for (const char* resource : {
            "Pokemon/ItemNameDisplay.json",
            "Pokemon/Medicines.json",
            "Pokemon/BattleItems.json",
            "Pokemon/KeyItems.json",
            "Pokemon/Vitamins.json",
            "Pokemon/HeldItems.json",
            "Pokemon/OtherItems.json",
            "Pokemon/Treasures.json",
            "PokemonBDSP/BDSP_TM.json",
        }){
            const JsonValue names = load_json_file(RESOURCE_PATH() + resource);
            for (const auto& item : names.to_object_throw()){
                for (const auto& translation : item.second.to_object_throw()){
                    JsonValue& language = dictionary[translation.first];
                    if (language.is_null()){
                        language = JsonObject();
                    }
                    JsonValue& entry = language.to_object_throw()[item.first];
                    if (entry.is_null()){
                        entry = JsonArray();
                    }
                    JsonArray& candidates = entry.to_array_throw();
                    // TMs have both a numbered item label and a localized move name.
                    const JsonArray* aliases = translation.second.to_array();
                    const size_t count = aliases ? aliases->size() : 1;
                    for (size_t i = 0; i < count; i++){
                        const std::string& name = aliases ? (*aliases)[i].to_string_throw() :
                            translation.second.to_string_throw();
                        bool present = false;
                        for (const auto& candidate : candidates){
                            if (candidate.to_string_throw() == name){
                                present = true;
                                break;
                            }
                        }
                        if (!present){
                            candidates.push_back(name);
                        }
                    }
                }
            }
        }
        return dictionary;
    }())
{}

const ShopItemNameReader& ShopItemNameReader::instance(){
    static ShopItemNameReader reader;
    return reader;
}

OCR::StringMatchResult ShopItemNameReader::read_substring(
    Logger& logger,
    Language language,
    const ImageViewRGB32& image,
    double min_text_ratio, double max_text_ratio
) const{
    return match_substring_from_image_multifiltered(
        &logger, language, image,
        OCR::BLACK_TEXT_FILTERS(),
        MAX_LOG10P, MAX_LOG10P_SPREAD,
        min_text_ratio, max_text_ratio
    );
}


}
}
}
