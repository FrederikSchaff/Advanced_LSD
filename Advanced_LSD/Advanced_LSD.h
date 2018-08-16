/*************************************************************
                                                    May 2018

  Wrapper for several modules. See the readme (to be updated)

	Copyright Frederik Schaff
  This code is distributed under the GNU General Public License


 *************************************************************/


/*++++++++++++++++++++++++++++*/
/* Some additional LSD Macros */

/* We use the FUNCTION keyword to visually mark the difference between
  Equations (updated once per time-step) and Functions (updated whenever called)
  within the source code. Obviously the real difference is defined in the
  LSD Browser.
*/
#ifndef FUNCTION
  #define FUNCTION EQUATION
#endif

/* Same as ADDEXT but without dealocation. This allows to call class initialisers (!)*/
#define ADDEXT2( CLASS ) { p->cext = reinterpret_cast< void * >( new CLASS ); }
#define ADDEXT2S( PTR, CLASS ) { PTR->cext = reinterpret_cast< void * >( new CLASS ); }

/*Mark the variable hosted in the pointed object as PARAMETER*/
#ifndef PARAMETERS
  #define PARAMETERS(obj,label) obj->search_var(obj,label,false)->param = 1;
#endif

#include "tools/CreateDir.h" //to create dirs

/*----------------------------*/

// Forward, because ABMAT is not yet adjusted.
#ifdef MODULE_ABMAT
  #define ABMAT_USE_ANALYSIS
#endif

/*
For all the backends, there is an Agent-class defined. The default is "Agent"
*/
#ifndef BACKEND_AGENT
  #define BACKEND_AGENT "Agent"
#endif

/*
For all the backends, there is a Patch-class defined. The default is "Patch"
*/
#ifndef BACKEND_PATCH
  #define BACKEND_PATCH "Patch"
#endif

#include <random>    //else potential problems with _abs()
//#include <Eigen/Eigen>
#include "fun_head_fast.h"

//to do: link to lsd rng
// template< class tmp_prng > //template class for random number generator

#include "tools/debug.h"
#include "tools/utilities.h" //rnd generator, beta distribution
#include "Advanced_LSD.cpp" //some general helpers for the backends

//Macros also used in submodules!
#define GET_ID(c) AdvLSD_FakeID(c)
#define GET_ID_LABEL(c) AdvLSD_FakeID_Label(c).c_str()

#define GET_VAR(c,lab2) AdvLSD_FakeVar(c,lab2)
#define GET_VAR_LABEL(c,lab2) AdvLSD_FakeVar_Label(c,lab2).c_str()

#ifdef MODULE_ABMAT
  #include "ABMAT/ABMAT_head.h"  //load the Analysis Toolkit
#endif


#ifdef MODULE_PAJEK
  //#include "pajek_backend/pajek.cpp"
  #include "pajek_backend/Pajek_new.cpp"   //create pajek object
#else
  #define PAJ_SAVE //do nothing
  #define PAJ_MAKE_AVAILABLE //do nothing
#endif

#ifdef MODULE_GEOGRAPHY
  #include "gis_backend/gis.cpp"
#endif

#ifdef MODULE_POPULATION
  #include "pop_backend/pop.cpp"
#endif


//Macros
#ifdef MODULE_PAJEK
  #include "pajek_backend/macro_pajek.h"
#endif

#ifdef MODULE_GEOGRAPHY
  #include "gis_backend/macro_gis.h"
#endif

#ifdef MODULE_POPULATION
  #include "pop_backend/macro_pop.h"
#endif

//Wrapper Macros
#define OPEN_ADVANCED_LSD \
  PAJ_MAKE_AVAILABLE

#define CLOSE_ADVANCED_LSD \
    PAJ_SAVE \

