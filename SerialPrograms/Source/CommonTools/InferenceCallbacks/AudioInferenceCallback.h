/*  Audio Inference Callback
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_AudioInferenceCallback_H
#define PokemonAutomation_CommonTools_AudioInferenceCallback_H

#include <memory>
#include <vector>
#include "InferenceCallback.h"

namespace PokemonAutomation{

class AudioSpectrum;
class AudioFeed;

//  Base class for an audio inference object to be called perioridically by
//  inference routines in InferenceRoutines.h.
class AudioInferenceCallback : public InferenceCallback{
public:
    AudioInferenceCallback(std::string label)
        : InferenceCallback(InferenceType::AUDIO, std::move(label))
    {}

    //  Process new spectrums and do inferences on them.
    //  Return true if the inference session should stop.
    //  Input spectrums are ordered from newest (largest timestamp) to oldest (smallest timestamp) in the vector.
    //  If needed, access to `audioFeed` to render inference boxes.
    virtual bool process_spectrums(
        const std::vector<AudioSpectrum>& new_spectrums,
        AudioFeed& audio_feed
    ) = 0;

};




}
#endif
