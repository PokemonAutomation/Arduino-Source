/*  ML Segment Anything Embedding
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Creating and loading image embedding used by Segment Anything Model
 */

#ifndef PokemonAutomation_ML_SEGMENTANYTHINGEMBEDDING_H
#define PokemonAutomation_ML_SEGMENTANYTHINGEMBEDDING_H

#include <string>
#include <vector>

// load pre-computed image embedding from disk
// return true if there is the embedding file
bool load_image_embedding(const std::string& image_filepath, std::vector<float>& image_embedding);


#endif