#include <random>    //else potential problems with _abs()
#include "fun_head_fast.h"

/********************
 *   Define which Modules to use
 *   ***************/

#define MODULE_ABMAT
#define MODULE_POPULATION
#define MODULE_GEOGRAPHY
#define MODULE_PAJEK
/********************
 *   Change / Set settings for modules
 *   ***************/

  //ABMAT
#define ABMAT_ANALYSIS_CFG_PATH "Config/ABMAT_Analysis.cfg"
#define ABMAT_INTERVALS_CFG_PATH "Config/ABMAT_Intervals.cfg"

  //tools
// #define SWITCH_TEST_OFF
// #define SWITCH_VERBOSE_OFF  //(un)comment to switch on(off)
#define TRACK_SEQUENCE_MAX_T 5 //number of steps for tracking

#include "Advanced_LSD/Advanced_LSD.h" //include modules
/* -------------------------------------------------------------------------- */

MODELBEGIN

#include "Advanced_LSD/fun_Advanced_LSD.h" //Equation files

/*----------------------------------------------------------------------------*/
/*           General Helpers                                                  */
/*----------------------------------------------------------------------------*/

////////////////////////
EQUATION("Updating_Scheme")
/* Controls the flow of events at any single simulation tick. This should always
   be the first equation called in any model! */
TRACK_SEQUENCE

  if (t==1){
    V("Init_Global");
  }

  /* Before all else, the population is updated */
  #ifdef MODULE_POPULATION
    V("Pop_death");
    V("Pop_age");
    V("Pop_birth");   //If the population model generates new agents, that happens here.
  #endif

  V("TestAction");

  #ifdef MODULE_PAJEK
    V("Update_Pajek_Network");
  #endif

  #ifdef ABMAT_USE_ANALYSIS
    V("ABMAT_UPDATE"); //Take descriptive statistics
  #endif
RESULT(0.0)

////////////////////////
EQUATION("Init_Global")
/* This it the main initialisation function, calling all initialisation action
necessary. */
TRACK_SEQUENCE

  int initial_n_agents = V("Pop_const_n"); //for this example, use the target n as initial n
  ADDNOBJ("Agent",initial_n_agents-1); //add number of agents such that the constant target is met.

  #ifdef MODULE_PAJEK
    PAJEK_INIT;
    PAJEK_INIT_NEW_RELATION("Acquaintance",false);
  #endif

  #ifdef ABMAT_USE_ANALYSIS
    V("ABMAT_INIT"); //Initialise ABMAT
  #endif
  /* Backend Modules */
  #ifdef MODULE_POPULATION
    cur = SEARCH("Agent"); //we need to define the Agent object to which the backend connects.
    V_CHEAT("Pop_init",cur); //Initialise the population model
  #endif

  #ifdef MODULE_GEOGRAPHY
    cur = SEARCH("Patch"); //we need to define the patch object to which the backend connects.
    V_CHEAT("Gis_Init",cur); //and then we initialise it.
  #endif

  /* Note: Currently only one type of each is possible. But one may modify it
    easily to allow for more instances...
  */

PARAMETER
RESULT(0)

/******************************************************************************/
/*           Population modul linking - see description.txt                   */
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

////////////////////////
EQUATION("Init_New_Agent")
/* This EQUATION is called via fake_caller and initialises the new agent.
   It is called from the population modul, whenever a new agent is created
   (after it has been added).
   The user can provide content for the new agent (caller pointer c) */
TRACK_SEQUENCE

   WRITES(c,"Agent_type",uniform(0,1));
   WRITES(c,"Agent_family",uniform(0,1));



   //WRITES("father",ID); //if there shall be a father, provide the ID here.
   //WRITES("mother",ID); //if there shall be a mother, provide the ID here.

   //If it shall be linked to the parents, do so now
   //If it shall be linked to sibblings, do so now

RESULT(0)


////////////////////////
FUNCTION("Delete_Agent")
/* This FUNCTION is called via fake_caller when an agent dies.
   It is called from the population modul, whenever an agent dies (before it
   is removed).
   The user can provide content for the dying agent (caller pointer c) */
TRACK_SEQUENCE

  //Cut all links to living people (?)
  CYCLE_SAFES(c,cur,"Acquaintance"){
    V_CHEATS(c,"delete_acquaintance",cur);
  }

RESULT(0)

/******************************************************************************/
/*  Population modul linking end                                              */
/*----------------------------------------------------------------------------*/

/* Theory specific below */

FUNCTION("family_distance")
/*Reports the family distance between the callee and the agent called.
  Needs to be called from another agent (or fake caller)*/
