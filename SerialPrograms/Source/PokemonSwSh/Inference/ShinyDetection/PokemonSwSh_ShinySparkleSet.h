/*  Shiny Sparkle Set
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinySparkleSet_H
#define PokemonAutomation_PokemonSwSh_ShinySparkleSet_H

#include <memory>
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "Pokemon/Pokemon_ShinySparkleSet.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



class ShinySparkleSetSwSh : public Pokemon::ShinySparkleSet{
public:
    std::vector<ImagePixelBox> balls;
    std::vector<ImagePixelBox> stars;
    std::vector<ImagePixelBox> squares;
    std::vector<ImagePixelBox> lines;

    virtual ~ShinySparkleSetSwSh();
    virtual void clear() override;

    double alpha_overall() const{ return m_alpha_overall; }
    double alpha_star() const{ return m_alpha_star; }
    double alpha_square() const{ return m_alpha_square; }

    virtual std::string to_str() const override;
    virtual void read_from_image(size_t screen_area, const ImageViewRGB32& image) override;

    virtual void draw_boxes(
        VideoOverlaySet& overlays,
        const ImageViewRGB32& frame,
        const ImageFloatBox& inference_box
    ) const override;

private:
    void update_alphas();

    double m_alpha_overall = 0;
    double m_alpha_star = 0;
    double m_alpha_square = 0;
};



class ShinySparkleAggregator{
public:
    double best_overall() const{ return m_best_overall; }
    double best_star() const{ return m_best_star; }
    double best_square() const{ return m_best_square; }
    const std::shared_ptr<const ImageRGB32>& best_image() const{ return m_best_image; }

    void add_frame(const std::shared_ptr<const ImageRGB32>& image, const ShinySparkleSetSwSh& sparkles);

private:
    double m_best_overall = 0;
    double m_best_star = 0;
    double m_best_square = 0;
    double m_best_type = 0;
    std::shared_ptr<const ImageRGB32> m_best_image;
};




}
}
}
#endif
