/*  Label Cell
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Qt/Options/ConfigWidget.h"
#include "LabelCellOption.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


struct LabelCellOption::Data{
//    mutable SpinLock m_lock;
    std::string m_text;
//    ImageRGB32 m_icon_owner;
    ImageViewRGB32 m_icon;
    Resolution m_resolution;

    Data(std::string text)
        : m_text(std::move(text))
    {}
    Data(std::string text, const ImageViewRGB32& icon)
        : m_text(std::move(text))
        , m_icon(icon)
    {}
    Data(std::string text, const ImageViewRGB32& icon, size_t icon_size)
        : m_text(std::move(text))
        , m_icon(icon)
    {
        size_t width = icon.width();
        size_t height = icon.height();
        if (width < height){
            width = width * icon_size / height;
            height = icon_size;
        }else{
            height = height * icon_size / width;
            width = icon_size;
        }
        m_resolution.width = width;
        m_resolution.height = height;
    }
//    Data(std::string text, ImageRGB32 icon)
//        : m_text(std::move(text))
//        , m_icon_owner(std::move(icon))
//        , m_icon(m_icon_owner)
//    {}
};


LabelCellOption::~LabelCellOption() = default;
LabelCellOption::LabelCellOption(
    LockMode lock_while_running,
    std::string text
)
    : ConfigOption(lock_while_running)
    , m_data(CONSTRUCT_TOKEN, std::move(text))
{}
LabelCellOption::LabelCellOption(
    LockMode lock_while_running,
    std::string text, const ImageViewRGB32& icon
)
    : ConfigOption(lock_while_running)
    , m_data(CONSTRUCT_TOKEN, std::move(text), icon)
{}
LabelCellOption::LabelCellOption(
    LockMode lock_while_running,
    std::string text, const ImageViewRGB32& icon, size_t icon_size
)
    : ConfigOption(lock_while_running)
    , m_data(CONSTRUCT_TOKEN, std::move(text), icon, icon_size)
{}
//LabelCellOption::LabelCellOption(std::string text, ImageRGB32 icon)
//    : m_data(CONSTRUCT_TOKEN, std::move(text), std::move(icon))
//{}
const std::string& LabelCellOption::text() const{
    return m_data->m_text;
}
const ImageViewRGB32& LabelCellOption::icon() const{
    return m_data->m_icon;
}
Resolution LabelCellOption::resolution() const{
    return m_data->m_resolution;
}
void LabelCellOption::load_json(const JsonValue&){
}
JsonValue LabelCellOption::to_json() const{
    return JsonValue();
}





}
