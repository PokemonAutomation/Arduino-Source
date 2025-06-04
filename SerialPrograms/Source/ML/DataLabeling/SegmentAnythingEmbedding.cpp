/*  ML Segment Anything Embedding
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Creating and loading image embedding used by Segment Anything Model
 */

#include <fstream>
#include <iostream>
#include "SegmentAnythingEmbedding.h"


bool load_image_embedding(const std::string& image_filepath, std::vector<float>& image_embedding){
    std::string emebdding_path = image_filepath + ".embedding";
    std::ifstream fin(emebdding_path, std::ios::binary);
    if (!fin.is_open()){
        std::cout << "No embedding for image " << image_filepath << std::endl;
        return false;
    }

    int embedding_n_channels = 0, embedding_height = 0, emebedding_width = 0;
    fin.read(reinterpret_cast<char*>(&embedding_n_channels), sizeof(int));
    fin.read(reinterpret_cast<char*>(&embedding_height), sizeof(int));
    fin.read(reinterpret_cast<char*>(&emebedding_width), sizeof(int));

    std::cout << "Image embedding shape [" << embedding_n_channels << ", " << embedding_height
              << ", " << emebedding_width << "]" << std::endl;
    if (embedding_n_channels <= 0 || embedding_height <= 0 || emebedding_width <= 0){
        std::string err_msg = "Image embedding wrong dimension from " + emebdding_path;
        std::cerr << err_msg << std::endl;
        throw std::runtime_error(err_msg);
    }

    const int size = embedding_n_channels * embedding_height * emebedding_width;
    image_embedding.resize(size);
    fin.read(reinterpret_cast<char*>(image_embedding.data()), sizeof(float) * size);
    std::cout << "Loaded image embedding from " << emebdding_path << std::endl;
    return true;
}