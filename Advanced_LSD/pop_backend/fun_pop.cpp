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
fun_LSD_pop.cpp

This file contains all the LSD EQUATIONS that are part of the
template and can be left "as is" by users.
****************************************************/

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
/*----------------------------------------------------------------------------*/

FUNCTION("Pop_init")
/* Initialise the population model. Called via fake_caller with callee being the
agent object */
TRACK_SEQUENCE

  /* Load parameters */
  double beta = V("Pop_beta");
  double alpha = V("Pop_alpha");
  int n_const = int(V("Pop_const_n"));

                      VERBOSE_IN(true)
                        PLOG("\nInitialising Population model.");
                        PLOG("\n alpha: %g, beta: %g, n_const: %i",alpha,beta,n_const);
                      VERBOSE_OUT

                      TEST_IN( !(alpha > 1 && beta < 0) && !(alpha > 0 && alpha < 1 && beta > 0) )
                        	PLOG("\nError: alpha and beta combination not allowed!");
                        	ABORT END_EQUATION(0.0)
                      TEST_OUT


  /* Init external backend */
  POP_INIT(c->label,seed,alpha,beta, n_const);

  /*------*/

  double birth_rate = V("Pop_birth_rate");  //exogeneous, fixed (pot) birth rate
                            VERBOSE_IN(true)
                              PLOG("\nPopulation birth_rate is %g",birth_rate);
                            VERBOSE_OUT

  /* For efficiency only. Realocate the vector holding the pointers for each
    agent ever existed in order to be sufficiently big for the whole run. */
  if (n_const > 0){
    int max_expected_entries = n_const + birth_rate*double(n_const)*double(max_step)*1.05; //extra 5%
    P_EXT(ext_pop)->expected_total(max_expected_entries);
  }

  /*++++++++++++++++++++++++++++++++++++++++++++*/
  /*    Initialise the age-structure

    We can initialise the initial age structure proportional to the survival
    rates by assigning each agent a random uniform value within 0,1.

    Remark: We *should* re-initialise the death-age to the conditional one
            Current approach: Those initial agents do not get a death-age,
            instead we check each year if the die (AND NOT CORRECTLY).
  */

    int n_agent = COUNTS(p->up,P_EXT(ext_pop)->agent_label);

    PLOG("\nWe expect there to be a total of %i initial agents.",n_agent);
    #ifndef NO_WINDOW
//       INTERACT("pausing",0.0);
    #endif
    std::vector< double > age =  P_EXT(ext_pop)->pop_init_age_dist(n_agent);
    int cur_age_idx = 0;     //Assign age to agents

    //For all existing agents, create and link backend file + add age and ID
    object *ptrAgent;
    ext_pop_agent *ptrAgent_ext;
    int ID;
    CYCLES(p->up,ptrAgent,P_EXT(ext_pop)->agent_label){
        TEST_IN(cur_age_idx>age.size()-1)
          PLOG("\nError! See line 181 in fun_templ..");
          break;
        TEST_OUT
      ptrAgent_ext = P_EXT(ext_pop)->newAgent(ptrAgent);
      ID = ptrAgent_ext->ID;
      WRITES(ptrAgent,GET_ID_LABEL(ptrAgent), ID);
      WRITES(ptrAgent,GET_VAR_LABEL(ptrAgent,"_age"),age.at(cur_age_idx)); //Age from dist
      ptrAgent_ext->age=age.at(cur_age_idx);
      WRITES(ptrAgent,GET_VAR_LABEL(ptrAgent,"_death_age"), -1); //use negative value to indicate that death is decided each single year for the initial population.
      ptrAgent_ext->death_age=-1;
      VERBOSE_IN(false)
        PLOG("\nAdded an extension to agent %i/%i",ID,(int)VS(ptrAgent,GET_ID_LABEL(ptrAgent)));
      VERBOSE_OUT
      cur_age_idx++;
    }

  /*--------------------------------------------*/


  /*------*/


  //Go through the standard birth process, only skipping the part with the
  //creation and linking of age
  CYCLES(p->up,ptrAgent,P_EXT(ext_pop)->agent_label){
    V_CHEAT("Pop_agent_birth",ptrAgent);
  }
  /*------*/




  /* What about relations? How to initialise a heritage network? */
  //to do,

