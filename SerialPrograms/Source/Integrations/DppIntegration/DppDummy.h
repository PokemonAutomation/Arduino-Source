#pragma once
#ifndef DPP_DUMMY_H
#define DPP_DUMMY_H

#if 0

#elif _MSC_VER

#pragma warning( push )
#pragma warning( disable : 4100 )
#include <dpp/dpp.h>
#pragma warning( pop )

#elif __GNUC__

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <dpp/dpp.h>
#pragma GCC diagnostic pop

#else

#include <dpp/dpp.h>

#endif

#endif
