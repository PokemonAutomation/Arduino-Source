/*  Right-Side Panel Options
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_RightPanel_H
#define PokemonAutomation_RightPanel_H

#include <QWidget>

namespace PokemonAutomation{


class MainWindow;

class RightPanel : public QObject{
    Q_OBJECT

public:
    RightPanel(std::string category, std::string name, std::string doc_link)
        : m_category(std::move(category))
        , m_name(std::move(name))
        , m_doc_link(std::move(doc_link))
    {}
    virtual ~RightPanel() = default;

    const std::string& category() const{ return m_category; }
    const std::string& name() const{ return m_name; }

    virtual QWidget* make_ui(MainWindow& parent) = 0;

protected:
    std::string m_category;
    std::string m_name;
    std::string m_doc_link;
};


}
#endif
