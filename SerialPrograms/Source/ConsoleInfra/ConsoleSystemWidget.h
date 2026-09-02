/*  Console System (Qt Widget)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ConsoleInfra_ConsoleSystemWidget_H
#define PokemonAutomation_ConsoleInfra_ConsoleSystemWidget_H

#include <QVBoxLayout>
#include <QWidget>
#include "Common/Qt/UiStateQtWidget.h"
#include "Common/Qt/CollapsibleGroupBox.h"
#include "CommonFramework/AudioPipeline/UI/AudioSelectorWidget.h"
#include "CommonFramework/AudioPipeline/UI/AudioDisplayWidget.h"
#include "CommonFramework/VideoPipeline/UI/VideoSourceSelectorWidget.h"
#include "CommonFramework/VideoPipeline/UI/VideoDisplayWidget.h"
#include "ConsoleSystemSession.h"

namespace PokemonAutomation{
namespace ConsoleInfra{


class ConsoleSystemWidget : public QWidget, public UiComponentQtWidget{
public:
    using ParentState = ConsoleSystemSession;

public:
    virtual ~ConsoleSystemWidget();

    ConsoleSystemWidget(
        QWidget& parent,
        ConsoleSystemSession& session
    )
        : ConsoleSystemWidget(parent, session, true)
    {}


protected:
    ConsoleSystemWidget(
        QWidget& parent,
        ConsoleSystemSession& session,
        bool include_command_row
    );


private:
    virtual QWidget& widget() override{ return *this; }
    virtual void focusInEvent(QFocusEvent* event) override;
    virtual void focusOutEvent(QFocusEvent* event) override;
    virtual void keyPressEvent(QKeyEvent* event) override;
    virtual void keyReleaseEvent(QKeyEvent* event) override;


protected:
    ConsoleSystemSession& m_session;

    CollapsibleGroupBox* m_group_box;

    VideoDisplayWidget* m_video_display;
    AudioDisplayWidget* m_audio_display;

    QVBoxLayout* m_group_layout;
    VideoSourceSelectorWidget* m_video_selector;
    AudioSelectorWidget* m_audio_widget;
};



}
}
#endif
