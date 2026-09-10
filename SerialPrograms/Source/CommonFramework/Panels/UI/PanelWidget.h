/*  Panel
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PanelWidget_H
#define PokemonAutomation_PanelWidget_H

#include <QWidget>
#include "Common/Qt/UiStateQtWidget.h"
#include "CommonFramework/Panels/PanelTools.h"
#include "CommonFramework/Panels/PanelSession.h"

namespace PokemonAutomation{

class CollapsibleGroupBox;

// A base class to define the UI widgets of a program panel.
// A PanelSession can call make_widget() to create it.
// Its derived classes can call make_header() to create a collabspile program header
// that shows program title, link to online documentation and others.
class PanelWidget : public QWidget, public UiComponentQtWidget{
public:
    using ParentState = PanelSession;

public:
    PanelWidget(QWidget& parent, PanelSession& session);
    virtual ~PanelWidget() = default;

//    PanelSession& session(){ return m_session; }

    virtual QWidget& widget() override{
        return *this;
    }

protected:
    //  Generate a collapsible UI element that shows the program panel header.
    //  It contains the name of the program and its description.
    virtual CollapsibleGroupBox* make_header();

protected:
    PanelSession& m_session;
};




}
#endif
