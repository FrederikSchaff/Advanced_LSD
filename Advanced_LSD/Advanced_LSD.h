/*************************************************************
                                                    May 2018
  LSD Population module - backend for LSD (least 7.0)
  written by Frederik Schaff, Ruhr-University Bochum

  for infos on LSD see ...

	Copyright Frederik Schaff
  This code is distributed under the GNU General Public License

  The complete package has the following files:
  [0] readme.md         ; readme file with instructions and information
                          on the underlying model.
  [1] fun_templ_pop.cpp ; a template file for the user model, containing the
                          links to the population model.
  [2] fun_LSD_pop.cpp   ; contains the LSD Equations for the population model.
  [3] backend_pop.h     ; contains the c++ declarations and new macros.
  [4] backend_pop.cpp   ; contains the c++ core code for the pop backend.
  [5] backend_compability.h ; helper to link with other modules.


  The package can be used together with LSD debug tools by
    F. Schaff. For further informations see: ...

 *************************************************************/

/***************************************************
backend_compability.h

This file contains necessary code checked for compability.
It must be compiled AFTER the other modules have been compiled.
****************************************************/

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

/* Same as ADDEXT but without dealocation. This should allow to call class initialisers (?)*/
#define ADDEXT2( CLASS ) { p->cext = reinterpret_cast< void * >( new CLASS ); }
#define ADDEXT2S( PTR, CLASS ) { PTR->cext = reinterpret_cast< void * >( new CLASS ); }

/*Mark the variable hosted in the pointed object as PARAMETER*/
#ifndef PARAMETERS
  #define PARAMETERS(obj,label) obj->search_var(obj,label,false)->param = 1;
#endif

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
#include <Eigen/Eigen>
#include "fun_head_fast.h"

#include "tools/debug.h"
#include "Advanced_LSD.cpp" //some general helpers for the backends

#define GET_ID(c) AdvLSD_FakeID(c)
#define GET_ID_LABEL(c) AdvLSD_FakeID_Label(c).c_str()

#define GET_VAR(c,lab2) AdvLSD_FakeVar(c,lab2)
#define GET_VAR_LABEL(c,lab2) AdvLSD_FakeVar_Label(c,lab2).c_str()

#ifdef MODULE_ABMAT
  #include "ABMAT/ABMAT_head.h"  //load the Analysis Toolkit
#endif

#ifdef MODULE_GEOGRAPHY
  #include "gis_backend/gis.cpp"
#endif

#ifdef MODULE_POPULATION
  #include "pop_backend/pop.cpp"
#endif

#ifdef MODULE_PAJEK
  #include "pajek_backend/pajek.cpp"
#endif
