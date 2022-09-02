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
    ProgramTabs(QWidget& parent, PanelHolder& holder);

    // Load the panel specified in the persistent setting.
    void load_persistent_panel();

    virtual QSize sizeHint() const override;

private:
    void add(PanelList* list);

private:
    std::vector<PanelList*> m_lists;
    std::map<std::string, int> m_tab_map;
};




}
#endif
