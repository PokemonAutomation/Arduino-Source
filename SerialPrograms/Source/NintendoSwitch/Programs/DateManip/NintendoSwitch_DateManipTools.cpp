/*  Date Manipulation Tools
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonTools/Images/ImageFilter.h"
//#include "CommonTools/OCR/OCR_RawOCR.h"
#include "CommonTools/OCR/OCR_NumberReader.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch_DateManipTools.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace DateReaderTools{



ImageRGB32 filter_image(const ImageViewRGB32& image){
    double brightness = image_stats(image).average.sum();
    bool white_theme = brightness > 500;

    ImageRGB32 filtered = to_blackwhite_rgb32_range(
        image,
        white_theme,
        0xff000000, white_theme ? 0xffff7fff : 0xff7f7f7f
    );
    return filtered;
}
int read_box(
    Logger& logger,
    int min, int max,
    const ImageViewRGB32& screen, const ImageFloatBox& box
){
    ImageViewRGB32 cropped = extract_box_reference(screen, box);

//    static int c = 0;
//    cropped.save("image-" + std::to_string(c++) + ".png");

    double brightness = image_stats(cropped).average.sum();
    bool white_theme = brightness > 500;

    int value;
    if (white_theme){
        value = OCR::read_number_waterfill(
            logger, cropped,
            0xff000000, 0xffff7fff, true
        );
    }else{
        value = OCR::read_number_waterfill(
            logger, cropped,
            0xff000000, 0xff7f7f7f, false
        );
    }

    if (value < min || value > max){
        value = -1;
    }
    return value;
}



void move_horizontal(ProControllerContext& context, int current, int desired){
    while (current < desired){
        ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
        current++;
    }
    while (current > desired){
        ssf_issue_scroll_ptv(context, SSF_SCROLL_LEFT);
        current--;
    }
}

void adjust_no_wrap_Switch1(ProControllerContext& context, int current, int desired){
//    cout << "adjust_no_wrap_Switch1(): " << current << " -> " << desired << endl;

    //  Invalid read. Change something and hope the next attempt fixes it.
    if (current < 0){
        ssf_issue_scroll_ptv(context, SSF_SCROLL_UP);
        return;
    }

    while (current < desired){
        ssf_issue_scroll_ptv(context, SSF_SCROLL_UP);
        current++;
    }
    while (current > desired){
        ssf_issue_scroll_ptv(context, SSF_SCROLL_DOWN);
        current--;
    }
}
void adjust_no_wrap_Switch2(ProControllerContext& context, int current, int desired){

    //  Invalid read. Change something and hope the next attempt fixes it.
    if (current < 0){
        ssf_issue_scroll(context, SSF_SCROLL_UP, 112ms, 48ms, 24ms);
        return;
    }

    while (current < desired){
        ssf_issue_scroll(context, SSF_SCROLL_UP, 112ms, 48ms, 24ms);
        current++;
    }
    while (current > desired){
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 112ms, 48ms, 24ms);
        current--;
    }
}

void adjust_wrap_Switch1(ProControllerContext& context, int min, int max, int current, int desired){
    //  Invalid read. Change something and hope the next attempt fixes it.
    if (current < min || current > max){
        ssf_issue_scroll_ptv(context, SSF_SCROLL_UP);
        return;
    }

    int total = max - min + 1;
    int half = total / 2;

    int diff = desired - current;
    if ((diff >= 0 && diff <= half) || (diff < 0 && diff < -half)){
        while (current != desired){
            ssf_issue_scroll_ptv(context, SSF_SCROLL_UP);
            current++;
            if (current > max){
                current -= total;
            }
        }
    }else{
        while (current != desired){
            ssf_issue_scroll_ptv(context, SSF_SCROLL_DOWN);
            current--;
            if (current < min){
                current += total;
            }
        }
    }
}
void adjust_wrap_Switch2(ProControllerContext& context, int min, int max, int current, int desired){
    //  Invalid read. Change something and hope the next attempt fixes it.
    if (current < min || current > max){
        ssf_issue_scroll(context, SSF_SCROLL_UP, 112ms, 48ms, 24ms);
        return;
    }

    int total = max - min + 1;
    int half = total / 2;

    int diff = desired - current;
    if ((diff >= 0 && diff <= half) || (diff < 0 && diff < -half)){
        while (current != desired){
            ssf_issue_scroll(context, SSF_SCROLL_UP, 112ms, 48ms, 24ms);
            current++;
            if (current > max){
                current -= total;
            }
        }
    }else{
        while (current != desired){
            ssf_issue_scroll(context, SSF_SCROLL_DOWN, 112ms, 48ms, 24ms);
            current--;
            if (current < min){
                current += total;
            }
        }
    }
}
























}
}
}
