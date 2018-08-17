/* ========================================================================== */
/*                                                                            */
/*   ABMAT.h                                                                  */
/*   (c) 2016 frederik.schaff@fernuni-hagen.de                                */
/*                                                                            */
/*   See the source for documentation                                         */
/*                                                                            */
/* ========================================================================== */

#ifndef ABMAT_H //guard
#define ABMAT_H   

/*****************************************************************************/

#ifndef _WIN32
//  #define SUPPRESS_SUB_FOLDERS         //do not create more subfolders than Top
#endif

#ifndef ABMAT_INTERVAL_PANELS  //potentially use panel structure, each time-slice/interval being an individual cross-section unit
  #define ABMAT_INTERVAL_PANELS false
#else
  #undef ABMAT_INTERVAL_PANELS
  #define ABMAT_INTERVAL_PANELS true  
#endif

#ifndef FUNCTION
  #define FUNCTION EQUATION
#endif

// #ifndef ABMAT_DPM_CFG_PATH
//   #define ABMAT_DPM_CFG_PATH "DOE/ABMAT_DPM.cfg"
// #endif

#ifndef ABMAT_ANALYSIS_CFG_PATH   //file containing the information of the variables processed
  #define ABMAT_ANALYSIS_CFG_PATH "DOE/ABMAT_Analysis.cfg"
#endif

#ifndef ABMAT_INTERVALS_CFG_PATH //file containing the information of the                                                               
  #define ABMAT_INTERVALS_CFG_PATH "DOE/ABMAT_Intervals.cfg"
#endif

/*dependencies*/
#include <cmath>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <tuple>
#include <iterator>
#include <sstream>
#include <limits>
#include <cstring>
#include <ctime>
#include <algorithm> //for sorting


/* For folder creation */
// #ifndef CREATEDIR_H //only include once
//   #include "CreateDir.h" //creation of directories
// #endif

/* GZIP Options */
//#include "zstr/src/zstr.hpp"
//Not yet used. Also, LSD already uses GZIP, so one should be able to use
//it from LSD

/* Some parameters that have defaults */
/* global defines */
namespace ABMAT //a general namespace for the ABMAT tools
{

   
  std::istream& safeGetline(std::istream& is, std::string& t);
  std::istream& safeGetline(std::istream& is, std::string& t, char delim);
  std::string sim_is_done_file(int ConfigID);
  
  std::string Config_Name = "no-name";

  #ifndef ABMAT_VERBOSE //verbose logging
    void vlog(const char *x){ };
  #else
    void vlog(const char *x){plog(x);};
  #endif
  
  std::vector< std::vector<int> > Intervals; //Time intervals
  int max_t_interval=1; //ABMAT::max_t - the number of time steps necessary to compute
  bool add_Interval(int start, int end); //allow possibility to define intervals endogeneously  
  bool load_Intervals_cfg();     
    
   
  bool load_Analysis_cfg();
  bool Analysis_already_loaded=false;
  bool Intervals_already_loaded=false;  
    /*********************
   *  The Tools for the on-the run descriptive analysis and data gathering 
   *********************/
  
  // <label,type>
  std::vector<std::string> Labels_macro;
  std::vector<std::string> Labels_macro_short;
  std::vector<bool> Timeseries_macro;
  std::vector<bool> CumTimeseries_macro;
  std::vector<bool> Runs_macro;
  std::vector<bool> RunsA_macro; //assymmetrical option, see source.
  std::vector<bool> Stats_macro;
  std::vector<bool> Initial_macro;
  std::vector<bool> Last_macro;
  std::vector<bool> cumulative_macro;
  std::vector<bool> Const_macro;
  std::vector<bool> LMoments_macro;
  std::vector <std::vector <std::string>>  Compare_macro;  //can be more than one var for comparison
  std::vector<bool> onlycompare_macro; //mark variables as "only for comparative reasons"

  std::vector<std::string> Labels_micro;
  std::vector<std::string> Labels_micro_short;
  std::vector<bool> Timeseries_micro;
  std::vector<bool> Runs_micro;
  std::vector<bool> RunsA_micro;  
  std::vector<bool> Stats_micro;
//   std::vector<bool> Initial_micro;
//   std::vector<bool> Last_micro;
//   std::vector<bool> Const_micro;
  std::vector<bool> LMoments_micro;
  std::vector <std::tuple <bool,std::string,int,std::string > > Conditional_micro; //conditional statistics (depending on discrete values of other variable)

  std::vector<std::string> Labels_individual;   //similar to micro, but we store each single time series. Missing numbers are adjusted by NA (start later / stop early)
   std::vector<std::string> Labels_individual_short;
  std::vector<bool> Timeseries_individual;
  std::vector<bool> Runs_individual;
  std::vector<bool> RunsA_individual;  
  std::vector<bool> Stats_individual;
  std::vector<bool> Initial_individual;
  std::vector<bool> Last_individual;
  std::vector<bool> Const_individual;
  std::vector<bool> LMoments_individual;

  std::vector<std::string> Labels_parameter; //added: LSD parameter names
  std::vector<std::string> Labels_parameter_short;
  

  std::vector< std::vector<double> > Data_macro;
  //std::vector< std::vector < std::tuple <std::string, std::vector <double> > > > Data_micro;
  std::vector< std::vector < std::vector <double> > > Data_micro;
  std::vector< std::vector < std::vector <double> > > Data_micro_lmoments;
  std::vector< std::vector<std::vector<double> > > Data_individual;
  std::vector< std::vector<object* > > Pointer_individual;   // not save! Pointers me be reused in LSD if an item vanishes! Beware of Memory Leaks!
//  std::vector< std::vector< std::unique_ptr<object> > > Pointer_individual;   // not save! Pointers me be reused in LSD if an item vanishes! Beware of Memory Leaks!
  std::vector<double>      Data_parameter;    //in case of events that change parameters, load them as variable instead / in addition!
  
  std::vector< std::vector <int> > processed_cids_info; //to load and retrieve processed files
  
  // To get time info, see   http://stackoverflow.com/a/2962914/3895476
  struct timespec start, finish;  
                           
  bool grab_Parameters_LSD(object * cur_r); //For use without DOE, grab par vals from LSD according to Config Specification of Pars
  bool update_Data(object * cur_r);
  int get_CID(object * cur_r, int alt_id=0);  
  bool save_stats(object * cur_r, int id=0);
                           
  bool add_Variable(const std::string& Label, const std::string& options, bool theCondVar = false);
  bool prepare_Analysis();
  bool clear_Data(bool reload=false);
  int size_of_Data = 0;  
  std::string results_folder;
  bool create_results_folder();
  bool mark_sim_as_done(int ConfigID);
  std::string in_process_name(int ConfigID);
  bool mark_sim_as_in_process(int ConfigID);
  
}
#endif

