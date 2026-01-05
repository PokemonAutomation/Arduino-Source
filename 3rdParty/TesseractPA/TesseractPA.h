/*  Tesseract Wrapper
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 */

#ifndef PokemonAutomation_TesseractPA_H
#define PokemonAutomation_TesseractPA_H

#include <stdint.h>
#include <cstddef>

//#define TESSERACT_STATIC

#ifdef TESSERACT_STATIC
#define TESSERACT_EXPORT
#else

#ifdef _WIN32

#ifdef _WINDLL
#define TESSERACT_EXPORT __declspec(dllexport)
#else
#define TESSERACT_EXPORT __declspec(dllimport)
#endif

#else // not on _WIN32

#define TESSERACT_EXPORT __attribute__((visibility("default")))

#endif // _WIN32
#endif // TESSERACT_STATIC


#ifdef PA_TESSERACT
#elif _MSC_VER
#pragma warning(disable:4100)   //  Unreferenced Formal Parameter
#elif __GNUC__
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif



#ifdef __cplusplus
extern "C" {
#endif

// We build a custom Tesseract library,
// https://github.com/PokemonAutomation/Tesseract-OCR_for_Windows
// to ship with SerialProgram on Windows, in the form of tesseractPA.dll.
// The following C API is a wrapper for the raw Tesseract API. The wrapper
// along with Tesseract itself is implemented in tesseractPA.dll.
struct TesseractAPI_internal;
TESSERACT_EXPORT TesseractAPI_internal* TesseractAPI_construct(
    const char* path, const char* language
);
TESSERACT_EXPORT void TesseractAPI_destroy(TesseractAPI_internal* api);

//TESSERACT_EXPORT char* TesseractAPI_read_file(TesseractAPI_internal* api, const char* filepath);
TESSERACT_EXPORT char* TesseractAPI_read_bitmap(
    TesseractAPI_internal* api,
    const unsigned char* data,
    size_t width, size_t height,
    size_t bytes_per_pixel, size_t bytes_per_line,
    size_t ppi
);
TESSERACT_EXPORT void TesseractAPI_set_page_seg_mode(
    TesseractAPI_internal* api,
    int psm
);
TESSERACT_EXPORT void Tesseract_delete(char* text);


#ifdef __cplusplus
}
#endif


class TesseractString{
public:
    ~TesseractString(){
#ifdef PA_TESSERACT
        if (m_str != nullptr){
            Tesseract_delete(m_str);
        }
#endif
    }
    TesseractString(const TesseractString&) = delete;
    void operator=(const TesseractString&) = delete;
    TesseractString(TesseractString&& x)
        : m_str(x.m_str)
    {
        x.m_str = nullptr;
    }
    void operator=(TesseractString&& x){
        m_str = x.m_str;
        x.m_str = nullptr;
    }

public:
    const char* c_str() const{
        return m_str;
    }

private:
    TesseractString(char* str)
        : m_str(str)
    {}

private:
    friend class TesseractAPI;
    char* m_str;
};


class TesseractAPI{
public:
    ~TesseractAPI(){
#ifdef PA_TESSERACT
        if (m_api != nullptr){
            TesseractAPI_destroy(m_api);
        }
#endif
    }
    TesseractAPI(const TesseractAPI&) = delete;
    void operator=(const TesseractAPI&) = delete;
    TesseractAPI(TesseractAPI&& x)
        : m_api(x.m_api)
    {
        x.m_api = nullptr;
    }
    void operator=(TesseractAPI&& x){
        m_api = x.m_api;
        x.m_api = nullptr;
    }

public:
    TesseractAPI(const char* path, const char* language)
#ifdef PA_TESSERACT
        : m_api(TesseractAPI_construct(path, language))
#endif
    {}

    bool valid() const{ return m_api != nullptr; }

    void set_page_seg_mode(int psm){
#ifdef PA_TESSERACT
        TesseractAPI_set_page_seg_mode(m_api, psm);
#endif
    }

//    TesseractString read(const char* filepath){
//#ifdef PA_TESSERACT
//        return TesseractAPI_read_file(m_api, filepath);
//#else
//        return nullptr;
//#endif
//    }
    TesseractString read32(
        const unsigned char* data,
        size_t width, size_t height,
        size_t bytes_per_line, size_t ppi = 100
    ){
#ifdef PA_TESSERACT
        return TesseractAPI_read_bitmap(
            m_api,
            data,
            width, height,
            sizeof(uint32_t),
            bytes_per_line,
            ppi
        );
#else
        return nullptr;
#endif
    }

private:
    TesseractAPI_internal* m_api = nullptr;
};




#endif

