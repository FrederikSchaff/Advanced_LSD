
#ABMAT - ABM Analysis Toolkit (for LSD)
 LSD ABMAT module - backend for LSD (least 7.0)                                     May 2018

  written by Frederik Schaff, Ruhr-University Bochum

  for infos on LSD see https://github.com/marcov64/Lsd/tree/7.1-alpha 

    Copyright Frederik Schaff

  This code is distributed under the GNU General Public License


##Minimal Description

The ABM Analysis Toolkit for LSD is a work-in-progress toolkit to facilitate simple but comprehensive and performant descriptive analysis of ABMs programmed with LSD.

###Integration & Usage
Assuming you have the content of this package under "/external/ABMAT/" in your LSD model directory. In the fun... file, before MODELBEGIN, put the following (and follow the instructions)
    /******************************************************************************/
    /*          A B M A T                                                    */
    /*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    To use a _minimal version_ of ABMAT (only computation of stats, no DOE)
    In LSD Model, provide an object "ABMAT" which holds the following:

    ++Mechanics++
    ABMAT_Update    EQUATION  at the end of each tick, update all variables
    ABMAT_Init      EQUATION  initialise the ABMAT tool
    
    in the folders specified below, provide configs for
    .. the intervals of choice
    .. the parameters/variables of choice
    (see the example config for details)
    
    For each run, a folder structure with the results will be created.
    The "CID" equals the seed-1 , i.e. the seed that needs to be set to re-
    create the same results. (LSD increments the seed after initialisation the
    prng)
    /*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    #define ABMAT_ANALYSIS_CFG_PATH "DOE/ABMAT_Analysis.cfg"
    #define ABMAT_INTERVALS_CFG_PATH "DOE/ABMAT_Intervals.cfg"
    #define ABMAT_USE_ANALYSIS                               
    #include "external/ABMAT/ABMAT_head.h"  //load the Analysis Toolkit
    /* -------------------------------------------------------------------------- */
and after MODELBEGIN put:

    #include "external/ABMAT/ABMAT_LSD.cpp"   //load the ABMAT equations

Then (optionally but preferable) provide an equation "Update_Scheme" or similar that is called first at each simulation step. It should very fist call the equation "ABMAT_INIT" (which becomes a parameter thereafter) and at the end of the procedure call "ABMAT_UPDATE" to save all the information.
___
___
##Currently undergoing change! Old readme parts below
___
. Integrated but separeted concept: One may use:

As before: DOE & Analysis
Or: Analysis only

In addition, inductive inference is possible.
Currently MSER analysis is not included any more.

One may define parameters in the config.
One may define the time-frame for the analysis of the stationary part (default: data_size/2) via ABMAT_NON_TRANSIENT
One may define ABMAT_TRANSIENT (positive: length of trans, 0 or neg: half data size) to have an additional analysis of the transient 

To Do:

. use DOE only (?)
. Use Config File for Analysis Parameters.
. Implement ABMAT such that the .LSD file needs NO manipulation.
  .. Add to Analysis option to specify Parameters. 
    ... Ensure that LSD-seed is selected as parameter always
    ...   
  .. Define DOE Parameters separate from Analysis
    ... Specify that if this is the choice, DOE parameters are checked to be
        part already in the analysis files
. provide more easy configuration of stats to be taken. Via defs??
. ...

- - -

----old readme below---

###Example Components ...
The following variables need to be present in the *.lsd file/model.
+-------------------+----------------------------+------------+-------------------------------------------------------------------------------------------------------+--+
|      Label        |         Values             |    Type    |                                                Comment                                                |  |
+-------------------+----------------------------+------------+-------------------------------------------------------------------------------------------------------+--+
| ABMAT_INIT        | -                          | EQUATION   | Needs to be called when the simulation is initialised.                                                |  |
| ABMAT_UPDATE      | -                          | EQUATION   | Needs to be called at the end of a simulation step, when all the statistics are gathered.             |  |
| ________________  | __________________________ | __________ | _____________________________________________________________________                                 |  |
| ABMAT_DPM         | [1,...,m]                  | Parameter  | Specify the number of the design point matrix to be loaded.                                           |  |
| ABMAT_Switch      | 0, [1,...,n] or [-1,...-s] | Parameter  | Specify either a specific single config (pos) or a set (neg) from ... or disable ABMAT (0)            |  |
| ABMAT_Sets        | [1,...,s]                  | Parameter  | ... the total number of sets to be generated (each containing 1/s configs, the last set pot. smaller) |  |
| ________________  | __________________________ | __________ | _____________________________________________________________________                                 |  |
| ABMAT_Max_t       | [t in N]                   | Parameter  | The number of simulation steps and thus ticks.                                                        |  |
| ABMAT_SeqMC_BCc   | [10,20, or any N]          | Parameter  | The Besag-Clifford Constant for the sequental Monte Carlo analysis.                                   |  |
| ABMAT_SeqMC_Pv    | (>0,<<1)                   | Parameter  | The target p-value for the sequential Monte Carlo analysis.                                           |  |
| ABMAT_SeqMC_seed  | (>0)                       | Parameter  | The seed for the sequential Monte Carlo analysis                                                      |  |
| ABMAT_Window      | [>>1,<<t]                  | Parameter  | Specifies the length of the tail of each statistic covered that is subject of the analysis.           |  |
| ABMAT_Transient   | [0, or any N>0]            | Parameter  | Specifies the length of the transient to be analysed.                                                 |  | 
| ________________  | __________________________ | __________ | _____________________________________________________________________                                 |  |
| ABMAT_ConfigID    | out                        | Parameter  | Save the config ID (from the column ConfigID in the DPM.                                              |  |
+-------------------+----------------------------+------------+-------------------------------------------------------------------------------------------------------+--+

###USAGE

In the fun_*.cpp, right after the #include "fun_head.h" add:

    #define ABMAT_ANALYSIS_CFG_PATH "DOE/ABMAT_Analysis.cfg" //Change Path Accordingly
    #define ABMAT_DPM_CFG_PATH "DOE/ABMAT_DPM.cfg" //Change Path Accordingly! 
    #include "ABMAT/ABMAT.cpp" //Load the ABMAT toolkit

and after MODELBEGIN add the necessary variables ABMAT_INIT and ABMAT_UPDATE 
by simply adding:

	#include "ABMAT/ABMAT_LSD.cpp"

In the LSD model, ensure that the DOE is initialised at the very beginning by 
either shifting the Variablea ABMAT_INIT to the top in the root directory of
the model, or by calling it, before anything else, manually.


###EXAMPLES
In the folder ABMAT/EXAMPLE_FILES you can get an example DPM.tsv and also 
example *.cfg files, as well as a blueprint for the *.lsd model that contains
only the ABMAT structure. NOTE: The parameters from the example file are not 
contained in that model.

###Notes
A good way to provide a DPM is given by the accompanying python toolkit.  
It is good to include the Parameters from _Max_t to _Window within the DPM. 