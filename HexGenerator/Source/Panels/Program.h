/*  Parent Class for all Programs
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Program_H
#define PokemonAutomation_Program_H

#include <thread>
#include <QObject>
#include <QPushButton>
#include "UI/BoardList.h"
#include "RightPanel.h"

namespace PokemonAutomation{

class JsonArray;
class JsonObject;


class Program : public RightPanel{
public:
    static const std::string JSON_PROGRAM_NAME;
    static const std::string JSON_DOCUMENTATION;
    static const std::string JSON_DESCRIPTION;
    static const std::string JSON_PARAMETERS;

    static const std::string BUILD_BUTTON_NORMAL;
    static const std::string BUILD_BUTTON_BUSY;

public:
    Program(std::string category, std::string name, std::string description, std::string doc_link);
    Program(std::string category, const JsonObject& obj);
    ~Program();

    const std::string& description() const{ return m_description; }

    virtual QString check_validity() const{ return QString(); }
    virtual void restore_defaults(){}

    JsonObject to_json() const;
    std::string to_cfile() const;

    std::string save_json() const;
    std::string save_cfile() const;

    void save_and_build(const std::string& board);

    virtual QWidget* make_ui(MainWindow& parent) override;

protected:
    virtual QWidget* make_options_body(QWidget& parent);
    virtual JsonArray parameters_json() const;
    virtual std::string parameters_cpp() const{ return ""; }

private:
    std::string m_description;
    QPushButton* m_build_button;
    std::thread m_builder;
};




}
#endif
