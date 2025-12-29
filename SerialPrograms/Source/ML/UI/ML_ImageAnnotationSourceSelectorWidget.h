/*  ML Image Annotation Source Selector Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ML_ImageAnnotationSourceSelectorWidget_H
#define PokemonAutomation_ML_ImageAnnotationSourceSelectorWidget_H

#include <vector>
#include <QWidget>
#include "CommonFramework/VideoPipeline/VideoSession.h"

class QComboBox;
class QPushButton;

namespace PokemonAutomation{
namespace ML{

class ImageAnnotationDisplaySession;

//  UI to select what image file to use for image annotation
//  This class inherits VideoSession::StateListener so it has callbacks for state change
//  in the video session.
//  It has following UI elements:
//  - Text cell "Image File Path": shows the current loaded image's file path
//  - Button "Open Image": load a new image from disk
//  - Button "Reload Image": reload current image in case there is some external change to the image.
//  - Text cell "x/xx in folder": shows how many images in the folder where the current image is in
//    and what in the order is this image in the folder, e.g. "3/40 in folder" means this is the 3rd
//    image in the folder.
//    - Images in subfolders inside the current folder are not considered.
//    - The images are ordered by what QDirIterator returns. It's order may not be what you display
//      on OS.
//  - Button "Prev Image in Folder": click to load the previous image: "3/40 in folder" -> "2/40"
//    (wraps to last image if at the beginning)
//  - Button "Next Image in Folder": click to load the next image: "3/40 in folder" -> "4/40"
//    (wraps to first image if at the end)
// - The two above buttons also have associated hotkeys that are implemented in
//   ML_ImageAnnotationDisplayWidget.cpp
class ImageAnnotationSourceSelectorWidget : public QWidget, public VideoSession::StateListener{
public:
    ~ImageAnnotationSourceSelectorWidget();
    ImageAnnotationSourceSelectorWidget(ImageAnnotationDisplaySession& session);

    // Get the folder path of the current loaded image
    std::string image_folder_path() const { return m_image_folder_path; }

    // Jump to an image by index offset from current position.
    // Positive offset moves forward, negative offset moves backward.
    // Wraps around: going before the first image wraps to the last image,
    // and going past the last image wraps to the first image.
    void go_to_image_by_index_offset(int offset);

private:
    //  Overwrites VideoSession::StateListener::post_startup().
    virtual void post_startup(VideoSource* source) override;


private:
    ImageAnnotationDisplaySession& m_session;

    QLabel* m_source_file_path_label;
    QLabel* m_folder_info_label;

    std::string m_image_folder_path;
    std::vector<std::string> m_image_paths_in_folder;
    std::size_t m_cur_image_file_idx_in_folder = 0;
};



}
}
#endif
