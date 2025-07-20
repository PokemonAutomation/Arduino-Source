/*  ML Annotation IO
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Functions for IO of annotation related files
 */

#include <fstream>
#include <iostream>
#include <QDirIterator>
#include <QDir>

#include "ML_AnnotationIO.h"
#include "ML_SegmentAnythingModelConstants.h"

namespace PokemonAutomation{
namespace ML{

// save the image embedding as a file with path <image_filepath>.embedding
void save_image_embedding_to_disk(const std::string& image_filepath, const std::vector<float>& embedding){
    const std::string embedding_path = image_filepath + ".embedding";
    std::ofstream fout(embedding_path, std::ios::binary);
    // write embedding shape
    fout.write(reinterpret_cast<const char*>(&SAM_EMBEDDER_OUTPUT_N_CHANNELS), sizeof(SAM_EMBEDDER_OUTPUT_N_CHANNELS));
    fout.write(reinterpret_cast<const char*>(&SAM_EMBEDDER_OUTPUT_IMAGE_SIZE), sizeof(SAM_EMBEDDER_OUTPUT_IMAGE_SIZE));
    fout.write(reinterpret_cast<const char*>(&SAM_EMBEDDER_OUTPUT_IMAGE_SIZE), sizeof(SAM_EMBEDDER_OUTPUT_IMAGE_SIZE));
    fout.write(reinterpret_cast<const char*>(embedding.data()), sizeof(float) * embedding.size());
    fout.close();
    std::cout << "Saved image embedding as " << embedding_path << std::endl;
}


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


std::vector<std::string> find_images_in_folder(const std::string& folder_path, bool recursive){
    QDir image_dir(folder_path.c_str());
    if (!image_dir.exists()){
        std::cerr << "Error: input image folder path " << folder_path << " does not exist." << std::endl;
        return {};
    }

    auto flag = recursive ? QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags;
    QDirIterator image_file_iter(image_dir.absolutePath(), {"*.png", "*.jpg", "*.jpeg"}, QDir::Files, flag);
    std::vector<std::string> all_image_paths;
    while (image_file_iter.hasNext()){
        all_image_paths.emplace_back(image_file_iter.next().toStdString());
    }
    std::cout << "Found " << all_image_paths.size() << " images " << (recursive ? "recursively " : "") << 
        "in folder " << folder_path << std::endl;
    return all_image_paths;
}


}
}