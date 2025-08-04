/*  ML Label Images Overlay Manager
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Manages image overlay rendering for program LabelImages.
 */

#include "Pokemon/Resources/Pokemon_PokemonForms.h"
#include "ML_LabelImagesOverlayManager.h"
#include "ML_LabelImages.h"

#include <iostream>
using std::cout, std::endl;


namespace PokemonAutomation{
namespace ML{

inline size_t size_t_diff(size_t x, size_t y){
    return x >= y ? x - y : y - x;
}
inline size_t size_t_subtract_clamp(size_t x, size_t y){
    // max(x - y, 0)
    return x >= y ? x - y : 0;
}

LabelImages_OverlayManager::LabelImages_OverlayManager(LabelImages& program)
    : m_program(program)
    , m_overlay_set(program.m_display_session.overlay())
{
    m_inclusion_point_icon_template = ImageRGB32(21, 21);
    m_inclusion_point_icon_template.fill(0);
    const size_t strip_size = 5, strip_start = (21 - 5) / 2;
    for(size_t y = 0; y < strip_size; y++){
        for(size_t x = 0; x < m_inclusion_point_icon_template.width(); x++){
            m_inclusion_point_icon_template.pixel(x, y+strip_start) = uint32_t(COLOR_RED);
        }
    }
    for(size_t y = 0; y < m_inclusion_point_icon_template.height(); y++){
        for(size_t x = 0; x < strip_size; x++){
            m_inclusion_point_icon_template.pixel(x+strip_start, y) = uint32_t(COLOR_RED);
        }
    }

    m_exclusion_point_icon_template = ImageRGB32(21, 21);
    m_exclusion_point_icon_template.fill(0);
    const size_t center = m_exclusion_point_icon_template.height() / 2;
    const size_t d2_min_th = (center-2)*(center-2);
    const size_t d2_max_th = (center+2)*(center+2);
    for(size_t y = 0; y < m_exclusion_point_icon_template.height(); y++){
        for(size_t x = 0; x < m_exclusion_point_icon_template.width(); x++){
            const size_t dx = size_t_diff(center, x);
            const size_t dy = size_t_diff(center, y);
            const size_t d2 = dx*dx + dy*dy;
            if (d2 >= d2_min_th && d2 <= d2_max_th){
                // double offset = std::fabs(std::sqrt(d2) - (double)center); // 0-3
                // int color_magnitude = int(255.0 * (1.0 - std::max(0.0, offset-2)) + 0.5);
                // color_magnitude = std::max(std::min(color_magnitude, 255), 0);
                // uint32_t color = combine_argb(uint8_t(color_magnitude), 0, 0, uint8_t(color_magnitude));
                // cout << x << " " << y << " " << offset << " " << color_magnitude << endl;
                m_exclusion_point_icon_template.pixel(x, y) = combine_argb(255, 0, 0, 200); // dark blue
            }
        }
    }

    // m_inclusion_point_icon_template.save("./inclusion.png");
    // m_exclusion_point_icon_template.save("./exclusion.png");
}

void LabelImages_OverlayManager::clear(){
    m_overlay_set.clear();

    m_inclusion_point_icon = ImageRGB32();
    m_exclusion_point_icon = ImageRGB32();
}

void LabelImages_OverlayManager::set_image_size(){
    const size_t point_icon_size = std::max(size_t(9), std::min(m_program.source_image_height, m_program.source_image_width) / 100);

    m_inclusion_point_icon = m_inclusion_point_icon_template.scale_to(point_icon_size, point_icon_size);
    m_exclusion_point_icon = m_exclusion_point_icon_template.scale_to(point_icon_size, point_icon_size);
}


void LabelImages_OverlayManager::update_rendered_annotations(){
    m_overlay_set.clear();
    const size_t image_width = m_program.source_image_width;
    const size_t image_height = m_program.source_image_height;
    if (image_width <= 1 || image_height <= 1){
        return;
    }
    if (m_program.WIDTH > 0.0 && m_program.HEIGHT > 0.0){
        m_overlay_set.add(COLOR_RED, {m_program.X, m_program.Y, m_program.WIDTH, m_program.HEIGHT});
    }

    const auto& annotations = m_program.m_annotations;
    const size_t& m_selected = m_program.m_selected_obj_idx;

    auto create_overlay_for_index = [&](size_t i_obj){
        const auto& obj = annotations[i_obj];
        // overlayset.add(COLOR_RED, pixelbox_to_floatbox(source_image_width, source_image_height, obj.user_box));
        const auto mask_float_box = pixelbox_to_floatbox(image_width, image_height, obj.mask_box);
        std::string label = obj.label;
        const Pokemon::PokemonForm* form = Pokemon::get_pokemon_form(label);
        if (form != nullptr){
            label = form->display_name();
        }
        Color mask_box_color = (i_obj == m_selected) ? COLOR_BLACK : COLOR_BLUE;
        m_overlay_set.add(mask_box_color, mask_float_box, label);
        size_t mask_width = obj.mask_box.width();
        size_t mask_height = obj.mask_box.height();
        ImageRGB32 mask_image(mask_width, mask_height);
        // cout << "in render, mask_box " << obj.mask_box.min_x << " " << obj.mask_box.min_y << " " << obj.mask_box.max_x << " " << obj.mask_box.max_y << endl;

        for (size_t y = 0; y < mask_height; y++){
            for (size_t x = 0; x < mask_width; x++){
                const bool mask = obj.mask[y*mask_width + x];
                uint32_t& pixel = mask_image.pixel(x, y);
                // if the pixel's mask value is true, set a semi-transparent 45-degree blue strip color
                // otherwise: fully transparent (alpha = 0)
                uint32_t color = 0;
                if (mask){
                    color = (std::abs(int(x) - int(y)) % 4 <= 1) ? combine_argb(150, 30, 144, 255) : combine_argb(150, 0, 0, 60);
                }
                pixel = color;
            }
        }
        // cout << " count " << count << endl;
        m_overlay_set.add(std::move(mask_image), mask_float_box);

        if (i_obj == m_selected){
            const size_t icon_size = m_inclusion_point_icon.height();
            const size_t icon_min_offset = icon_size / 2;
            const size_t icon_max_offset = icon_size - icon_min_offset;
            auto create_box = [&](const std::pair<size_t, size_t>& p) -> ImageFloatBox{
                size_t x_min = size_t_subtract_clamp(p.first, icon_min_offset);
                size_t x_max = std::min(p.first + icon_max_offset, image_width-1);
                size_t y_min = size_t_subtract_clamp(p.second, icon_min_offset);
                size_t y_max = std::min(p.second + icon_max_offset, image_height-1);
                ImagePixelBox box(x_min, y_min, x_max, y_max);
                return pixelbox_to_floatbox(image_width, image_height, box);
            };
            // the inclusion and exclusion points are only rendered for the current selected object:
            for(const auto& p : obj.inclusion_points){
                m_overlay_set.add(m_inclusion_point_icon.copy(), create_box(p));
            }
            for(const auto& p : obj.exclusion_points){
                m_overlay_set.add(m_exclusion_point_icon.copy(), create_box(p));
            }
        }
    };
    for(size_t i_obj = 0; i_obj < annotations.size(); i_obj++){
        if (i_obj == m_selected){
            // skip current selected annotation because we want to render it last so that
            // it will not be occluded by other annotations
            continue;
        }
        create_overlay_for_index(i_obj);
    }
    if (m_selected < annotations.size()){
        create_overlay_for_index(m_selected);
    }
}


}
}