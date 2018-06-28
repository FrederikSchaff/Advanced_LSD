# update
now integrating into one package
# ww
​                                                                                              May 2018
 LSD Population module - backend for LSD (least 7.0)
  written by Frederik Schaff, Ruhr-University Bochum

  for infos on LSD see https://github.com/marcov64/Lsd/tree/7.1-alpha 

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

  The package relies on LSD debug module by
    F. Schaff. For further informations see: ...



# Usage

## General

You should use the template model, including the template.sim and change it to your needs.
Important: in the LSD fun\_.. file, you need to explicitly put 
	#include <random>
before the 
	#include "fun_head_fast.h"
part! Otherwise there is a problem with a macro "abs" defined in LSD source code.

The population model relies on a Top-Down (statistical) approach to define when new 
agents are born. But you may also add new agents by other means. The theoretical part
is the two parameter (discrete) survival function from (1):

​	S(age) = (exp(-beta * age) - alpha)/(1-alpha)

#### References:

(1) Boucekkine, Raouf; La Croix, David de; Licandro, Omar (2002): Vintage Human Capital, Demographic Trends, and Endogenous Growth. In Journal of Economic Theory 104 (2), pp. 340–375. DOI: 10.1006/jeth.2001.2854.

## New Macros

There is a new user macro to be used by the user / in the user code:

    POP_RCYCLE_AGENTS(X,Y)	X: Pop_model object, Y: object pointer   			  											(e.g. cur)
    	Cycles through the agents in random order, works otherwise
    	the same as the CYCLES(Z,Y) command where Z would be the 
        lsd object where the agent object is located.


## LSD Model Structure

In LSD Model, provide an object "Pop_model" which holds the following:

    ++Parameterisation++
    Pop_alpha       Parameter   alpha value
    Pop_beta        Parameter   beta value
    Pop_n_agents    Parameter   initial number of agents, equal to constant pop-size number
    Pop_birth_rate  Equation/Parameter  If equation, constant population birth rate. If instead parameter, defined birth rate.
    
    ++Mechanics++
    Pop_init        Equation    Initialise the Population Model
    Pop_birth       Equation    Controls the birth of new agent
    Pop_agent_birth Function    Each time an agent is born, creates a new agent
                                and takes care of all associated action
    Pop_death       Equation    Controls the (natural) death of agents by age
    Pop_agent_death Function    Each time an agent dies, deletes the agent
                                object from LSD and takes care of all associated
                                action, including user defined actions upon
                                death (see "Agent_death" in [1])
    Pop_age         Equation    Controls the aging of the agents.

Under root, provide also:


    Init_New_Agent  Function    User /model specific actions for new agents
    Delete_Agent    Function    User /model specific actions for dying agents
    nNewGeneration  Equation    User /model specific, defines number of agents
                                that are born (top-down) at any given time, if
                                any.