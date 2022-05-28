/*  Selected Region Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QImage>

#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageFilter.h"
#include "PokemonLA_PokemonMapSpriteReader.h"

#include <map>
#include <vector>
#include <string>
#include <iostream>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

using FeatureVector = std::vector<float>;

std::map<std::string, FeatureVector> compute_MMO_sprite_features(){
    std::map<std::string, FeatureVector> features;
    
    
    return features;
}

const std::map<std::string, FeatureVector>& MMO_SPRITE_FEATUES(){
    const static std::map<std::string, FeatureVector> features = compute_MMO_sprite_features();
    return features;
}



std::string match_pokemon_map_sprite(const QImage& image){


    return "";
}



}
}
}
