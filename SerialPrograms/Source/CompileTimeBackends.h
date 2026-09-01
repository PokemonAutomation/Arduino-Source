/*  Compile-Time Backends
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CompileTimeBackends_H
#define PokemonAutomation_CompileTimeBackends_H


//  ImageRGB32 (select at most 1)
#ifndef PA_IMAGE_BACKEND_None
#ifndef PA_IMAGE_BACKEND_Qt
#ifndef PA_IMAGE_BACKEND_OpenCV
#define PA_IMAGE_BACKEND_OpenCV     //  <-- Default
#endif
#endif
#endif

#ifndef PA_FILE_SYSTEM_BACKEND_OS_SPECIFIC
#ifndef PA_FILE_SYSTEM_BACKEND_Qt
#define PA_FILE_SYSTEM_BACKEND_OS_SPECIFIC  //  <-- Default
#endif
#endif


#endif
