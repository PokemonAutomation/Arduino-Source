/*  Box Float Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "BoxFloatOption.h"

namespace PokemonAutomation{



struct BoxFloatOption::Data{
    std::string m_label;
    double m_default_x;
    double m_default_y;
    double m_default_width;
    double m_default_height;

    double m_x;
    double m_y;
    double m_width;
    double m_height;

    mutable SpinLock m_lock;
};



BoxFloatOption::~BoxFloatOption() = default;
BoxFloatOption::BoxFloatOption(
    std::string label,
    LockMode lock_while_running,
    double default_x,
    double default_y,
    double default_width,
    double default_height
)
    : ConfigOption(lock_while_running)
    , m_data(CONSTRUCT_TOKEN)
{
    Data& self = *m_data;

    self.m_label = std::move(label);

    set_all(default_x, default_y, default_width, default_height);

    self.m_default_x = self.m_x;
    self.m_default_y = self.m_y;
    self.m_default_width = self.m_width;
    self.m_default_height = self.m_height;
}
const std::string& BoxFloatOption::label() const{
    return m_data->m_label;
}
double BoxFloatOption::x() const{
    return m_data->m_x;
}
double BoxFloatOption::y() const{
    return m_data->m_y;
}
double BoxFloatOption::width() const{
    return m_data->m_width;
}
double BoxFloatOption::height() const{
    return m_data->m_height;
}


bool BoxFloatOption::sanitize(double& x, double& y, double& width, double& height) const{
    bool changed = false;

    if (x < 0.0){
        x = 0.0;
        changed = true;
    }
    if (x > 1.0){
        x = 1.0;
        changed = true;
    }

    if (y < 0.0){
        y = 0.0;
        changed = true;
    }
    if (y > 1.0){
        y = 1.0;
        changed = true;
    }

    if (width < 0.0){
        width = 0.0;
        changed = true;
    }
    if (width > 1.0){
        width = 1.0;
        changed = true;
    }

    if (height < 0.0){
        height = 0.0;
        changed = true;
    }
    if (height > 1.0){
        height = 1.0;
        changed = true;
    }

    if (x + width > 1.00000000001){
        width = 1.0 - x;
        changed = true;
    }
    if (y + height > 1.00000000001){
        height = 1.0 - y;
        changed = true;
    }

    return changed;
}
void BoxFloatOption::get_all(double& x, double& y, double& width, double& height) const{
    const Data& self = *m_data;

    ReadSpinLock lg(self.m_lock);
    x = self.m_x;
    y = self.m_y;
    width = self.m_width;
    height = self.m_height;
}
void BoxFloatOption::set_all(double x, double y, double width, double height){
    sanitize(x, y, width, height);

    Data& self = *m_data;
    bool changed = false;

    {
        WriteSpinLock lg(self.m_lock);

        changed |= self.m_x != x;
        changed |= self.m_y != y;
        changed |= self.m_width != width;
        changed |= self.m_height != height;

        self.m_x = x;
        self.m_y = y;
        self.m_width = width;
        self.m_height = height;
    }

    if (changed){
        report_value_changed(this);
    }
}


void BoxFloatOption::load_json(const JsonValue& json){
    const JsonObject* obj = json.to_object();
    if (obj == nullptr){
        return;
    }

    double x, y, width, height;
    get_all(x, y, width, height);

    obj->read_float(x, "x");
    obj->read_float(y, "y");
    obj->read_float(width, "width");
    obj->read_float(height, "height");

    set_all(x, y, width, height);
}
JsonValue BoxFloatOption::to_json() const{
    double x, y, width, height;
    get_all(x, y, width, height);

    JsonObject ret;
    ret["x"] = std::to_string(x);
    ret["y"] = std::to_string(y);
    ret["width"] = std::to_string(width);
    ret["height"] = std::to_string(height);
    return ret;
}

std::string BoxFloatOption::check_validity() const{
    double x, y, width, height;
    get_all(x, y, width, height);

    if (x < 0 || x > 1.0){
        return "x must be between 0.0 and 1.0.";
    }
    if (y < 0 || y > 1.0){
        return "y must be between 0.0 and 1.0.";
    }
    if (width < 0 || width > 1.0){
        return "width must be between 0.0 and 1.0.";
    }
    if (height < 0 || height > 1.0){
        return "height must be between 0.0 and 1.0.";
    }
    if (x + width > 1.00000000001){
        return "x + width must be <= 1.0.";
    }
    if (y + width > 00000000001){
        return "y + width must be <= 1.0.";
    }

    return std::string();
}
void BoxFloatOption::restore_defaults(){
    Data& self = *m_data;
    set_all(
        self.m_default_x,
        self.m_default_y,
        self.m_default_width,
        self.m_default_height
    );
}




}

