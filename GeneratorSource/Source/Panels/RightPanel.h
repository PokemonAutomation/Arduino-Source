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
    RightPanel(QString category, QString name, QString doc_link)
        : m_category(std::move(category))
        , m_name(std::move(name))
        , m_doc_link(std::move(doc_link))
    {}
    virtual ~RightPanel() = default;

    const QString& category() const{ return m_category; }
    const QString& name() const{ return m_name; }

    virtual QWidget* make_ui(MainWindow& parent) = 0;

protected:
    QString m_category;
    QString m_name;
    QString m_doc_link;
};


}
#endif
