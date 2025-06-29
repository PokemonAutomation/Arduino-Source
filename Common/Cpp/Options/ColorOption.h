/*  Color Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ColorOption_H
#define PokemonAutomation_ColorOption_H

#include <atomic>
#include "Common/Cpp/Options/ConfigOption.h"

namespace PokemonAutomation{



class ColorCell : public ConfigOption{
public:
    ColorCell(const ColorCell& x);
    ColorCell(
        LockMode lock_while_running,
        bool has_alpha,
        uint32_t default_value, uint32_t current_value
    );

    uint32_t default_value() const;
    std::string to_str() const;
    static std::string color_to_str(uint32_t color, bool has_alpha);

    operator uint32_t() const;
    void set(uint32_t x);
    void set(const std::string& str);

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;
    virtual void restore_defaults() override;

    virtual ConfigWidget* make_QtWidget(QWidget& parent) override;


private:
    bool m_has_alpha;
    uint32_t m_default_value;
    std::atomic<uint32_t> m_current_value;
};



}
#endif
