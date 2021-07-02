/*  Tesseract Wrapper
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 */

#ifndef PokemonAutomation_TesseractPA_H
#define PokemonAutomation_TesseractPA_H

#include <stdint.h>

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

#else

#define TESSERACT_EXPORT __attribute__((visibility("default")))

#endif
#endif



#ifdef __cplusplus
extern "C" {
#endif


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

