/*  Cropped Silhouette Matcher
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTools/ImageDiff.h"
#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "ImageCropper.h"
#include "SilhouetteDictionaryMatcher.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace ImageMatch{


void SilhouetteDictionaryMatcher::add(const std::string& slug, const ImageViewRGB32& image){
    if (!image){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Null image.");
    }
    auto iter = m_database.find(slug);
    if (iter != m_database.end()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Duplicate slug: " + slug);
    }

    iter = m_database.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(slug),
        std::forward_as_tuple(trim_image_alpha(image).copy())
    ).first;
    m_database_vector.emplace_back(&*iter);
}



ImageMatchResult SilhouetteDictionaryMatcher::match(
    const ImageViewRGB32& image,
    double alpha_spread
) const{
    ImageMatchResult results;
    if (!image){
        return results;
    }

    SpinLock lock;
    GlobalThreadPools::normal_inference().run_in_parallel(
        [&](size_t index){
            const auto& matcher = *m_database_vector[index];
            double alpha = matcher.second.rmsd_masked(image);
            WriteSpinLock lg(lock);
            results.add(alpha, matcher.first);
            results.clear_beyond_spread(alpha_spread);
        },
        0, m_database_vector.size(),
        100
    );


#if 0
    for (const auto& item : m_database){
//        if (item.first != "solosis"){
//            continue;
//        }
        double alpha = item.second.rmsd_masked(image);
//        WriteSpinLock lg(lock);
        results.add(alpha, item.first);
        results.clear_beyond_spread(alpha_spread);
    }
#endif

    return results;
}






}
}
