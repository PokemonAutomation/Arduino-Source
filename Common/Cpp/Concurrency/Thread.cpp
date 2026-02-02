/*  Thread
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Qt6.9ThreadBugWorkaround.h"

#ifdef PA_ENABLE_QT_ADOPTION_WORKAROUND
#include "Backends/Thread_Qt.tpp"
//#include "Backends/Thread_StdThreadDetach.tpp"
#else
#include "Backends/Thread_StdThread.tpp"
#endif
