/*  Options Panel Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_OptionsPanelWidget_H
#define PokemonAutomation_OptionsPanelWidget_H

#include <QWidget>
#include <QVBoxLayout>
#include "Common/Qt/UiStateQtWidget.h"
#include "CommonFramework/Panels/OptionsPanel.h"
#include "CommonFramework/Panels/OptionsPanelSession.h"

class QPushButton;

namespace PokemonAutomation{

class BatchWidget;


class OptionsPanelWidget final : public QWidget, public UiComponentQtWidget{
public:
    using ParentState = OptionsPanelSession;

public:
    OptionsPanelWidget(QWidget& parent, OptionsPanelSession& session);

    virtual QWidget& widget() override{
        return *this;
    }

private:
    OptionsPanelSession& m_session;
    QVBoxLayout* m_layout;
    ConfigWidget* m_options;
};



}
#endif
