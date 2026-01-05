/*  Tesseract Wrapper
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 */

#ifdef UNIX_LINK_TESSERACT

#include "TesseractPA.h"

#include <tesseract/baseapi.h>
#include <cstddef>
#include <iostream>
using std::cout;
using std::endl;


// We hope to use our own Tesseract build in future for Unix systems too.
// But right now to run on Linux and Mac we need to use external Tesseract library API directly.
// So the Tesseract API wrapper is defined here to match Windows.


struct TesseractAPI_internal{
    tesseract::TessBaseAPI m_api;

    TesseractAPI_internal(const char* path, const char* language){
        if (m_api.Init(path, language)){
            throw "Could not initialize TesseractAPI.";
        }
    }

};



TesseractAPI_internal* TesseractAPI_construct(const char* path, const char* language){
    try{
        return new TesseractAPI_internal(path, language);
    }catch (const char* err){
        cout << err << endl;
    }
    return nullptr;
}
void TesseractAPI_destroy(TesseractAPI_internal* api){
    delete api;
}



char* TesseractAPI_read_bitmap(
    TesseractAPI_internal* api,
    const unsigned char* data,
    size_t width, size_t height,
    size_t bytes_per_pixel, size_t bytes_per_line,
    size_t ppi
){
    api->m_api.SetImage(data, (int)width, (int)height, (int)bytes_per_pixel, (int)bytes_per_line);
    api->m_api.SetSourceResolution((int)ppi);
    return api->m_api.GetUTF8Text();
}
void TesseractAPI_set_page_seg_mode(TesseractAPI_internal* api, int psm){
    api->m_api.SetPageSegMode(static_cast<tesseract::PageSegMode>(psm));
}
void Tesseract_delete(char* text){
    delete[] text;
}


#endif // UNIX_LINK_TESSERACT




