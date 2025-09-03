/*  ML Annotation IO
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Functions for IO of annotation related files
 */

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <QDirIterator>
#include <QDir>
#include <QMessageBox>

#include "Common/Cpp/Json/JsonTools.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/StringTools.h"
#include "Common/Cpp/PrettyPrint.h"
#include "ML_AnnotationIO.h"
#include "ML_SegmentAnythingModelConstants.h"
#include "ML_ObjectAnnotation.h"

namespace fs = std::filesystem;
using std::cout, std::endl;

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

void export_image_annotations_to_yolo_dataset(
    const std::string& image_folder_path,
    const std::string& annotation_folder_path,
    const std::string& yolo_dataset_path
){
    const bool recursive = true;
    const std::vector<std::string>& image_paths = find_images_in_folder(image_folder_path, recursive);
    if (image_paths.size() == 0){
        QMessageBox box;
        box.critical(nullptr, "Empty Image Folder",
            QString::fromStdString("No images found in " + image_folder_path + "."));
        return;
    }

    // TODO for simplicity we will parse this YAML file. In future we should use a proper YAML library
    std::ifstream fin(yolo_dataset_path.c_str());
    if (!fin){
        QMessageBox box;
        box.critical(nullptr, "Cannot Open YOLO Dataset Config File",
            QString::fromStdString("Cannot open " + yolo_dataset_path + "."));
        return;
    }

    std::vector<std::string> label_names;
    bool reading_labels = false;

    std::string line;
    int line_id = 0;
    while(std::getline(fin, line)){
        line_id++;
        // remove "#" comments
        size_t pound_idx = line.find_first_of("#");
        if (pound_idx != std::string::npos){
            line = line.substr(0, pound_idx);
        }
        line = StringTools::strip(line);
        if (line.size() == 0){
            continue;
        }

        // cout << "Line: " << line << endl;
        if (line.starts_with("names:")){
            reading_labels = true;
            line.clear();
            continue;
        }

        if (reading_labels){
            // cout << "start reading labels" << endl;
            size_t colon_idx = line.find_first_of(":");
            if (colon_idx == std::string::npos){
                QMessageBox box;
                box.critical(nullptr, "Error Parsing Dataset YAML",
                QString::fromStdString("YAML file " + yolo_dataset_path + " line " + std::to_string(line_id) + " contains no colon for labels."));
                return;
            }
            std::string number = StringTools::strip(line.substr(0, colon_idx));
            std::string label = StringTools::strip(line.substr(colon_idx+1));
            
            // cout << "found number " << number << " label " << label << endl;
            size_t num = StringTools::to_size_t(number);
            if (num != label_names.size()){
                QMessageBox box;
                box.critical(nullptr, "Error Parsing Dataset YAML",
                QString::fromStdString("YAML file " + yolo_dataset_path + " line " + std::to_string(line_id) + " has no label index."));
                return;
            }
            label_names.push_back(label);
        }

        line.clear();
    }

    std::map<std::string, size_t> label_indices;
    cout << "Load dataset labels: " << endl;
    for(size_t i = 0; i < label_names.size(); i++){
        cout << "- " << label_names[i] << endl;
        label_indices[label_names[i]] = i;
    }
    

    // convert images and annotations into new subfolders in the folder of the dataset config
    auto converted_folder_name = "exported-" + now_to_filestring();

    const auto yolo_dataset_config_file = fs::path(yolo_dataset_path);
    const fs::path yolo_dataset_folder = yolo_dataset_config_file.parent_path();
    const auto target_folder = yolo_dataset_folder / converted_folder_name;
    if (fs::exists(target_folder)){
        QMessageBox box;
        box.critical(nullptr, "Export Destination Folder Already Exists",
            QString::fromStdString("Folder " + target_folder.string() + " already exists."));
        return;
    }
    const auto target_image_folder = target_folder / "images";
    const auto target_label_folder = target_folder / "labels";
    cout << "Export to image folder: " << target_image_folder << endl;
    cout << "Export to label folder: " << target_label_folder << endl;

    fs::create_directories(target_image_folder);
    fs::create_directories(target_label_folder);

    fs::path anno_folder(annotation_folder_path);
    for(size_t i = 0; i < image_paths.size(); i++){
        const auto& image_path = image_paths[i];
        const auto image_file = fs::path(image_path);

        const std::string anno_filename = image_file.filename().replace_extension(".json").string();
        fs::path anno_file = anno_folder / anno_filename;
        if (!fs::exists(anno_file)){
            QMessageBox box;
            box.critical(nullptr, "Cannot Find Annotation File",
                QString::fromStdString("No annotation for " + image_path + "."));
            return;
        }

        const auto target_image_file = target_image_folder / image_file.filename();
        try{
            fs::copy_file(image_file, target_image_file);
        }catch (fs::filesystem_error&){
            QMessageBox box;
            box.critical(nullptr, "Cannot Copy File",
                QString::fromStdString(
                    "Cannot copy from " + image_file.string() + " to " + target_image_file.string() + 
                    ". Probably permission issue, source image is broken or target image path already exists due to image folder having same image filenames"
                ));
            return;
        }

        std::string json_content;
        const bool anno_loaded = file_to_string(anno_file.string(), json_content);
        if (!anno_loaded){
            QMessageBox box;
            box.warning(nullptr, "Unable to Load Annotation",
                QString::fromStdString("Cannot open annotation file " + anno_file.string() + ". Probably wrong permission?"));
            return;
        }

        const JsonValue loaded_json = parse_json(json_content);
        const JsonObject* json_obj = loaded_json.to_object();
        if (!json_obj){
            QMessageBox box;
            box.warning(nullptr, "Wrong JSON content",
                QString::fromStdString("Wong JSON content in annotation file " + anno_file.string() + 
                ". Probably older annotataion? Try loading and saving this annotation file."));
            return;
        }

        std::vector<std::string> label_file_lines;
        try{
            const int64_t image_width = json_obj->get_integer_throw("IMAGE_WIDTH");
            const int64_t image_height = json_obj->get_integer_throw("IMAGE_HEIGHT");
            const JsonArray& json_array = json_obj->get_array_throw("ANNOTATION");
            for(size_t j = 0; j < json_array.size(); j++){
                const ObjectAnnotation anno_obj = ObjectAnnotation::from_json((json_array)[j]);
                const std::string& label = anno_obj.label;

                auto it = label_indices.find(label);
                if (it == label_indices.end()){
                    if (label.ends_with("-male")){
                        it = label_indices.find(label.substr(0, label.size()-5));
                    } else if(label.ends_with("-female")){
                        it = label_indices.find(label.substr(0, label.size()-7));
                    }
                }
                if (it == label_indices.end()){
                    continue; // label not part of the YOLO dataset. Ignored.
                }

                const size_t label_id = it->second;

                 // TODO: once we implement the user checkbox on mask reliability, we should change this line
                const auto& box = anno_obj.mask_box;
                const double center_x = (box.min_x + box.max_x) / (2.0 * image_width);
                const double center_y = (box.min_y + box.max_y) / (2.0 * image_height);
                const double width = box.width() / (double)image_width;
                const double height = box.height() / (double)image_height;
                
                // each row in the YOLO dataclass label file is: class_index x_center y_center width height
                // https://docs.ultralytics.com/yolov5/tutorials/train_custom_data/#12-leverage-models-for-automated-labeling
                std::ostringstream os;
                os << label_id << " " << center_x << " " << center_y << " " << width << " " << height;
                label_file_lines.push_back(os.str());
            }
        } catch(JsonParseException& ){
            QMessageBox box;
            box.warning(nullptr, "Wrong JSON content",
                QString::fromStdString("Wong JSON content in annotation file " + anno_file.string() + "."));
            return;
        }

        const auto target_label_file = target_label_folder / image_file.filename().replace_extension(".txt");
        std::ofstream fout(target_label_file.string());
        for(const auto& file_line : label_file_lines){
            fout << file_line << "\n";
        }
    }
    cout << "Done exporting " << image_paths.size() << " annotations to YOLOv5 dataset" << endl;
}


}
}
