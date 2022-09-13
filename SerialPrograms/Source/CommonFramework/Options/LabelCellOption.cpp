/*  Label Cell
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Qt/Options/ConfigWidget.h"
#include "LabelCellOption.h"

namespace PokemonAutomation{


struct LabelCellOption::Data{
//    mutable SpinLock m_lock;
    std::string m_text;
    ImageViewRGB32 m_icon;

    Data(std::string text, const ImageViewRGB32& icon)
        : m_text(std::move(text))
        , m_icon(icon)
    {}
};


LabelCellOption::~LabelCellOption() = default;
LabelCellOption::LabelCellOption(std::string label, const ImageViewRGB32& icon)
    : m_data(CONSTRUCT_TOKEN, std::move(label), icon)
{}
const std::string& LabelCellOption::text() const{
    return m_data->m_text;
}
const ImageViewRGB32& LabelCellOption::icon() const{
    return m_data->m_icon;
}
void LabelCellOption::load_json(const JsonValue&){
}
JsonValue LabelCellOption::to_json() const{
    return JsonValue();
}





}
