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


  The package relies on LSD debug module by
    F. Schaff. For further informations see: ...

 *************************************************************/

/***************************************************
backend_pop.h

This file contains the declarations and new macros for the population model.
It also includes the other core-code files (all not fun_*)
****************************************************/

#include <vector>
#include <tuple>
#include <algorithm>
#include <functional>
//#include <random>    //needs to be before fun_head...? - problems with abs() def.



//I'd like to have the pop_prng in class scope. How?

auto pop_prng = std::mt19937 {}; //own prng which can be used with all the c++11 stuff
//std::uniform_real_distribution<> POP_UNIFORM(0.0,1.0);    //usage: POP_UNIFORM(pop_prng)
auto pop_uniform = std::bind(std::uniform_real_distribution<>(0.0,1.0),
                           pop_prng);

class ext_pop;
struct ext_pop_agent;


struct ext_pop_agent {
  public:
    /*generic*/
    object* LSD_counterpart = NULL; //Link to LSD object holding this external object

    /*specific*/
    int ID = 0;
    int age = 0;
    bool female; //gender, random on creation!
    bool alive = false;
    ext_pop_agent* father = NULL;
    ext_pop_agent* mother = NULL;
    std::vector <ext_pop_agent*> children;
    double death_age; //Age of death

    //perhaps in the future... for now everything via LSD to keep the package light
//     #ifdef MODULE_GEOGRAPHY
//       ext_gis_patch* ext_location = NULL; //link to patch at gis level
//              object* lsd_location = NULL; //link to patch as LSD object
//       ext_gis* gis_counterpart = NULL; //link to the gis class instance
//
//     #endif

    //to do: Pointer to own pos. in randomised and by age queue, to improve performance?
};

/* An iterator type to get the next agent in a vector of agents.
  See: https://stackoverflow.com/a/27307613/3895476
*/
struct ext_pop_agent_it : public std::vector<ext_pop_agent*>::iterator {
    using std::vector<ext_pop_agent*>::iterator::iterator;
    object* operator*() { return std::vector<ext_pop_agent*>::iterator::operator*()->LSD_counterpart; }
};

class ext_pop {
  public:
    /*generic*/
    object* LSD_counterpart = NULL; //Link to LSD object holding this external object
    char agent_label[ MAX_ELEM_LENGTH ]; //the label of the LSD object, e.g. "Agent" or "Agent_Type_A"

    /* specific*/

    //For the agents alive and dead
    std::vector<ext_pop_agent> agents; //List of agents (alive?, ID, pointer to ext_agent)
    std::vector<ext_pop_agent*> random_agents_alive; //List of all agents alive, can be randomised.
    std::vector<ext_pop_agent*> byAge_agents_alive; //List of all agents alive, decreasing in age.
    ext_pop_agent_it it_random_agents_alive;
    void mother_and_child(int mother_ID, int child_ID);
    void father_and_child(int father_ID, int child_ID);

    //Statistical Population Model
    double survival_function(int age);
    std::vector <double> survival_rate; //Unconditional survival probability of surviving age x. Index = age.
    double cum_survival_rate;
    double alpha;
    double beta;
    int max_life;    //Maximum possible life. The inverse is equal to the constant pop. birth rate.
    double expected_death=0.0; //this way, an error is thrown if used uninitialised.
    int n_const; //is there a statistically constant number of agents?

    void pop_init(object* counterpart, char const *_agent_label, int seed, double _alpha, double _beta, int _n_const);
    void pop_survival_init();
    int pop_newborn_death_age();
    std::vector< double > init_pop_age;
    std::vector< double > &pop_init_age_dist();
    void agents_alive_get_older();
    double unconditional_survival_rate(int age);
    int total();
    ext_pop_agent* newAgent(object* LSD_Agent);
    double pop_hazard_rate(int cur_age);
    bool agentDies(int ID);
    ext_pop_agent* getAgentExt(int ID, bool alive = true);
    object* getAgent(int ID, bool alive = true);

    /* gender: 0 = any, 1 = female, 2 = male */
    ext_pop_agent* getRandomAgentExt(int gender=0, bool alive = true);
    ext_pop_agent* getRandomAgentExtAliveAge(int gender=0, int min_age = -1, int max_age = -1);
    object* getRandomAgent(int gender=0, int min_age = -1, int max_age = -1);


    void shuffle_random_agents();
    void expected_total(int n);


};

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
#define POP_GET_AGENTS(ext_obj,ID) P_EXTS(ext_obj,ext_pop)->getAgent((int)ID,true)
#define POP_GET_AGENT(ID) POP_GET_AGENTS(SEARCHS(root,"Pop_Model"),ID)

//Return pointer to living agents ext_pop_agent record or NULL if it is dead / does not exist
#define POP_GET_AGENT_EXTS(ext_obj,ID) P_EXTS(ext_obj,ext_pop)->getAgentExt(ID,true)
#define POP_GET_AGENT_EXT(ID) POP_GET_AGENT_EXTS(SEARCHS(root,"Pop_Model"),ID)

//Return random agents in live intervall and with gender as specified

  //Multipurpose
#define POP_GET_RAGENTAXS(ext_obj,gender,min_age,max_age) \
P_EXTS(ext_obj,ext_pop)->getRandomAgent(0, (int) min_age, (int) max_age)
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

