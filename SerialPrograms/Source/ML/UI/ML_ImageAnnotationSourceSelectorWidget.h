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
class ImageAnnotationSourceSelectorWidget : public QWidget, public VideoSession::StateListener{
public:
    ~ImageAnnotationSourceSelectorWidget();
    ImageAnnotationSourceSelectorWidget(ImageAnnotationDisplaySession& session);


private:
    //  Overwrites VideoSession::StateListener::post_startup().
    virtual void post_startup(VideoSource* source) override;


private:
    ImageAnnotationDisplaySession& m_session;

    QLabel* m_source_file_path_label;
};



}
}
#endif
