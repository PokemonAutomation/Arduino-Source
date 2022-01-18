/*  Shiny Sparkle Set
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinySparkleSet_H
#define PokemonAutomation_PokemonSwSh_ShinySparkleSet_H

#include <vector>
#include <QImage>
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
    virtual void read_from_image(const QImage& image) override;

    virtual void draw_boxes(
        VideoOverlaySet& overlays,
        const QImage& frame,
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
    const QImage& best_image() const{ return m_best_image; }

    void add_frame(const QImage& image, const ShinySparkleSetSwSh& sparkles);

private:
    double m_best_overall = 0;
    double m_best_star = 0;
    double m_best_square = 0;
    double m_best_type = 0;
    QImage m_best_image;
};




}
}
}
#endif
