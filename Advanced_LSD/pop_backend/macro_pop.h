/* Some macros to simplify the usage of the population backend. */


/* pop_obj = object containing the Population Model
   pop_seed = seed for the pop prng
   pop_alpha = alpha value for the population model
   pop_beta = beta value for the population model
  */

#define POP_INITS(pop_obj,pop_agent,pop_seed,pop_alpha,pop_beta,pop_const_n)\
  ADDEXT2S(pop_obj,ext_pop); P_EXTS(pop_obj,ext_pop)->pop_init(pop_obj,pop_agent,pop_seed,pop_alpha,pop_beta,pop_const_n );
#define POP_INIT(pop_agent,pop_seed,pop_alpha,pop_beta,pop_const_n) POP_INITS(p,pop_agent,pop_seed,pop_alpha,pop_beta,pop_const_n)

// #define POP_SHUFFLE_AGENTS(x) P_EXTS(x,ext_pop)->shuffle_random_agents();
// #define POP_SHUFFLE_AGENT P_EXTS(p,ext_pop)->shuffle_random_agents();

/* x is the pointer to the object that holds ext_pop instance, i.e. "Pop_Model"
   y is the object point, i.e. cur or cur1
*/

/*  POP_RCYCLE_AGENTS(x,y)
  Init:
   a) Shuffle
   b) set iterator to first element
   c) set y to current agent pointer
  Check:
   a) We are not yet out of scope
  Do ({} part):
   a) IN LSD, work with y object
  Increment:
   a) increment iterator
   b) set y to current agent pointer
*/

#define POP_RCYCLE_AGENTS(ext_obj,y) \
  for (P_EXTS(ext_obj,ext_pop)->shuffle_random_agents(),  \
       P_EXTS(p->hook,ext_pop)->it_random_agents_alive=std::begin(P_EXTS(p->hook,ext_pop)->random_agents_alive), \
       y=*(P_EXTS(p->hook,ext_pop)->it_random_agents_alive); \
       P_EXTS(p->hook,ext_pop)->it_random_agents_alive!=std::end(P_EXTS(p->hook,ext_pop)->random_agents_alive); \
       P_EXTS(p->hook,ext_pop)->it_random_agents_alive++, \
       y=*(P_EXTS(p->hook,ext_pop)->it_random_agents_alive) )
//#define POP_RCYCLE_AGENT(y) POP_RCYCLE_AGENT(p,y)

//Return the pointer to the living agent with ID or NULL if it does not exist.
#define POP_GET_AGENTS(ext_obj,ID) P_EXTS(ext_obj,ext_pop)->getAgent((int)ID)
#define POP_GET_AGENT(ID) POP_GET_AGENTS(SEARCHS(root,"Pop_Model"),ID)

//Return pointer to living agents ext_pop_agent record or NULL if it is dead / does not exist
#define POP_GET_AGENT_EXTS(ext_obj,ID) P_EXTS(ext_obj,ext_pop)->getAgentExt(ID,true)
#define POP_GET_AGENT_EXT(ID) POP_GET_AGENT_EXTS(SEARCHS(root,"Pop_Model"),ID)

//Return random agents in live intervall and with gender as specified

  //Multipurpose
#define POP_GET_RAGENTAXS(ext_obj,gender,min_age,max_age) \
P_EXTS(ext_obj,ext_pop)->getRandomAgent(gender, (int) min_age, (int) max_age)
#define POP_GET_RAGENTAX(gender,min_age,max_age) POP_GET_RAGENTAXS(SEARCHS(root,"Pop_Model"),gender,min_age,max_age)


//Any gender
#define POP_GET_RAGENTAS(ext_obj,min_age,max_age) POP_GET_RAGENTXS(ext_obj,0,min_age,max_age)
#define POP_GET_RAGENTA(min_age,max_age) POP_GET_RAGENTX(0,min_age,max_age)
  //and any age
#define POP_GET_RAGENTS(ext_obj) POP_GET_RAGENTAXS(ext_obj,0,-1,-1)
#define POP_GET_RAGENT POP_GET_RAGENTAX(0,-1,-1)


