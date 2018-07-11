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
backend_pop.cpp

This file contains the core code of the population backend.
****************************************************/

#include "pop.h"

  double ext_pop::unconditional_survival_rate(int age){
    /* The unconditional survival probability. */
    if (age>=0 && age<survival_rate.size()){
      return survival_rate.at(age);
    } else {
      PLOG("\nPopulation Model :   Error in unconditional_survival_rate(). Age %i outside of range 0,%i",age,survival_rate.size()-1);
      //NOPE, not an error. It is possible to survive at max-age and then die.
      return 0.0; //no chance to survive.
    }
  }

  void ext_pop::pop_init(object* counterpart, char const *_agent_label, int seed, double _alpha, double _beta, int _n_const){
    snprintf(agent_label,	MAX_ELEM_LENGTH, "%s",_agent_label); //copy the label
    pop_prng.seed(seed); //using provided seed.
    pop_prng.discard(800000); //discard first 800k to get rid of warm-up.
    LSD_counterpart=counterpart;
//    agent_parent_object=agent_parent;
    alpha=_alpha;
    beta=_beta;
    n_const=_n_const;
    pop_survival_init();

          VERBOSE_IN(true)
            PLOG("\nPopulation Model :   Initialising population of agents '%s'",agent_label);
            PLOG("\nPopulation Model :   Expected death with age %g",expected_death);
            PLOG("\nPopulation Model :   Max age %i",max_life);
            if (n_const < 0){
              PLOG("\nPopulation Model :   Endogeneous new generation size (birth_rate * current n agents)");
            } else if (n_const == 0) {
              PLOG("\nPopulation Model :   Birth's not controlled by pop. model");
            } else {
              PLOG("\nPopulation Model :   Fixed birth rate (birth_rate * n_const (%i))",n_const);
            }

          VERBOSE_OUT
  }

  int ext_pop::total(){
    return agents.size();
  }

  ext_pop_agent* ext_pop::newAgent(object* LSD_Agent){ //create a new agent card, link the card to the agent, return the ID of the agent.
    ext_pop_agent new_agent;
    new_agent.ID = agents.size();
    new_agent.alive = true;
    new_agent.female = pop_uniform()<.5; //random male or female.
    new_agent.LSD_counterpart=LSD_Agent;
    new_agent.born = t;

    new_agent.death_age = pop_newborn_death_age();

    agents.push_back(new_agent);
    random_agents_alive.push_back(&agents.back()); //add pointer to new agent.
    byAge_agents_alive.push_back(&agents.back()); //add pointer to new agent.
    return &agents.back();
  }

  /* In the discrete case: h(i)=1- S(i+1)/S(i) */
  double ext_pop::pop_hazard_rate(int cur_age){ //calculate the chance to die, cond. on having survived until now.
    if (survival_rate.size() - 1 > cur_age) {
      double _now = unconditional_survival_rate(cur_age);
      double _next = unconditional_survival_rate(cur_age+1);
      double _cond_hazard = 1 - _next/_now; //chance of surviving current year, conditional on survival of prior.
                                  VERBOSE_IN(false)
                                    PLOG("\nPopulation Model :   Unconditional survival at cur age %i is %g. hazard rate is %g",cur_age, _next ,  _cond_hazard);
                                  VERBOSE_OUT
      return  _cond_hazard ;
    } else {
      return 1.0; //sure death
    }
  }



  bool ext_pop::agentDies(int ID){ //mark agent as dead.
    if (agents.size() > ID && ID >= 0){
      TEST_IN(agents.at(ID).alive == false)
        PLOG("\nError! Population Model :   ext_pop::agentDies: Agent with ID %i is already dead?",ID);
      TEST_OUT
      agents.at(ID).LSD_counterpart=NULL;
      agents.at(ID).alive=false;

      //remove the agent from the list of agents alive with (pot.) random order
      //https://stackoverflow.com/a/26567766/3895476
      {
        VERBOSE_IN(true)
          PLOG("\nPopulation Model :   : ext_pop::agentDies : Currently there are %i agents alive. Now removing one with ID %i age %i from random list.",random_agents_alive.size(),ID,agents.at(ID).age);
        VERBOSE_OUT
        auto it = std::find(random_agents_alive.begin(), random_agents_alive.end(), &agents.at(ID));
        if (it != random_agents_alive.end()) {
          VERBOSE_IN(true)
            PLOG("\n\t... success");
          VERBOSE_OUT
          random_agents_alive.erase(it);
        } else {
          PLOG("\nERROR Population Model :   : ext_pop::agentDies : could not find the target in random-list!");
          return false;
        }
      }

      //https://stackoverflow.com/a/26567766/3895476
      {
        VERBOSE_IN(true)
          PLOG("\nPopulation Model :   : ext_pop::agentDies : Currently there are %i agents alive. Now removing one with ID %i age %i from sorted (by age) list",byAge_agents_alive.size(),ID,agents.at(ID).age);
        VERBOSE_OUT
        auto it = std::find(byAge_agents_alive.begin(), byAge_agents_alive.end(), &agents.at(ID));
        if (it != byAge_agents_alive.end()) {
          VERBOSE_IN(true)
            PLOG("\n\t... success");
          VERBOSE_OUT
          byAge_agents_alive.erase(it);
        } else {
          PLOG("\nERROR Population Model :   : ext_pop::agentDies : could not find the target in by-age list!");
          return false;
        }
      }


//       for (auto it = std::begin(byAge_agents_alive); it!=std::end(byAge_agents_alive); ){
//         if (*it == &agents.at(ID) ){
//           //PLOG("\nPopulation Model :   Removing agent with ID %i == %i",agents.at(ID).ID,*it.ID);
//           it = byAge_agents_alive.erase(it);
//         } else {
//           ++it;
//         }
//       }


      return true;
    } else {
      PLOG("\nError! Population Model :   Search for agent %i -to be killed- not allowed. Only %i agents created so far.",ID,total());
      return false;
    }
  }

  //Check if there is potential of incest with given "degree" - only direct biology.
  // allowed degree: 0 - incest allowed, 1 - siblings, 2 - also parent-child, 3 - also grandchild-grandparent, 4 - also nephew - uncle/aunt, 5 - also cousin - cousin.
  // we compare be-directional, to also catch if a mother would have a child with a (grand)child.
  //returns true if there is incest
  bool ext_pop::check_if_incest(int id_mother, int id_father, int prohibited_degree){
    if (prohibited_degree == 0){
      return false; //no prohibited incest.
    }
    ext_pop_agent* c_mother = &agents.at(id_mother);
    ext_pop_agent* c_father = &agents.at(id_father);

    if (c_mother == NULL || c_father == NULL){
      PLOG("\nPopulation Model :   : ext_pop::check_if_incest : Error? mother or father does not exist. No incest.");
      return false;
    }

    //(half-)siblings?
    if (c_mother->mother != NULL
        && (c_mother->mother == c_father->mother || c_mother->father == c_father->father)
      ){
      VERBOSE_IN(true)
        PLOG("\nPopulation Model :   : ext_pop::check_if_incest : siblings!")
      VERBOSE_OUT
      return true;

    } else if (prohibited_degree == 1){
      return false; //OK

    //parent-child?
    } else if ( (c_mother->father != NULL && c_mother->father == c_father)
             || (c_father->mother != NULL && c_father->mother == c_mother)
      ){
      VERBOSE_IN(true)
        PLOG("\nPopulation Model :   : ext_pop::check_if_incest : parent-child relation!")
      VERBOSE_OUT
      return true;
    } else if (prohibited_degree == 2 ){
      return false; //OK

    //grandchild-grandparent?
    } else if (
      (/* grandchild-grandpas */ (c_mother->father != NULL && c_mother->father->father != NULL && c_mother->father->father == c_father)
                              || (c_mother->mother != NULL && c_mother->mother->father != NULL && c_mother->mother->father == c_father)
      ) || (
       /*grandson-grandmas */  (c_father->father != NULL && c_father->father->mother != NULL && c_father->father->mother == c_mother)
                            || (c_father->mother != NULL && c_father->mother->mother != NULL && c_father->mother->mother == c_mother)
      )
    ){
      VERBOSE_IN(true)
        PLOG("\nPopulation Model :   : ext_pop::check_if_incest : grandparent-grandchild relation!")
      VERBOSE_OUT
      return true;
    } else if (prohibited_degree == 3){
      return false; //OK

    //Aunt/Uncle - Nephew
    } else if (
       /*niece-uncles*/ (c_mother->father != NULL && c_mother->father->father != NULL && c_father->father != NULL && c_father->father == c_mother->father->father)
                    ||  (c_mother->father != NULL && c_mother->father->mother != NULL && c_father->mother != NULL && c_father->mother == c_mother->father->mother)
                    ||  (c_mother->mother != NULL && c_mother->mother->father != NULL && c_father->father != NULL && c_father->father == c_mother->mother->father)
                    ||  (c_mother->mother != NULL && c_mother->mother->mother != NULL && c_father->mother != NULL && c_father->mother == c_mother->mother->mother)
   /*aunt-nephews*/ ||  (c_mother->father != NULL && c_father->father != NULL && c_father->father->father != NULL && c_mother->father == c_father->father->father)
                    ||  (c_mother->father != NULL && c_father->mother != NULL && c_father->mother->father != NULL && c_mother->father == c_father->mother->father)
                    ||  (c_mother->mother != NULL && c_father->father != NULL && c_father->father->mother != NULL && c_mother->mother == c_father->father->mother)
                    ||  (c_mother->mother != NULL && c_father->mother != NULL && c_father->mother->mother != NULL && c_mother->mother == c_father->mother->mother)
    ){
      VERBOSE_IN(true)
        PLOG("\nPopulation Model :   : ext_pop::check_if_incest : niece-uncle or aunt-nephew relation!")
      VERBOSE_OUT
      return true;

      //Cousins
    } else if (
       /*cousins*/      (c_mother->father != NULL && c_mother->father->father != NULL && c_father->father != NULL && c_father->father->father != NULL && c_father->father->father == c_mother->father->father)
                    ||  (c_mother->father != NULL && c_mother->father->mother != NULL && c_father->father != NULL && c_father->father->mother != NULL && c_father->father->mother == c_mother->father->mother)
                    ||  (c_mother->mother != NULL && c_mother->mother->father != NULL && c_father->father != NULL && c_father->father->father != NULL && c_father->father->father == c_mother->mother->father)
                    ||  (c_mother->mother != NULL && c_mother->mother->mother != NULL && c_father->father != NULL && c_father->father->mother != NULL && c_father->father->mother == c_mother->mother->mother)
       /*         */||  (c_mother->father != NULL && c_mother->father->father != NULL && c_father->mother != NULL && c_father->mother->father != NULL && c_father->mother->father == c_mother->father->father)
                    ||  (c_mother->father != NULL && c_mother->father->mother != NULL && c_father->mother != NULL && c_father->mother->mother != NULL && c_father->mother->mother == c_mother->father->mother)
                    ||  (c_mother->mother != NULL && c_mother->mother->father != NULL && c_father->mother != NULL && c_father->mother->father != NULL && c_father->mother->father == c_mother->mother->father)
                    ||  (c_mother->mother != NULL && c_mother->mother->mother != NULL && c_father->mother != NULL && c_father->mother->mother != NULL && c_father->mother->mother == c_mother->mother->mother)
    ){
      VERBOSE_IN(true)
        PLOG("\nPopulation Model :   : ext_pop::check_if_incest : cousins' relation!")
      VERBOSE_OUT
      return true;

    } else {
      return false; //all fine
    }
  }

  void ext_pop::agents_alive_get_older(){
    for (auto pTemp : byAge_agents_alive){
      pTemp->age++;
    }
  }

  ext_pop_agent *ext_pop::getAgentExt(int ID, bool alive){
    if (agents.size() > ID && ID >= 0){
      ext_pop_agent *pTemp = &agents.at(ID);
      if (!alive || pTemp->alive){
        return pTemp;
      } else {
        PLOG("\nERROR: Population Model :   getAgentExt(): Search for agent %i (alive) - agent is dead!.",ID);
        return NULL;
      }
    } else {
      PLOG("\nERROR: Population Model :   getAgentExt(): Search for agent %i -to be found- not allowed. Only %i agents created so far.",ID,total());
      return NULL;
    }
  }

  object *ext_pop::getAgent(int ID, bool alive){
    if (agents.size() > ID && ID >= 0){
      ext_pop_agent *pTemp = &agents.at(ID);
      if (!alive || pTemp->alive){
        return pTemp->LSD_counterpart;
      } else {
        PLOG("\nPopulation Model :   getAgent(): Search for agent %i (alive) - agent is dead!.",ID);
        return NULL;
      }
      return pTemp->LSD_counterpart;
    } else {
      PLOG("\nERROR! Population Model :   getAgent(): Search for agent %i -to be found- not allowed. Only %i agents created so far.",ID,total());
      return NULL;
    }
  }

  ext_pop_agent *ext_pop::getRandomAgentExt(int gender, bool alive){
  /* 0 = any, 1 = female, 2 = male */
    ext_pop_agent *pTemp = NULL;
    int indx = 0;
    int temp_gender = 0;
    if (alive) {
      for (int safeguard = 0; safeguard < 1000; safeguard++) {
        indx = pop_uniform() * random_agents_alive.size();
        pTemp = random_agents_alive.at(indx);
        if (gender == 0){
          return pTemp;
        } else {
          int temp_gender = (pTemp->female?1:2);
          if (temp_gender == gender) {
            return pTemp;
          } //else continue!
        }
      }
    } else {
      for (int safeguard = 0; safeguard < 1000; safeguard++) {
        indx = pop_uniform() * agents.size();
        pTemp = &agents.at(indx);
        if (gender == 0){
          return pTemp;
        } else {
          int temp_gender = (pTemp->female?1:2);
          if (temp_gender == gender) {
            return pTemp;
          } //else continue!
        }
      }
    }
    return NULL;
  }

  object *ext_pop::getRandomAgent(int gender, int min_age, int max_age){
  /* 0 = any, 1 = female, 2 = male */
    if (min_age==max_age==-1){
      ext_pop_agent *pTemp = getRandomAgentExt(gender, true /*alive*/);
      if (pTemp == NULL){
        VERBOSE_IN(true)   //It can totally happen that there is no candidate
          PLOG("\npotERROR? Population Model :   getRandomAgent(): Error (?), see msgs before.");
        VERBOSE_OUT
        return NULL;
      } else {
        return pTemp->LSD_counterpart;
      }
    } else {
      ext_pop_agent *pTemp = getRandomAgentExtAliveAge(gender, min_age, max_age);
      if (pTemp == NULL){
        VERBOSE_IN(true)   //It can totally happen that there is no candidate
          PLOG("\npotERROR! Population Model :   getRandomAgent(): Error (?), see msgs before.");
        VERBOSE_OUT
        return NULL;
      } else {
        TEST_IN(pTemp->LSD_counterpart == NULL)
          PLOG("\nERROR! Population Model :   getRandomAgent(): counterpart to agent with ID %i is NULL?",pTemp->ID);
        TEST_OUT
        return pTemp->LSD_counterpart;
      }
    }
  }

  ext_pop_agent* ext_pop::getRandomAgentExtAliveAge(int gender, int min_age, int max_age){
  /* Find subset of agents with age, use sorted alive agent list. Then commence
    as in the version without AliveAge.
    To do: more efficient procedure if many calls. (and without chance to select twice the same)
  */
    VERBOSE_IN(true)
      PLOG("\n\nPopulation Model :   getRandomAgentExtAliveAge() called with gender %s, min_age %i and max_age %i",gender==1?"female":(gender==2?"male":"unspecified"),min_age,max_age);
    VERBOSE_OUT
    if (min_age > max_age){
        PLOG("\nPopulation Model :   getRandomAgentExtAliveAge(): Error, min_age %i > max_age %i.",min_age,max_age);
      return NULL;
    }
    int start = -1;   //oldest person allowed
    int end = -1;     //youngest person allowed
    for (int indx = 0; indx < byAge_agents_alive.size(); indx++) {
      TEST_IN(byAge_agents_alive.at(indx)==NULL)
        PLOG("\nERROR! Population Model :   getRandomAgentExtAliveAge(): at indx %i byAge_agents_alive points to NULL. byAge_agents_alive.size() is %i.",indx,byAge_agents_alive.size());
      TEST_OUT
      if (start == -1 && byAge_agents_alive.at(indx)->age <= max_age){
        start=indx; //first one that is not to old
      }
      if (byAge_agents_alive.at(indx)->age >= min_age){
        end = indx; //current last one meeting the criteria
      }
    }
    if (end == -1 || start == -1){
      VERBOSE_IN(true)
        PLOG("\nPopulation Model :   getRandomAgentExtAliveAge(): Could not find a suitable candidate.");
        if (byAge_agents_alive.size()>0) {
          PLOG("\n\t... Oldest is %i and youngest %i", byAge_agents_alive.front()->age,byAge_agents_alive.back()->age);
        } else {
          PLOG("\n\t... Error   - No agents alive??");
        }
      VERBOSE_OUT
      return NULL;
    }
    VERBOSE_IN(true)
      PLOG("\nPopulation Model :   getRandomAgentExtAliveAge(): Start: %i (age %i), end: %i (age %i)",
      start,byAge_agents_alive.at(start)->age,end,byAge_agents_alive.at(end)->age);
    VERBOSE_OUT
    TEST_IN(start == 0 || end == 0)
      PLOG("\nFire");
    TEST_OUT
    int indx;
    ext_pop_agent* pTemp = NULL;
    for (int safeguard = 0; safeguard < 1000; safeguard++) {
      indx = int (pop_uniform() * double(end-start+1) + start );
      TEST_IN(indx > byAge_agents_alive.size())
        PLOG("\nERROR! Population Model :   getRandomAgentExtAliveAge(): random draw outside of range!");
        return NULL;
      TEST_OUT
      pTemp = byAge_agents_alive.at(indx);
      if (gender == 0){
        VERBOSE_IN(true)
          PLOG("\nPopulation Model :   getRandomAgentExtAliveAge(): Found a candidate after %i trials. Indx %i, ID %i, age %i",
                safeguard,indx,byAge_agents_alive.at(indx)->ID,byAge_agents_alive.at(indx)->age);
        VERBOSE_OUT
        return pTemp;
      } else {
        int temp_gender = (pTemp->female?1:2);
        if (temp_gender == gender) {
          VERBOSE_IN(true)
            PLOG("\nPopulation Model :   getRandomAgentExtAliveAge(): Found a candidate after %i trials. Indx %i, ID %i, age %i",
                  safeguard,indx,byAge_agents_alive.at(indx)->ID,byAge_agents_alive.at(indx)->age);
          VERBOSE_OUT
          return pTemp;
        } //else continue!
      }
    }
        VERBOSE_IN(true)
          PLOG("\nERROR! Population Model :   getRandomAgentExtAliveAge(): Found no candidate after 1000 trials.");
        VERBOSE_OUT

    return NULL;
  }


  void ext_pop::mother_and_child(int mother_ID, int child_ID){
    ext_pop_agent *mother = getAgentExt(mother_ID,false);
    ext_pop_agent *child = getAgentExt(child_ID,false);

    if (child == NULL || mother == NULL){
      PLOG("\nPopulation Model :   mother_and_child(): Error. See prev. message(s). mother ID %i, child ID %i",mother_ID,child_ID);
    } else {
      mother->children.push_back(child);
      child->mother=mother;
    }
  }

  void ext_pop::father_and_child(int father_ID, int child_ID){
    ext_pop_agent *father = getAgentExt(father_ID);
    ext_pop_agent *child = getAgentExt(child_ID);

    if (child == NULL || father == NULL){
      PLOG("\nPopulation Model :   father_and_child(): Error. See prev. message(s). father ID %i, child ID %i",father_ID,child_ID);
    } else {
      father->children.push_back(child);
      child->father=father;
    }
  }

  void ext_pop::shuffle_random_agents(){
    std::shuffle(random_agents_alive.begin(),random_agents_alive.end(),pop_prng); //shuffle the whole series
  }

  /* optional*/

  void ext_pop::expected_total(int n){ //increase the vector size to its final capacity. This increases performance.
      agents.reserve(n);  //if already more space is allocated, nothing happens.
  }



  void ext_pop::pop_survival_init(){
    /* Initialise the statistical population model, i.e. the discrete ccdf
      aka survival/reliability function.
    */
    max_life =  int(- log(alpha) / beta) + 1.0; //+1.0 to get the ceil
    expected_death =  1.0 / beta + (alpha * log(alpha) )/ ( (1.0-alpha)*beta);

      //Here was an error! A certain part may not life "after" the age of zero.
      //survival_rate.push_back(1.0); //everybody survives age 0.
      //cum_survival_rate = 1.0;
    cum_survival_rate = 0.0;
    double c_age = 0.0;
    do {
      survival_rate.push_back( survival_function(++c_age) );
      cum_survival_rate += survival_rate.back();
    } while (survival_rate.back()>0.0);
  }

  double ext_pop::survival_function(int age){
    return (exp(-beta * age) - alpha)/(1-alpha);
  }

  int ext_pop::pop_newborn_death_age(){
    return int (- log( pop_uniform() * (1.0-alpha) + alpha ) / beta ); //from solving m(a) for a. +1 because you die AFTER having
  }

  std::vector<double> ext_pop::pop_init_age_dist(int n_agent){
    /* Provide a random initial age-structure that matches the long-run statistics
       0: initial age
       1: age of death
    */
    //random distribution
    std::vector<double> init_pop_age;
    init_pop_age.reserve(n_agent);
    //std::vector <std::pair<int,int> > age_out; age_out.reserve(n_agent);
    for (int i = 0; i< n_agent; i++){ init_pop_age.push_back(pop_uniform()); }

    std::sort(init_pop_age.begin(),init_pop_age.end() ); //sort ascending in age
    int cur_age = -1; //we start with the "youngest" people
    double cond_cdf = 0.0; //chance of beeing left of/below current age
    for (int i = 0; i < n_agent; i++){

      //The random number stored for age should indicate a death with just this age.
      //Hence, it should be between the survival rate for one yes lear of life-time
      //and the current life-time
      while (init_pop_age.at(i) > cond_cdf ){ //shift to current age class, if necessary
        cur_age++;
        cond_cdf+=survival_rate.at(cur_age)/cum_survival_rate;
      }
      init_pop_age.at(i)=cur_age;
//                                   VERBOSE_IN(i%int(n_agent/10)==0)
//                                   PLOG("\nPopulation Model :   Sorted agent %i has age %i and will die with age %i",i+1,age_out[i].first,age_out[i].second);
//                                   VERBOSE_OUT
    }
    VERBOSE_IN(init_pop_age.size()>10)
      PLOG("\nPopulation Model :   Max age: %i, size of survival CDF: %i",max_life, survival_rate.size());

      PLOG("\nPopulation Model :   pop_init_age_dist(): First 10 agents age in original order is:");
      for (int i = 0; i<10; i++){
        PLOG(" %g,",init_pop_age.at(i));
      }
    VERBOSE_OUT
    std::reverse(std::begin(init_pop_age), std::end(init_pop_age));  //make order decreasing in age.
    VERBOSE_IN(init_pop_age.size()>10)
      PLOG("\nPopulation Model :   pop_init_age_dist(): First 10 agents age in FINAL order is:");
      for (int i = 0; i<10; i++){
        PLOG(" %g,",init_pop_age.at(i));
      }
    VERBOSE_OUT
    return init_pop_age;
  }