TRACK_SEQUENCE

  double unnorm_distance = abs(V("Agent_family") - VS(c->hook->up,"Agent_family"));
  double norm_distance = 1 - 2* min( unnorm_distance, 1-unnorm_distance);

RESULT(norm_distance)

FUNCTION("type_distance")
/*Reports the type distance between the calle and the agent called.
  Needs to be called from another agent (or via fake caller) */
TRACK_SEQUENCE

  double type_distance = 1 - exp(- abs(V("Agent_type") - VS(c->hook->up,"Agent_type") ) );

RESULT(type_distance)

FUNCTION("delete_acquaintance")
/* Cuts a link between to agents, mutually deleting the acquaintance objects.
  Called via fake-caller, callee being the Acquaintance object at p.
*/
TRACK_SEQUENCE

  //Check if we do not have the blue-print
  if (VS(c,"Acquaintance_ID") == -1){
//     PLOG("\nInfo  :: delete_acquaintance : Selected blueprint, skipping.");
    END_EQUATION(0);
  }



  //Check not necessary, for upon deleting an agent we will first cut all links.
  //If we ever change this, we need to fix things here!

  //Check if the acquainted agent lives, if yes, delete acquaintance object at
  //acquainted agent first.
//   cur =   P_EXTS(SEARCHS(root,"Pop_Model"),ext_pop)->getAgent((int)VS(c,"Acquaintance_ID"),true);
//  if (POP_GET_AGENT(VS(c,"Acquaintance_ID")) != NULL){
   INCRS(c->hook->up,"Agent_degree",-1);
   DELETE(c->hook);
//  }

    INCRS(c->up,"Agent_degree",-1);
    DELETE(c);

RESULT(0)

FUNCTION("new_acquaintance")
/* Produces a new acquaintance. Called via fake-caller - the callee is the
  accquaintanted agent, linked to p.
  Also creates the object at the partner.
*/
TRACK_SEQUENCE

  //for p
  double c_id = VS(c,"Agent_ID");

    /* May be skipped for performance later */
    //check if not self
    if (c==p){
     // PLOG("\nInfo :: new_acquaintance : selected self. skipped.");
      END_EQUATION(0);
    }
    //check if it does not yet exist
    CYCLE(cur,"Acquaintance"){
      if (VS(cur,"Acquaintance_ID")==c_id) {
       // PLOG("\nInfo :: new_acquaintance : Already known, update only");
        V_CHEAT("update_info",cur);
        END_EQUATION(0); //End prematurely, the Acquaintance exists already
      }
    }

  object *p_newAcq = ADDOBJ("Acquaintance");
  WRITES(p_newAcq,"Acquaintance_ID",c_id);

  //for the c partner
  double p_id = V("Agent_ID");
  object *c_newAcq = ADDOBJS(c,"Acquaintance");
  WRITES(c_newAcq,"Acquaintance_ID",p_id);

  //link the Acquaintance objects mutually.
  p_newAcq->hook=c_newAcq;
  c_newAcq->hook=p_newAcq;

  //update information, always mutually
  V_CHEAT("update_info",p_newAcq);

  //Increase degree for both
  INCR("Agent_degree",1);
  INCRS(c,"Agent_degree",1);

RESULT(0)

FUNCTION("update_info")
/* Whenever two agents interact, the information of their distances is updated
  for both interaction partners. If the acquaintance is dead, the acquaintance
  object is removed. (i.e. without interaction links are allowed to
  persist beyond death).

  This function is called via fake-caller, with the callee being the
  acquaintance object at p (the agent starting the interaction).
*/
TRACK_SEQUENCE
  object *p_Acq = c;

  if (VS(p_Acq,"Acquaintance_ID")==-1){
    END_EQUATION(0);
    //This is the template for technical reasons only. It is not associated to
    //any real agent.
  }

  /*
  cur = SEARCHS(root,"Pop_Model");
  object *cur_Acq = POP_GET_AGENTS(cur,VS(p_Acq,"Acquaintance_ID"));

  if (cur_Acq == NULL) { //The Acquaintance is dead!
    DELETE(c);
    END_EQUATION(0);
  }
  */ //not necessary - upon dead we cut the links. But if this changes, we need to check!

  //If distances shall not change, we only update this information when a
  //new acquaintance is set.

  double family_distance = V_CHEAT("family_distance",p_Acq);
  double type_distance =   V_CHEAT("type_distance",p_Acq);

  WRITES(p_Acq,"Acquaintance_fd",family_distance);
  WRITES(p_Acq->hook,"Acquaintance_fd",family_distance);

  WRITES(p_Acq,"Acquaintance_td",type_distance);
  WRITES(p_Acq->hook,"Acquaintance_td",type_distance);

