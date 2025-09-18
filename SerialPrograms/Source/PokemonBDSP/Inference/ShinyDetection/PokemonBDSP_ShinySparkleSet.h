/*  Shiny Sparkle Set
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_ShinySparkleSet_H
#define PokemonAutomation_PokemonBDSP_ShinySparkleSet_H

#include <memory>
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "Pokemon/Pokemon_ShinySparkleSet.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{



class ShinySparkleSetBDSP : public Pokemon::ShinySparkleSet{
public:
    std::vector<ImagePixelBox> balls;
    std::vector<ImagePixelBox> stars;

    virtual ~ShinySparkleSetBDSP();
    virtual void clear() override;

    double alpha_overall() const{ return m_alpha_overall; }

    ShinySparkleSetBDSP extract_subbox(const ImagePixelBox& subbox) const;

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
};



class ShinySparkleAggregator{
public:
    double best_overall() const{ return m_best_overall; }
    const std::shared_ptr<const ImageRGB32>& best_image() const{ return m_best_image; }

    void add_frame(const std::shared_ptr<const ImageRGB32>& image, const ShinySparkleSetBDSP& sparkles);

private:
    double m_best_overall = 0;
    std::shared_ptr<const ImageRGB32> m_best_image;
};




}
}
}
#endif