PARAMETER
RESULT(0)

FUNCTION("Pop_agent_death")
/* Delete the provided (via FAKE CALLER) agent.*/
TRACK_SEQUENCE

                    TEST_IN(std::string(c->label)!=std::string(P_EXT(ext_pop)->agent_label))
                      PLOG("\nError! Trying to delete %s which is not of type %s",c->label,P_EXT(ext_pop)->agent_label);
                    TEST_OUT
  V_CHEATS(p->up,"Delete_Agent",c); //User specific action when an agent is deleted.
  P_EXT(ext_pop)->agentDies(GET_ID(c));
  DELETE(c); //delete the caller.

RESULT(0)

FUNCTION("Pop_agent_birth")
/* Create a new agent. FAKE CALLER tells where. If the callee object is an agent,
we are in initialisation-only mode and the existing agent is initialised instead.
If the fake caller is the object that contains the agents, we are in the create-and-initialise mode.
Additional conditions are possible, e.g. the availability of suitable parents.
*/
TRACK_SEQUENCE

  bool initialise_only = false;
    object *ptrAgent;
    ext_pop_agent *ptrAgent_ext;
    int ID=-1;

      //initialise_only mode
  if (std::string(c->label) == std::string(P_EXT(ext_pop)->agent_label)){
    ptrAgent = c;  //we are in initialisation mode - agent exists as does ext_pop_agent
    ID = GET_ID(ptrAgent);
    ptrAgent_ext = P_EXT(ext_pop)->getAgentExt(ID);
    initialise_only = true;

      //init-and-create mode
  } else {
    ptrAgent = ADDOBJS(c,P_EXT(ext_pop)->agent_label); //we are in creation mode - agent does not yet exist
    ptrAgent_ext = P_EXT(ext_pop)->newAgent(ptrAgent);
    ID = ptrAgent_ext->ID;
    WRITES(ptrAgent,GET_ID_LABEL(ptrAgent), ID);
    WRITES(ptrAgent,GET_VAR_LABEL(ptrAgent,"_age"),0); //is the same at agent_ext initialisation
    WRITES(ptrAgent,GET_VAR_LABEL(ptrAgent,"_death_age"),ptrAgent_ext->death_age); //Already, it is decided when the agent will die (at least)
  }

  WRITES(ptrAgent,GET_VAR_LABEL(ptrAgent,"_female"),ptrAgent_ext->female); //will be 0 for male, 1 for female.

  V_CHEATS(p->up,"Init_New_Agent",ptrAgent); //User specific action on agent creation
                    // User specific action includes the provision of parents

  int father_ID = GET_VAR(ptrAgent,"_father");
  int mother_ID = GET_VAR(ptrAgent,"_mother");
  if (mother_ID<0 ) {
    PLOG("\n No mother for newborn %i!",ID);
  } else {
    P_EXT(ext_pop)->mother_and_child(mother_ID,ID); //Tell child its mother
  }
  if (father_ID<0 ) {
    PLOG("\n No father for newborn %i!",ID);
  } else {
    P_EXT(ext_pop)->father_and_child(father_ID,ID); //Tell father his child
  }




RESULT(0)

EQUATION("Pop_death")
/* Check which agents need to be deleted and call "Pop_agent_death" for each one
*/
TRACK_SEQUENCE
  double dead=0.0;
  int death_age,age;
  bool others_alive = false; //check to NOT delete last agent.
  CYCLE_SAFES(p->up,cur,P_EXT(ext_pop)->agent_label){
    age = GET_VAR(cur,"_age");
    death_age = GET_VAR(cur,"_death_age");
    if ( (death_age == -1 && pop_uniform() < P_EXT(ext_pop)->pop_hazard_rate(age) )
         || (death_age >= 0 && (age >= death_age ) )
       ) {
      if (others_alive || go_brother(cur)!=NULL){
        V_CHEAT("Pop_agent_death",cur);
        dead++;
      } else {
        PLOG("\nAt time %i: Simulation at premature end. Last agent would have died.",t);
        ABORT2
      }
    } else {
      others_alive = true;
    }
  }
RESULT(dead)