//Female
#define POP_GET_RAGENTAFS(ext_obj,min_age,max_age) POP_GET_RAGENTAXS(ext_obj,1,min_age,max_age)
#define POP_GET_RAGENTAF(min_age,max_age) POP_GET_RAGENTAX(1,min_age,max_age)
  //and any age
#define POP_GET_RAGENTFS(ext_obj) POP_GET_RAGENTXS(ext_obj,1,-1,-1)
#define POP_GET_RAGENTF POP_GET_RAGENTX(1,-1,-1)

//Male
#define POP_GET_RAGENTAMS(ext_obj,min_age,max_age) POP_GET_RAGENTAXS(ext_obj,2,min_age,max_age)
#define POP_GET_RAGENTAM(min_age,max_age) POP_GET_RAGENTAX(2,min_age,max_age)
  //and any age
#define POP_GET_RAGENTMS(ext_obj) POP_GET_RAGENTAXS(ext_obj,2,-1,-1)
#define POP_GET_RAGENTM POP_GET_RAGENTAX(2,-1,-1)


//Check if relation would be forbidden kinship relation
#define POP_INCEST_CHECKXS(ext_obj,id_mother,id_father,degree) P_EXTS(ext_obj,ext_pop)->check_if_incest((int)id_mother,(int)id_father,degree)  //returns true if there is incest
#define POP_INCEST_CHECKX(id_mother,id_father,degree) POP_INCEST_CHECKXS(SEARCHS(root,"Pop_Model"),id_mother,id_father,degree)

#define POP_INCEST_CHECKS(ext_obj,id_mother,id_father) P_EXTS(ext_obj,ext_pop)->check_if_incest((int)id_mother,(int)id_father)  //returns true if there is incest
#define POP_INCEST_CHECK(id_mother,id_father) POP_INCEST_CHECKS(SEARCHS(root,"Pop_Model"),id_mother,id_father)

//Get information on the family degree
#define POP_FAMILY_DEGREES(ext_obj,agent1,agent2) P_EXTS(ext_obj,ext_pop)->family_degree(GET_ID(agent1),GET_ID(agent2));
#define POP_FAMILY_DEGREE(agent1,agent2) P_EXTS(SEARCHS(root,"Pop_Model"),ext_pop)->family_degree(GET_ID(agent1),GET_ID(agent2));


//A set of macros in combination with pajek

#ifdef MODULE_PAJEK
//to do: Make class-dependent
//Add "child" relation
  #define PAJEK_POP_LINEAGE_INIT \
    PAJEK_INIT_NEW_RELATION("child",false)

  //Cycle through all agents, alive or dead, and add them to the network.
  //Then cycle through all again and add links to children.
  //use as y the time of birth, as x the # at birth
  #define PAJEK_POP_LINEAGE_UPDATES(pop_obj)  \
    int x = 0;     \
    int last_born = 0;      \
    for (auto item : P_EXTS(pop_obj,ext_pop)->agents) {    \
      if (last_born < item.born){  \
        last_born = item.born;    \
        x = 0;   \
      } else {    \
        x++;      \
      }           \
      PAJEK_VERTICE_XP(item.ID,BACKEND_AGENT,item.age,item.female?8:7,item.alive?"Green":"Gray40",1.0+double(item.children.size()),1.0+2.0*double(item.age/double(P_EXTS(pop_obj,ext_pop)->max_life)),x,last_born);  \
    }        \
    for (auto item : P_EXTS(pop_obj,ext_pop)->agents) {      \
      for (auto child : item.children) {      \
        PAJEK_ARC_X(item.ID,child->ID,"child",item.alive?1:-1,1,"Black"); \
      }              \
    }

  #define PAJEK_POP_LINEAGE_UPDATE PAJEK_POP_LINEAGE_UPDATES(SEARCHS(root,"Pop_Model"))

#else
  #define PAJEK_POP_LINEAGE_INIT
  #define PAJEK_POP_LINEAGE_UPDATES(pop_obj)
  #define PAJEK_POP_LINEAGE_UPDATE
#endif


