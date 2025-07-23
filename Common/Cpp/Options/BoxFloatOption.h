/*  Box Float Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Options_BoxFloatOption_H
#define PokemonAutomation_Options_BoxFloatOption_H

#include "Common/Cpp/Containers/Pimpl.h"
#include "Common/Cpp/Options/ConfigOption.h"

namespace PokemonAutomation{


class BoxFloatOption : public ConfigOption{
public:
    ~BoxFloatOption();
    BoxFloatOption(
        std::string label,
        LockMode lock_while_running,
        double default_x,
        double default_y,
        double default_width,
        double default_height
    );

    const std::string& label() const;
    double x() const;
    double y() const;
    double width() const;
    double height() const;

    //  Returns true if a value was changed.
    bool sanitize(double& x, double& y, double& width, double& height) const;

    void get_all(double& x, double& y, double& width, double& height) const;
    void set_all(double x, double y, double width, double height);

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual std::string check_validity() const override;
    virtual void restore_defaults() override;

    virtual ConfigWidget* make_QtWidget(QWidget& parent) override;


private:
    struct Data;
    Pimpl<Data> m_data;
};



}
#endif
