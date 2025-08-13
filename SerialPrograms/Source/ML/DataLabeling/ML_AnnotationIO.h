/*  ML Annotation IO
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Functions for IO of annotation related files
 */

#ifndef PokemonAutomation_ML_AnnotationIO_H
#define PokemonAutomation_ML_AnnotationIO_H

#include <vector>
#include <string>

namespace PokemonAutomation{
namespace ML{

// Load pre-computed image embedding from disk
// Return true if there is the embedding file.
// The embedding is stored in a file in the same folder as the image, having the same name but with a suffix ".embedding".
bool load_image_embedding(const std::string& image_filepath, std::vector<float>& image_embedding);

// Save the image embedding as a file with path <image_filepath>.embedding.
void save_image_embedding_to_disk(const std::string& image_filepath, const std::vector<float>& embedding);

// Find image paths stored in a folder. The search can be recursive into child folders or not.
std::vector<std::string> find_images_in_folder(const std::string& folder_path, bool recursive);

void export_image_annotations_to_yolo_dataset(
    const std::string& image_folder_path,
    const std::string& annotation_folder_path,
    const std::string& yolo_dataset_path
);


}
}

#endif