EQUATION("Pop_birth")
/* Calculates the number of agents that are born at any given time. Fractionals
are carried over to the next time, until a "complete" additional baby results.*/
TRACK_SEQUENCE
  double carryover = CURRENT - floor(CURRENT); //carry over fractional birth's from previous times.
  double n_base = V("Pop_const_n");

  if (n_base == 0){
    //the population model is not used to determine new births
    PARAMETER
    END_EQUATION(0);
  }

  if (n_base < 0){
    //(partly) endogenous: use number of agents alive as base
    n_base = P_EXT(ext_pop)->byAge_agents_alive.size(); //number of living agents
  }

  double newborn = carryover + V("Pop_birth_rate") * n_base; //the number of newborn (round down)

  /* General spawn process */
  for (int i=0; i< newborn; i++){
    V_CHEAT("Pop_agent_birth",p->up); //use create-and-initialise mode
  }
  /*-----------------------*/

  INCR("Pop_age",newborn);

RESULT(newborn)

EQUATION("Pop_age")
/* Each agents get older one year. */
TRACK_SEQUENCE
  double alive = 0;
  CYCLES(p->up,cur,P_EXT(ext_pop)->agent_label){
    alive++;
    INCRS(cur,GET_VAR_LABEL(cur,"_age"),1.0);
  }
  P_EXT(ext_pop)->agents_alive_get_older();

  TEST_IN(false && t<5) //protocoll ok
    ext_pop_agent* pAgentExt;
    cur=P_EXT(ext_pop)->getRandomAgent(2,t,t+2); //male
    PLOG("\nTest: Random agent should have gender %s and age between %i and %i","male",t,t+2);
    if (cur==NULL){
      PLOG("\nTest: No such agent exists");
    } else {
      PLOG("\nTest: Returned agent: ID %g  gender %s and age %g",GET_ID(cur),GET_VAR(cur,"_female")==1?"female":"male",GET_VAR(cur,"_age"));
      pAgentExt = P_EXT(ext_pop)->getAgentExt(GET_ID(cur), "true");
      PLOG("\nTest: Returned agent Ext: ID %i  gender %s and age %i",pAgentExt->ID,pAgentExt->female==1?"female":"male",pAgentExt->age);
    }

    cur=P_EXT(ext_pop)->getRandomAgent(1,t+10,t+20); //female
    PLOG("\nTest: Random agent should have gender %s and age between %i and %i","female",t,t+2);
    if (cur==NULL){
      PLOG("\nTest: No such agent exists");
    } else {
      PLOG("\nTest: Returned agent: ID %g  gender %s and age %g",GET_ID(cur),GET_VAR(cur,"_female")==1?"female":"male",GET_VAR(cur,"_age"));
      pAgentExt = P_EXT(ext_pop)->getAgentExt(GET_ID(cur), "true");
      PLOG("\nTest: Returned agent Ext: ID %i  gender %s and age %i",pAgentExt->ID,pAgentExt->female==1?"female":"male",pAgentExt->age);
    }
    cur=P_EXT(ext_pop)->getRandomAgent(0,t+30,t+30); //doesn't matter
    PLOG("\nTest: Random agent should have gender %s and age between %i and %i","unspecified",t,t+2);
    if (cur==NULL){
      PLOG("\nTest: No such agent exists");
    } else {
      PLOG("\nTest: Returned agent: ID %g  gender %s and age %g",GET_ID(cur),GET_VAR(cur,"_female")==1?"female":"male",GET_VAR(cur,"_age"));
      pAgentExt = P_EXT(ext_pop)->getAgentExt(GET_ID(cur), "true");
      PLOG("\nTest: Returned agent Ext: ID %i  gender %s and age %i",pAgentExt->ID,pAgentExt->female==1?"female":"male",pAgentExt->age);
    }
  TEST_OUT

RESULT(alive)

EQUATION("Pop_birth_rate")
/* If defined as parameter, this is the fixed birth rate. If defined as variable,
it is the constant population size birth rate (see [2]). Note that in order to
guarantee a statistically constant population size, the parameter Pop_const_n
needs to be fixed to the target population size.
*/
TRACK_SEQUENCE

double birth_rate = 1.0 / P_EXT(ext_pop)->expected_death; //Important: Module needs to be initialised!

                              VERBOSE_IN(true)
                              PLOG("\nConstant population birth_rate is %g",birth_rate);
                              VERBOSE_OUT

PARAMETER
RESULT(birth_rate)

