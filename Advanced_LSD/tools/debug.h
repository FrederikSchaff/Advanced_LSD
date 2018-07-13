/*************************************************************
                                                    May 2018
  LSD Debug module - backend for LSD (least 7.0)
  written by Frederik Schaff, Ruhr-University Bochum

  for infos on LSD see ...

	Copyright Frederik Schaff
  This code is distributed under the GNU General Public License

  The complete package has the following files:
  [0] readme.md          ; readme file with instructions and information on this
                           LSD module.
  [1] debug.h            ; contains the macros and includes other module files
  [2] validate.cpp       ; simple utilities to enhance the validation /
                           debugging of LSD models

 *************************************************************/

/***************************************************
debug.h

This file contains the declarations and new macros for the population model.
It also includes the other core-code files (all not fun_*)
****************************************************/


/********************************************************/
/* Some LSD Macros for debugging                        */
/* Add BEFORE #include externals/debug.h

  . to switch .. off :
    .. testing off:       #define SWITCH_TEST_OFF
    .. verbose mode off:  #define SWITCH_VERBOSE_OFF
    .. tracking off:      #define SWITCH_TRACK_OFF

  . to define .. :
   ..  the maximum steps tracking is active: #define TRACK_SEQUENCE_MAX_T n
      //n>0 is the number of steps tracking is active

/********************************************************/

#include <ctime>

// To get time info, see   http://stackoverflow.com/a/2962914/3895476
#define SET_LOCAL_CLOCK \
  struct timespec local_start, local_finish;   \
  clock_gettime(CLOCK_MONOTONIC, &local_start); \
  int local_clock_id = int(local_start.tv_sec);\
  PLOG("\ntCLOCK Setting clock with ID %i",local_clock_id);

#define RESET_LOCAL_CLOCK \
  local_clock_id++;\
  clock_gettime(CLOCK_MONOTONIC, &local_start);\
  local_clock_id = int(local_start.tv_sec);\
  PLOG("\n\tCLOCK Re-setting clock with new ID %i",local_clock_id);

#define REPORT_LOCAL_CLOCK    \
{                             \
clock_gettime(CLOCK_MONOTONIC, &local_finish);\
double elapsed = (local_finish.tv_sec - local_start.tv_sec);\
       elapsed += (local_finish.tv_nsec - local_start.tv_nsec) / 1000000000.0;\
PLOG("\n\tCLOCK Local clock with ID %i: Seconds elapsed: %g",local_clock_id,elapsed);\
}

#ifndef MODULE_DEBUG //GUARD
  #define MODULE_DEBUG
#endif

//in no window mode, stop all information printing
#ifndef NO_WINDOW_TRACKING
  #ifdef NO_WINDOW
    #ifndef SWITCH_TRACK_SEQUENCE_OFF
      #define SWITCH_TRACK_SEQUENCE_OFF
    #endif
    #ifndef SWITCH_VERBOSE_OFF
      #define SWITCH_VERBOSE_OFF
    #endif
    #ifndef SWITCH_TEST_OFF
      #define SWITCH_TEST_OFF
    #endif
  #endif
#endif


/* To clearly mark tests and also allow to not run them */

#ifndef SWITCH_TEST_OFF
  #define TEST_MODE(X) if (X)           //Testing on
  #define TEST_ELSE } else {
#else
  #define TEST_MODE(X) if (false && X)  //Testing off
  #define TEST_ELSE
#endif

/* A verbose mode */

#ifndef SWITCH_VERBOSE_OFF
  #define VERBOSE_MODE(X) if (X)          //Verbose on
#else
  #define VERBOSE_MODE(X) if (false && X) //Verbose off
#endif

/* A macro to save the stats withour updating */
#ifndef ABMAT_USE_ANALYSIS
  #define ABORT_STATS
#else
  #define ABORT_STATS V_CHEAT("ABMAT_UPDATE",NULL);
#endif

/* A more severe ABORT, that also stops the computation of the
  current EQUATION*/
#define ABORT2 \
        ABORT_STATS \
        ABORT       \
        END_EQUATION(0.0) //prematurely end the current equation

/* Tracking of equations etc., special tracking of objects with "_ID" or "ID".*/

#ifndef SWITCH_TRACK_SEQUENCE_OFF
  #include "validate.cpp"
//   #undef TRACK_SEQUENCE
  #define TRACK_SEQUENCE \
  if ( t <= TRACK_SEQUENCE_MAX_T)  { LSD_VALIDATE::track_sequence(t,p,c,var); };
  //   #undef TRACK_SEQUENCE_FIRST_OR_LAST
  #define TRACK_SEQUENCE_FIRST_OR_LAST \
    if ( t <= TRACK_SEQUENCE_MAX_T)  { LSD_VALIDATE::track_sequence(t,p,c,var,false); };
  #define TRACK_SEQUENCE_ALWAYS { LSD_VALIDATE::track_sequence(t,p,c,var); };
#else
  #define TRACK_SEQUENCE
  #define TRACK_SEQUENCE_FIRST_OR_LAST
  #define TRACK_SEQUENCE_ALWAYS
#endif



