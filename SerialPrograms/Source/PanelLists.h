/*  Program Tabs
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ProgramTabs_H
#define PokemonAutomation_ProgramTabs_H

#include <QTabWidget>
#include "CommonFramework/Panels/PanelList.h"

namespace PokemonAutomation{


class ProgramTabs : public QTabWidget{
public:
    ProgramTabs(QWidget& parent, PanelListener& listener);

    virtual QSize sizeHint() const override;

private:
    void add(PanelList* list);

private:
    std::vector<PanelList*> m_lists;
};




}
#endif
