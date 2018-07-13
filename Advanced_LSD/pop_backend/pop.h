/*************************************************************
                                                    May 2018
  LSD Population module - backend for LSD (least 7.0)
  written by Frederik Schaff, Ruhr-University Bochum

****************************************************/

#include <vector>
#include <tuple>
#include <algorithm>
#include <functional>




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
    int born = 0; //time of birth
    int age = 0;
    bool female = false; //gender, random on creation!
    bool alive = false;
    ext_pop_agent* father = NULL;
    ext_pop_agent* mother = NULL;
    std::vector <ext_pop_agent*> children;
    double death_age = -1; //Age of death


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
    int max_size_agents = 0; //current method of using pointers to elements in vector is unsave. Rais Error when necessary.
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
    std::vector< double > pop_init_age_dist(int n_agent);
    void agents_alive_get_older();
    double unconditional_survival_rate(int age);
    int total();
    ext_pop_agent* newAgent(object* LSD_Agent);
    double pop_hazard_rate(int cur_age);
    bool agentDies(int ID);
    ext_pop_agent* getAgentExt(int ID, bool alive = true);
    object* getAgent(int ID);

    /* gender: 0 = any, 1 = female, 2 = male */
    ext_pop_agent* getRandomAgentExt(int gender=0, bool alive = true);
    ext_pop_agent* getRandomAgentExtAliveAge(int gender=0, int min_age = -1, int max_age = -1);
    object* getRandomAgent(int gender=0, int min_age = -1, int max_age = -1);

    int family_degree(int id_mother, int id_father,  int max_tested_degree=-1); //returns the family degree from 0 (not tracked) over 1 (siblings) to ... see function, optional: define when to stop checking for kinship
    bool check_if_incest(int id_mother, int id_pot_father, int prohibited_degree=5);  //returns true if there is incest, default check all.


    void shuffle_random_agents();
    void expected_total(int n);


};