RESULT(double(t))

EQUATION("update_network")
/* Currently doing the net*/
TRACK_SEQUENCE

int age = V("Agent_age");
int ageOfMaturaty = V("ageOfMaturaty");
if (age == 0) {
   //Add links to family? Normally this is better done at
   // "Init_New_Agent".

} else if (age ==  ageOfMaturaty){
  //create initial links (besides family)
    for (int i = 0; i<5;i++){
      cur = RNDDRAW_FAIRS(p->up,"Agent");
      V_CHEAT("new_acquaintance",cur);
    }
} else if (age > ageOfMaturaty) {
  //normal process of change of links

      //TEST
    if (RND>0.1){
      cur = RNDDRAW_FAIRS(p->up,"Agent");
      V_CHEAT("new_acquaintance",cur);
    }
    if (RND>0.1){
      cur = RNDDRAW_FAIRS(p,"Acquaintance");
      V_CHEAT("delete_acquaintance",cur);
    }

}

RESULT(0)

EQUATION("TestAction")
/* A function on agent level to test some stuff */
TRACK_SEQUENCE

if (t == 1){
  i = 0;
  PLOG("\n Testing Neighbours cycle, non-sorted.");
  GIS_CYCLE_NEIGHBOURS_SIMPLE(cur,0,0,3.0){ //Cycle through neighbours at pos 0.0 with radius 3.0
    PLOG("\n %i (%g,%g,%g)",++i,VS(cur,"Patch_x"),VS(cur,"Patch_y"),temp_gis_search_obj.last_distance);
    //INTERACTS(cur,"test",0.0);
  }
  i = 0;
  PLOG("\n Testing Neighbours cycle, sorted.");
  GIS_CYCLE_NEIGHBOURS_SORT(cur,0,0,3.0){ //Cycle through neighbours at pos 0.0 with radius 3.0
    PLOG("\n %i (%g,%g,%g)",++i,VS(cur,"Patch_x"),VS(cur,"Patch_y"),temp_gis_search_obj.last_distance);
    //INTERACTS(cur,"test",0.0);
  }

  PLOG("\n --------------------- 2nd run ----------------");

  i = 0;
  PLOG("\n Testing Neighbours cycle, non-sorted.");
  GIS_CYCLE_NEIGHBOURS_SIMPLE(cur,3,3,10.0){ //Cycle through neighbours at pos 0.0 with radius 3.0
    PLOG("\n %i (%g,%g,%g)",++i,VS(cur,"Patch_x"),VS(cur,"Patch_y"),temp_gis_search_obj.last_distance);
  }
   i = 0;
  PLOG("\n Testing Neighbours cycle, sorted.");
  GIS_CYCLE_NEIGHBOURS_SORT(cur,3,3,10.0){ //Cycle through neighbours at pos 0.0 with radius 3.0
    PLOG("\n %i (%g,%g,%g)",++i,VS(cur,"Patch_x"),VS(cur,"Patch_y"),temp_gis_search_obj.last_distance);
  }

  PLOG("\n \n ...... \n Pausing simulation ... (press run to resume) ");
  INTERACT("Pause",(double)t);

}

#ifdef MODULE_PAJEK

#endif

// PLOG("\n\n\nTESTESTEST");
// ext_gis_rsearch temp = ext_gis_rsearch(P_EXTS(SEARCH("GIS_Model"),ext_gis),0,0,3.0);
// PLOG("\n Size is %i",temp.valid_objects.size());

RESULT(double(i))

EQUATION("Update_Pajek_Network")
/* Update the network info for the pajek network */

  //Cycle through all vertices and add them
//   double ratio = 1.0/ (double) P_EXTS(SEARCH("Pop_Model"),ext_pop)->max_life;
  PAJEK_VERTICE_SPECIAL("Agent","Agent_ID","Agent_family",1); //factor for resize

  //Cycle through all agents and gather the links as archs (unidirectional)
  CYCLE(cur,"Agent"){
    int ID_p = VS(cur,"Agent_ID");
    CYCLES(cur,cur1,"Acquaintance"){
      int ID_a = VS(cur1,"Acquaintance_ID");
      if (ID_a > -1){
        PAJEK_ARC(ID_p,ID_a,"Acquaintance",VS(cur1,"Acquaintance_td"));
      }
    }
  }

  PAJEK_SNAPSHOT;

RESULT(0)







MODELEND




void close_sim(void)
{

}


