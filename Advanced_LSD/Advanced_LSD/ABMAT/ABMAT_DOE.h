#ifndef ABMAT_DOE_H //guard
#define ABMAT_DOE_H

#ifndef ABMAT_DPM_CFG_PATH
  #define ABMAT_DPM_CFG_PATH "DOE/ABMAT_DPM.cfg"
#endif

/*dependencies*/
#include <cmath>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iterator>
#include <sstream>
#include <limits>
#include <cstring>
#include <ctime>
//#include <direct.h>

/* For folder creation */
#include <sys/types.h> // required for stat.h
#include <sys/stat.h> // no clue why required -- man pages say so


namespace ABMAT //a general namespace for the ABMAT tools
{

/* ========================================================================== */

 /*********************
   *  The Tools to load the DOE 
   *  
   *********************/
  bool single_mode; //aggregate is default
  
  //Holding the path to the DPM and their names
  std::vector<std::string> DPM_paths;
  std::vector<std::string> DPM_names;
  std::vector<std::string> DPM_modes; 
  int DPM_count; // = 0;
  int DPM_choice; // = 0;  
  bool add_DPM(const std::string& Path, const std::string& Name, int checkID, const std::string& Mode="Aggregate");
//   #define ABMAT_DPM_ADD(a,b,c) ABMAT::add_DPM(a,b,c);
    
  //global variables for Parameterspace and Labels
  std::vector< std::vector<double> > Parameters;
  std::vector< std::tuple< int, bool > > Processed_cids; //This is a vector of tuples, first CID second true/false implying processed y/n.
  bool processed_obs_loaded=false; //tried to load?
  bool processed_obs_file=false; //did it work? 
  std::vector<std::string> Labels; 
  bool use_grabbed_pars; // = false;
  std::vector<double> Parameters_grabbed; //There might be some convers. in LSD.     
  bool DPM_already_loaded; // = false; //Make sure that it is only loaded once. Also used for add_DPM
  bool grab_Config(object * cur_r);

  bool load_DPM_cfg();
  bool load_DPM();

  
  int Config_loaded; //=0;
   
  bool load_Config(object * cur_r, int ConfigID);
  bool check_if_processed(int id);
  bool load_processed_files();
  
//   bool final_run = false;    //Report if it is the final simulation 
  int cur_set_size; //=1; //The size of the current set.
  int first_item_in_set;
  int last_item_in_set;
  int cur_item_in_set; //The cur_sim from LSD can be changed within, but for each new simulation it is re-set / controlled by an external loop.
  bool set_initialised;
    
  bool load_Config_via_Set(object * cur_r, int SetID, int SetNum);
  bool initialise_Set(object * cur_r, int SetID, int SetNum);



/* ========================================================================== */
} //Namespace end
#endif