/*  Shiny Sparkle Set
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_ShinySparkleSet_H
#define PokemonAutomation_PokemonBDSP_ShinySparkleSet_H

#include <QImage>
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
    virtual void read_from_image(const ImageViewRGB32& image) override;

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
    const QImage& best_image() const{ return m_best_image; }

    void add_frame(const QImage& image, const ShinySparkleSetBDSP& sparkles);

private:
    double m_best_overall = 0;
    QImage m_best_image;
};




}
}
}
#endif
