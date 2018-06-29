Currently undergoing change!

. Integrated but separeted concept: One may use:

As before: DOE & Analysis
Or: Analysis only

In addition, inductive inference is possible.
Currently MSER analysis is not included any more.

One may define parameters in the config.


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

----old readme below---

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

:::USAGE:::

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


:::EXAMPLES:::
In the folder ABMAT/EXAMPLE_FILES you can get an example DPM.tsv and also 
example *.cfg files, as well as a blueprint for the *.lsd model that contains
only the ABMAT structure. NOTE: The parameters from the example file are not 
contained in that model.

:::Notes:::
A good way to provide a DPM is given by the accompanying python toolkit.  
It is good to include the Parameters from _Max_t to _Window within the DPM. 