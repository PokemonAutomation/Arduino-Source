/*  Panel List
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PanelList_H
#define PokemonAutomation_PanelList_H

#include <memory>
#include <QListWidget>
#include "CommonFramework/Panels/PanelDescriptor.h"
#include "CommonFramework/Panels/PanelTools.h"

namespace PokemonAutomation{

class PanelListWidget;


class PanelListDescriptor{
    using MakePanelEntries = std::vector<PanelEntry> (*)();

public:
    PanelListDescriptor(
        std::string name,
        MakePanelEntries factory,
        bool enabled = true
    );

    const std::string& name() const{ return m_name; }
    bool enabled() const{ return m_enabled; }

    PanelListWidget* make_QWidget(QWidget& parent, PanelHolder& holder) const;

protected:
    std::string m_name;
    MakePanelEntries m_factory;
    bool m_enabled;
};



class PanelListWidget : public QListWidget{
public:
    static const std::string JSON_PROGRAM_PANEL;

public:
    PanelListWidget(
        QWidget& parent, PanelHolder& holder,
        std::vector<PanelEntry> list
    );

    size_t items() const{ return m_panel_map.size(); }

    void set_panel(const std::string& panel_name);

private:
    void handle_panel_clicked(const std::string& text);

private:
    PanelHolder& m_panel_holder;
    std::map<std::string, std::shared_ptr<const PanelDescriptor>> m_panel_map;
};




}
#endif
