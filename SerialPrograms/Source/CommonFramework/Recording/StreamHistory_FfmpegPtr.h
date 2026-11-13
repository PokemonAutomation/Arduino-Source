/*  Stream History - ffmpeg Ownership Helpers
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  RAII owners for the ffmpeg objects used by the stream history:
 *  std::unique_ptr aliases for the objects with a single free function, and
 *  small owners for the two that need more (an output format context and an
 *  option dictionary).
 *
 *  Requires ffmpeg (PA_STREAM_HISTORY_FFMPEG).
 *
 */

#ifndef PokemonAutomation_StreamHistory_FfmpegPtr_H
#define PokemonAutomation_StreamHistory_FfmpegPtr_H

#include <memory>
#include <type_traits>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/dict.h>
#include <libavutil/frame.h>
#include <libswscale/swscale.h>
}

namespace PokemonAutomation{


//  A std::unique_ptr deleter that calls the ffmpeg free function
//  'free_function'. ffmpeg's free functions take either the pointer
//  (sws_freeContext) or its address (av_frame_free and the other *_free
//  functions, which also null the caller's pointer); the right form is
//  picked at compile time. The deleter is empty, so an FfmpegPtr is the
//  size of a raw pointer.
template <auto free_function>
struct FfmpegDeleter{
    template <typename Type>
    void operator()(Type* pointer) const{
        //  A function that takes void* (av_free, av_freep) accepts both forms,
        //  so the form cannot be picked safely. Write a deleter by hand.
        struct Unrelated{};
        static_assert(
            !std::is_invocable_v<decltype(free_function), Unrelated*>,
            "FfmpegDeleter cannot be used with a free function that takes void*."
        );
        if constexpr (std::is_invocable_v<decltype(free_function), Type**>){
            free_function(&pointer);
        }else{
            free_function(pointer);
        }
    }
};

template <typename Type, auto free_function>
using FfmpegPtr = std::unique_ptr<Type, FfmpegDeleter<free_function>>;

using FramePtr              = FfmpegPtr<AVFrame, &av_frame_free>;
using PacketPtr             = FfmpegPtr<AVPacket, &av_packet_free>;
using CodecParametersPtr    = FfmpegPtr<AVCodecParameters, &avcodec_parameters_free>;
using CodecContextPtr       = FfmpegPtr<AVCodecContext, &avcodec_free_context>;
using SwsContextPtr         = FfmpegPtr<SwsContext, &sws_freeContext>;


//  Owns an output (muxing) format context: closes its file, if the muxer
//  uses one, then frees it. Writing the trailer is not cleanup and stays
//  with the caller. Not for input contexts (those use avformat_close_input()).
struct OutputFormatContextDeleter{
    void operator()(AVFormatContext* context) const{
        if ((context->oformat->flags & AVFMT_NOFILE) == 0){
            avio_closep(&context->pb);
        }
        avformat_free_context(context);
    }
};
using OutputFormatContextPtr = std::unique_ptr<AVFormatContext, OutputFormatContextDeleter>;


//  Owns an option dictionary. The ffmpeg functions that take options
//  (av_dict_set, avio_open2, avformat_write_header) replace the dictionary
//  through an AVDictionary**, which a std::unique_ptr cannot hand out, so
//  this exposes the address of the owned pointer instead.
class FfmpegDictionary{
public:
    FfmpegDictionary() = default;
    ~FfmpegDictionary(){
        av_dict_free(&m_dictionary);
    }
    FfmpegDictionary(const FfmpegDictionary&) = delete;
    void operator=(const FfmpegDictionary&) = delete;

    AVDictionary** address(){
        return &m_dictionary;
    }

private:
    AVDictionary* m_dictionary = nullptr;
};


}
#endif
