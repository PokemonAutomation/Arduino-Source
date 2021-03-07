/*  Right-Side Panel Options
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_RightPanel_H
#define PokemonAutomation_RightPanel_H

#include <QVBoxLayout>
#include <QWidget>

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


class MainWindow;
class RightPanelUI;


class RightPanel{
public:
    virtual ~RightPanel() = default;

    RightPanel(QColor color, QString name, QString doc_link, QString description);

    virtual QJsonValue to_json() const;

    QColor color() const{ return m_color; }
    const QString& name() const{ return m_name; }
    const QString& description() const{ return m_description; }

    virtual QWidget* make_ui(MainWindow& window);

protected:
    friend class RightPanelUI;
    const QColor m_color;
    const QString m_name;
    const QString m_doc_link;
    const QString m_description;
};


class RightPanelUI : public QWidget{
    friend class RightPanel;

protected:
    RightPanelUI(RightPanel& factory);
    virtual void construct();
    virtual void append_description(QWidget& parent, QVBoxLayout& layout){}
    virtual void make_body(QWidget& parent, QVBoxLayout& layout){}

public:
    bool is_running() const{ return m_running; }

protected:
    RightPanel& m_factory;
    bool m_running = false;
};



}
#endif
