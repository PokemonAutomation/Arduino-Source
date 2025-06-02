/*  Program Tabs
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ProgramTabs_H
#define PokemonAutomation_ProgramTabs_H

#include <QGroupBox>
#include "CommonFramework/Panels/PanelList.h"

class QComboBox;

namespace PokemonAutomation{


// The program selection UI on the left side of the program window.
// It has a dropdown menu to select which Switch game's program list to show, and
// a display list window to show the current active game's program list.
// This class owns the a vector of PanelListDescriptor. A PanelListDescriptor is
// like a generator for the programs and their UIs for a game.
class ProgramSelect : public QGroupBox{
public:
    ProgramSelect(QWidget& parent, PanelHolder& holder);

    // Load the panel specified in the persistent setting.
    void load_persistent_panel();

    virtual QSize sizeHint() const override;

private:
    void add(std::unique_ptr<PanelListDescriptor> list);
    void change_list(int index);

private:
    PanelHolder& m_holder;

    std::vector<std::unique_ptr<PanelListDescriptor>> m_lists;
    std::map<std::string, int> m_tab_map;

    QComboBox* m_dropdown;

    int m_active_index = -1;
    PanelListWidget* m_active_list = nullptr;
};



}
#endif
