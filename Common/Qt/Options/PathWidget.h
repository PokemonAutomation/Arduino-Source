/*  Path Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Options_PathWidget_H
#define PokemonAutomation_Options_PathWidget_H

#include <QLineEdit>
#include <QPushButton>
#include "Common/Cpp/Options/PathOption.h"
#include "ConfigWidget.h"

namespace PokemonAutomation{



class PathCellWidget : public QWidget, public ConfigWidget{
public:
    ~PathCellWidget();
    PathCellWidget(QWidget& parent, PathCell& value);

    virtual void update_value() override;
    virtual void on_config_value_changed(void* object) override;
    virtual void on_config_visibility_changed() override;

private:
    void browse_file();

    PathCell& m_value;
    QLineEdit* m_line_edit;
    QPushButton* m_browse_button;
};



class PathOptionWidget : public QWidget, public ConfigWidget{
public:
    ~PathOptionWidget();
    PathOptionWidget(QWidget& parent, PathOption& value);

    virtual void update_value() override;
    virtual void on_config_value_changed(void* object) override;
    virtual void on_config_visibility_changed() override;

private:
    void browse_file();

    PathOption& m_value;
    QLineEdit* m_line_edit;
    QPushButton* m_browse_button;
};





}
#endif

