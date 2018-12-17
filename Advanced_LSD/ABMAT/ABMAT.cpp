/* ========================================================================== */
/*                                                                            */
/*   ABMAT.cpp                                                               */
/*   (c) 2016 frederik.schaff@fernuni-hagen.de                                */
/*                                                                            */
/*   Some tools for an improved and simplified sensitivity analysis with
     LSD by Marco Valente (www.labsimdev.de / https://github.com/marcov64/Lsd)
     Implemented for and tested with Version 7.0

    Update this version: Only "analysis" not "DOE".
     
     For more and detailed information see my paper: TBA
     
     The tool: 
     
     a) allows to load configurations from a design point matrix and create, 
        via LSD, the .lsd configuration files / run them. It is also 
        possible to create "sets" of configurations (reducing the number of 
        *.lsd files). The execution is controled in LSD.
     
     b) allows to automatically analyse a set of variables, either macro (1) or
        micro (several)
       
        i) By calculating descriptive information on the distribution over the 
           given Intervals (new!)
           
           ---forget this below---
        ii) By providing a sequential Monte Carlo Randomisation analysis aiming
           at providing evidence in favour of "Reasonable Stationarity"
        iii) This is repeated for the transient phase, if it is declared >0 it 
           is fixed, if 0, it will not be regarded. if < 0, the MSER-5 rule
           will be used to declare the end of the transient. The absolute value
           is equal to the minimum length of the transient (i.e. it will 
           overwrite the MSER Rule)
           
           --forget end---
           
     Requirements: Within LSD model:
     
      Son: ABMAT
      Label ABMAT
      {
      	Var: ABMAT_INIT
      	Var: ABMAT_UPDATE
      	Param: ABMAT_DPM
      	Param: ABMAT_Switch
      	Param: ABMAT_Sets
      	//Param: ABMAT_Max_t //now given by the max interval
      	Param: ABMAT_ConfigID        
      }
     
     Provided "as is" with no warranties or whatsoever. Any comments are welcome
           
     A complete documentation is yet missing, but will be added at some point
     (or, if somebody else than me realy want's to use this toolkit)
     
     Basically (the namespace is ABMAT, i.e. one needs to put "ABMAT::" in front 
     of each function that is called or variable that is changed):
          
     add_DPM("Path","Name")   Add a design point matrix (.tsv, see below) with 
                              the given (relative or absolute) path to be used.                              
     
     clear_Data() Delete all the content and also Variables known.                         
     add_Variable("Label","Type",["save | runs | lmoments"])   Add an LSD Variable (or Parameter/Function)
                              with name "Label" to be analysed. The "Type" can 
                              be either "macro" (only one variable) or "micro"
                              (i.e. a population statistic) or "individual" (i.e. 
                              treating each statistic individually, adding a 
                              generic identifier). In the second case
                              the distribution will be analysed at each "tick" 
                              and for each statistic the distribution over time
                              is analysed again. In the latter case, the indivi-
                              dual series are treated as a macro series, each.
                              This can be done in addition to gathering popula-
                              tion statistics.
                              If any of the additional third options (save | runs | lmoments | stats) are provided (not separated by comma),
                              individual timeseries data is saved / runs statistics are calculated. For the micro
                              stats this is the 8 (or more) stats series but not
                              agent level data. For this, you need to also 
                              include the series as "individual". 
                              
      
     
                                                         */
/*                                                                            */
/* ========================================================================== */

#include "ABMAT.h"
namespace ABMAT {

/* ========================================================================== */

// from: https://stackoverflow.com/a/6089413/3895476
// A getstream that works on windows and linux and mac files EOL chars.
//added deliminator option.
  std::istream& safeGetline(std::istream& is, std::string& t)
  {
      t.clear();
  
      // The characters in the stream are read one-by-one using a std::streambuf.
      // That is faster than reading them one-by-one using the std::istream.
      // Code that uses streambuf this way must be guarded by a sentry object.
      // The sentry object performs various tasks,
      // such as thread synchronization and updating the stream state.
  
      std::istream::sentry se(is, true);
      std::streambuf* sb = is.rdbuf();
  
      for(;;) {
          int c = sb->sbumpc();
          switch (c) {
          case '\n':
              return is;
          case '\r':
              if(sb->sgetc() == '\n')
                  sb->sbumpc();
              return is;
          case EOF:
              // Also handle the case when the last line has no line ending
              if(t.empty())
                  is.setstate(std::ios::eofbit);
              return is;
          default:
              t += (char)c;
          }
      }
}
  
  std::istream& safeGetline(std::istream& is, std::string& t, char delim)
  {
      t.clear();
  
      // The characters in the stream are read one-by-one using a std::streambuf.
      // That is faster than reading them one-by-one using the std::istream.
      // Code that uses streambuf this way must be guarded by a sentry object.
      // The sentry object performs various tasks,
      // such as thread synchronization and updating the stream state.
  
      std::istream::sentry se(is, true);
      std::streambuf* sb = is.rdbuf();
//       int int_delim = bitset<8>delim; //only a single deliminator allowed.
      const int delim_ = delim;
      for(;;) {
          int c = sb->sbumpc();                    
            if (c==delim_)
              return is;
          switch (c) {

          case '\n':
              return is;
          case '\r':
              if(sb->sgetc() == '\n')
                  sb->sbumpc();
              return is;
          case EOF:
              // Also handle the case when the last line has no line ending
              if(t.empty())
                  is.setstate(std::ios::eofbit);
              return is;
          default:
              t += (char)c;
          }
      }
  }

  
  
  
  
 
  //Add intervals. Note that it is taken care of the internal offset -1 here,
  //i.e. all intervals should be in the range [1,max_step]. 
  bool add_Interval(int start, int end){
    if (end < 0){
      plog("\nABMAT :   Info : The interval is autosized. The maximum end is: ");
      plog(std::to_string(-end).c_str());
      plog(" but it will also stop earlier, if necessary.");
    } else if (start >= end) {
      plog("\nABMAT :   Error : in add_Interval start (");
      plog(std::to_string(start).c_str());
      plog(") is not smaller than end (");
      plog(std::to_string(end).c_str());
      plog(")!");
      return false;
    }
    std::vector<int> temp;
    temp.push_back(start-1);
    if (end > 0){
      temp.push_back(end-1);
    } else {
      temp.push_back(end+1); //negative values indicate pot. early stop. we need to change the sign for the offset.
    }
    Intervals.push_back(temp);
    char buffer[64];
    snprintf(buffer,sizeof(char)*64,"\nABMAT :   .. (%i,%i)",start,end);
    int abs_end = end<0?-end:end;
    plog(buffer);
    if (abs_end>max_t_interval){
      max_t_interval=abs_end; //update
    }
    return true;        
  }
  
  bool load_Intervals_cfg(){
    //Only once!
    if (!Intervals_already_loaded){
      Intervals_already_loaded=true;
  
      Intervals.clear();
  //Open Intervals Config that points to single DPMs
      std::ifstream  INTERVALS_CFG(ABMAT_INTERVALS_CFG_PATH);      
      if (!INTERVALS_CFG){
        plog("\nABMAT :   Error: The Intervals Config File at path: ");
        plog(ABMAT_INTERVALS_CFG_PATH);
        plog("' could not be opened!\nABMAT :   ");      
      } else {
        plog("\nABMAT :   Opened the Intervals config file: '");
        plog(ABMAT_INTERVALS_CFG_PATH);
        plog("'.\nABMAT :   Now adding Intervals for analysis ...");
      
        //Parse the information, skip comments.
        std::string line;
        while(safeGetline(INTERVALS_CFG,line))
        {
          if (!(line.find(";")!=std::string::npos)&&!(line.find("#")!=std::string::npos)&&!line.empty()){
            std::stringstream  lineStream(line);        
            std::string  start,end;
            safeGetline(lineStream,start,',');
            safeGetline(lineStream,end,',');
            if (!(start.find(";")!=std::string::npos)&&!(start.find("#")!=std::string::npos)&&!start.empty()&& !(end.find(";")!=std::string::npos) && !(end.find("#")!=std::string::npos)&&!end.empty() ){
              try {            
                if (!add_Interval(std::stoi( start ), std::stoi( end ))){
                  
                }
              }
              catch (...)  {
                	  plog("\nnABMAT :   Error: A stoi error occured in ABMAT.cpp:load_Intervals_cfg(). Perhaps the Intervals file is not nice.\n");
                      return false;
              }
            }              
          }
        }        
        INTERVALS_CFG.close();
      }
    } else {
      plog("\nABMAT :   Subsequent simulation. Intervals for analysis already loaded.");    
    }
    return true;
  }
   
  //Load all the variables to be analysed.
  bool load_Analysis_cfg(){
    //Only once!
    if (!Analysis_already_loaded){
      Analysis_already_loaded=true;

      
      //Reset
      /* NOTE: THE RESET IS NOT WORKING CORRECTLY ATM */
      Labels_macro.clear();
      Labels_micro.clear();
      Labels_parameter.clear();
      //Labels_parameter_micro.clear();      
      //Open Analysis Config that points to single DPMs
      std::ifstream  ANALYSIS_CFG(ABMAT_ANALYSIS_CFG_PATH);      
      if (!ANALYSIS_CFG){
        plog("\nABMAT :   Error: The Analysis Config File at path: ");
        plog(ABMAT_ANALYSIS_CFG_PATH);
        plog("' could not be opened!");
        return false;      
      } else {
        plog("\nABMAT :   Opened the Analysis config file: '");
        plog(ABMAT_ANALYSIS_CFG_PATH);
        plog("'.\nABMAT :   Now adding Variables for analysis ...");
      
        //Parse the information, skip comments.
        std::string line;
        while(safeGetline(ANALYSIS_CFG,line))
        {
          if (!(line.find(";")!=std::string::npos)&&!(line.find("#")!=std::string::npos)&&!line.empty()){
            std::stringstream  lineStream(line);        
            std::string  Label,Options;
            safeGetline(lineStream,Label,',');
            safeGetline(lineStream,Options);            
            if(!add_Variable(Label, Options)){ //Add the var
              return false;
            }               
          }
        }        
        ANALYSIS_CFG.close();
      }
        
                
    } else {
      plog("'.\nABMAT :   Subsequent simulation. Variables for analysis already loaded.");
    }
    return prepare_Analysis(); //each time! 
  }
  
  //Add a variable to the maps, also add an un-initialised std::vector for the data. 
  //Update: use c++ std::basic_string::find()
  bool add_Variable(const std::string& Label, const std::string& Options, bool theCondVar /*= false*/){
    std::string short_label = "";
    int kind=0;
    if (Options.find("macro")!=std::string::npos){ //if string is contained in Type
      kind++;
      Labels_macro.push_back(Label);
      Timeseries_macro.push_back(Options.find("save")!=std::string::npos);  //evaluates to true if option selected, else false.
      CumTimeseries_macro.push_back(Options.find("cumSave")!=std::string::npos);  //evaluates to true if option selected, else false.
      Runs_macro.push_back(Options.find("runs")!=std::string::npos);
      RunsA_macro.push_back(Options.find("runs_asym")!=std::string::npos); //runsA is an additional option, always containing standard symmetrical runs.
      Stats_macro.push_back(Options.find("stats")!=std::string::npos);
      Initial_macro.push_back(Options.find("first")!=std::string::npos);
      Last_macro.push_back(Options.find("last")!=std::string::npos);
      cumulative_macro.push_back(Options.find("cumulative")!=std::string::npos); //cumulative
      Const_macro.push_back(Options.find("const")!=std::string::npos);
      LMoments_macro.push_back(Options.find("lmoments")!=std::string::npos);
      onlycompare_macro.push_back(theCondVar);

      /* Add the short-label to the table. If it does not exist, create an
        appropriate one of the first 4 characters and ensure that it is unique.
        short_labels should have no more than 6 characters, but the user is free
        to assign longer ones if it is done explicitly.
      */
      if (Options.find("short=")!=std::string::npos){
        size_t string_pos = Options.find("short=");
        string_pos = Options.find("=",string_pos) + 1; //find start of short variable label
        size_t string_end = Options.find_first_of(" ,;#\n\r",string_pos);//find end of short variable label
        short_label = Options.substr(string_pos,string_end-string_pos);  //extract substring
        Labels_macro_short.push_back(short_label);
      } else {
        short_label = Label.substr(0,4);  //extract substring
        bool short_label_exists;
        int add_i = 0;
        do {
          short_label_exists = false;
          for (auto item : Labels_macro_short){
            if (item.find(short_label)!=std::string::npos){
              short_label_exists = true;
              break;
            }
          }
          if (short_label_exists){
            add_i++;
            short_label = Label.substr(0,4) + "_" + std::to_string(add_i);
          }
        } while (short_label_exists);
        Labels_macro_short.push_back(short_label);
      }

      size_t string_pos = Options.find("compare=");
      std::vector  <std::string> compare;
      Compare_macro.push_back(compare); //important, create prior to possible recursions.
      int current_compare_indx = Compare_macro.size()-1;
      while (!theCondVar && string_pos!=std::string::npos){
          //We compare the data to some other data, both lying within the model
          //Which is, in a first step, added to the variables
          //And of which we take the same stats as for the base data
          string_pos = Options.find("=",string_pos) + 1; //find start of conditional variable label
          size_t string_end = Options.find_first_of(" ,;#\n\r",string_pos);//find end of conditional variable label
          std::string condLabel = Options.substr(string_pos,string_end-string_pos);  //extract substring
          Compare_macro.at(current_compare_indx).push_back(condLabel);
          std::string OptionsCopy = "macro only_for_comparison_not_saved";
          plog("\nABMAT :   Added macro variable ");plog(Label.c_str());plog(" compared to variable ");plog(condLabel.c_str());//no automatic testing at this position!
          add_Variable(condLabel,OptionsCopy,true); //important: this needs to be called last!
          string_pos = Options.find("compare=",string_pos); //move one step further in comparative variables
      }

    } //end macro
    if (Options.find("micro")!=std::string::npos){
      /* Add the short-label to the table. If it does not exist, create an
        appropriate one of the first 4 characters and ensure that it is unique.
        short_labels should have no more than 6 characters, but the user is free
        to assign longer ones if it is done explicitly.

        in diff to the macro var, the label is added later in the do cycle.

        Known issue: if short is not supplied, and there are similar variables
        with a conditional, this will cause problems...
      */
      short_label;
      if (Options.find("short=")!=std::string::npos){
        size_t string_pos = Options.find("short=");
        string_pos = Options.find("=",string_pos) + 1; //find start of short variable label
        size_t string_end = Options.find_first_of(" ,;#\n\r",string_pos);//find end of short variable label
        short_label = Options.substr(string_pos,string_end-string_pos);  //extract substring
      } else {
        short_label = Label.substr(0,4);  //extract substring
        bool short_label_exists;
        int add_i = 0;
        do {
          short_label_exists = false;
          for (auto item : Labels_micro_short){
            if (item.find(short_label)!=std::string::npos){
              short_label_exists = true;
              break;
            }
          }
          if (short_label_exists){
            add_i++;
            short_label = Label.substr(0,4) + "_" + std::to_string(add_i);
          }
        } while (short_label_exists);
      }

      //Due to the conditional option, we need to initialise a number of "micro" 
      //variables equal to the number of conditions. In the absence of a 
      //condition there is only one micro variable
      int n_conds = 0; //for each condition
      std::vector <int> conditions;
      std::string condLabel = "";
      std::string condShort_label = "";
      size_t string_pos = Options.find("conditional=");
      if (string_pos !=std::string::npos){
        //we have defined a conditional parameter (only one allowed. 
        //Use a second otherwise identical key to allow more.
        //Hence, if you want to have a condition that only becomes "active" later
        //you need to make sure it exists just when the variables are 
        //initialised (in t==1, in general).
        //  -- YOU MAY also call ABMAT::load_Analysis_cfg() out of order 
        //  -- (before the end of step 1) and before the initialisation, 
        //  -- to initialise it with fake data. 
        //to do: Class for Analysis to increase performance / calc at most 
        // and allow more flexibility (emergence of new options)      
     
        string_pos = Options.find("=",string_pos) + 1; //find start of conditional variable label     
        size_t string_end = Options.find_first_of(" ,;#\n\r",string_pos);//find end of conditional variable label
        condLabel = Options.substr(string_pos,string_end-string_pos);  //extract substring

          //define a short_label for the conditional variable, too.

          if (Options.find("condShort=")!=std::string::npos){
            size_t string_pos = Options.find("condShort=");
            string_pos = Options.find("=",string_pos) + 1; //find start of short variable label
            size_t string_end = Options.find_first_of(" ,;#\n\r",string_pos);//find end of short variable label
            condShort_label = Options.substr(string_pos,string_end-string_pos);  //extract substring
          } else {
            condShort_label = condLabel.substr(0,4);  //extract substring
          } //end short_label of cond

        plog("\nABMAT :   Added micro variable ");plog(Label.c_str());plog(" conditional on variable ");plog(condLabel.c_str()); plog(" aka ");plog(condShort_label.c_str());//no automatic testing at this position!
        
        //check how many options there are
        //Note: Currently only working if all objects holding the variable are
        //situated in the same parent. I THINK the same restrictions apply to STATX
        object *cur_r = root->search_var(root,condLabel.c_str())->up; //find first object with variable
//         double stats[5];
//         root->stat( condLabel.c_str(), stats);
//         plog("\n Stats command yields variance: ");plog(std::to_string(stats[2]).c_str());
//         plog("\n And first agent has: "); plog(std::to_string(VS(cur_r,condLabel.c_str())).c_str()); 
        for (;cur_r!=NULL; cur_r=cur_r->next){ //Cycle through all objects of same kind
          conditions.push_back(int(VS(cur_r,condLabel.c_str()))); //add variable to conditions                        
        }
        std::sort(conditions.begin(),conditions.end()); //sort
        auto it = std::unique(conditions.begin(),conditions.end()); //delete duplicates
        conditions.resize( std::distance(conditions.begin(),it) );
        plog("\nABMAT :   There are ");plog(std::to_string(conditions.size()).c_str()); plog(" different options for variable "); plog(condLabel.c_str());plog(":");
        for(auto it : conditions){
          plog(" ");plog(std::to_string(it).c_str());
        }
        
        
      }
         
      do {
        kind++;
        Labels_micro.push_back(Label);
        Labels_micro_short.push_back(short_label);
        Timeseries_micro.push_back(Options.find("save")!=std::string::npos);
        Runs_micro.push_back(Options.find("runs")!=std::string::npos);
        RunsA_micro.push_back(Options.find("runs_asym")!=std::string::npos);
        Stats_micro.push_back(Options.find("stats")!=std::string::npos);
//        Initial_micro.push_back(Options.find("first")!=std::string::npos);
//        Last_micro.push_back(Options.find("last")!=std::string::npos);
//        Const_micro.push_back(Options.find("const")!=std::string::npos);
        LMoments_micro.push_back(Options.find("lmoments")!=std::string::npos);
        
        if (conditions.size()>0) {                   
          Conditional_micro.push_back(make_tuple(true, condLabel, conditions.at(n_conds),condShort_label ) ); //init factors later/continuously.
        } else {
          Conditional_micro.push_back(make_tuple(false,condLabel,0, condShort_label)); //empty
        }
        n_conds++;
      } while (n_conds<conditions.size());      
    } //end micro

    if (Options.find("individual")!=std::string::npos){
      kind++;
      Labels_individual.push_back(Label);
      Timeseries_individual.push_back(Options.find("save")!=std::string::npos);
      Runs_individual.push_back(Options.find("runs")!=std::string::npos);
      RunsA_individual.push_back(Options.find("runs_asym")!=std::string::npos);
      Stats_individual.push_back(Options.find("stats")!=std::string::npos);
      Initial_individual.push_back(Options.find("first")!=std::string::npos);
      Last_individual.push_back(Options.find("last")!=std::string::npos);
      Const_individual.push_back(Options.find("const")!=std::string::npos);
      LMoments_individual.push_back(Options.find("lmoments")!=std::string::npos);

      if (Options.find("short=")!=std::string::npos){
        size_t string_pos = Options.find("short=");
        string_pos = Options.find("=",string_pos) + 1; //find start of short variable label
        size_t string_end = Options.find_first_of(" ,;#\n\r",string_pos);//find end of short variable label
        short_label = Options.substr(string_pos,string_end-string_pos);  //extract substring
        Labels_individual_short.push_back(short_label);
      } else {
        short_label = Label.substr(0,4);  //extract substring
        bool short_label_exists;
        int add_i = 0;
        do {
          short_label_exists = false;
          for (auto item : Labels_individual_short){
            if (item.find(short_label)!=std::string::npos){
              short_label_exists = true;
              break;
            }
          }
          if (short_label_exists){
            add_i++;
            short_label = Label.substr(0,4) + "_" + std::to_string(add_i);
          }
        } while (short_label_exists);
        Labels_individual_short.push_back(short_label);
      }

    } //end individual

    /* Parameters - there is no "short" option here */
    if (Options.find("parameter")!=std::string::npos){
      kind++;
      Labels_parameter.push_back(Label.c_str()); 
    }
    if (kind==0) {
      plog("\nABMAT :   Error: Unknown Type of Variable to be added in TOOLS::add_Variable: ");plog(Options.c_str());
      return false;
    }
    plog("\nABMAT :   .. '");plog(Label.c_str()); plog("' aka '");plog(short_label.c_str()); plog("' (");
        plog(Options.c_str()); plog(") '");
    return true;
  }
  

  
  //For each variable, add the current value to the std::vector
  bool update_Data(object * cur_r){
    size_of_Data++; // increase internal counter
    
    //Aggregate Data
    for (int i=0; i<Labels_macro.size();i++){
      Data_macro[i].push_back( VS(cur_r,Labels_macro[i].c_str() ) );  
    }
    
    //Micro Data (in-between analysis)
    for (int i=0; i<Labels_micro.size();i++){
      
      std::vector<double> pop_dist_full; //populate a temporary std::vector with the current values for each single agent
      variable * curv = cur_r->search_var(cur_r,Labels_micro[i].c_str());
      object * cur = curv->up;   
      
      //if conditional is selected, only add those who meet the criterium         
      for(;cur!=NULL; cur=cur->next){
        if (!std::get<bool>(Conditional_micro.at(i)) 
              || std::get<int>(Conditional_micro.at(i)) == int(VS(cur,std::get<1>(Conditional_micro.at(i)).c_str())) ){
          pop_dist_full.push_back(VS(cur,Labels_micro[i].c_str() ));
        }
      }
      if (Stats_micro[i]){
       //Calculate the stats for the current point in time
        stats_vector EightStats = calc_EightStats(&pop_dist_full[0],int(pop_dist_full.size()));     
        //first a temporary frame, to keep the size flexibel to changes in the stats
        //std::vector<double> temp_stats;
      
        for (int s=0; s<EightStats.size(); s++){        
          Data_micro[i][s].push_back(std::get<double>(EightStats[s])); //only the value, not the label
        }
      }
      
      if (LMoments_micro[i]){      
        stats_vector LMoments = calc_L_Moments(&pop_dist_full[0],int(pop_dist_full.size()));
        for (int s=0; s<LMoments.size(); s++){
          Data_micro_lmoments[i][s].push_back(std::get<double>(LMoments[s]));
        }           
      }
      
      
    }
    
    //Individual Data (within analysis)
    for (int i=0; i<Labels_individual.size();i++){
      int count_adds = 0;
      variable * curv = cur_r->search_var(cur_r,Labels_individual[i].c_str());  //search first item
      object * cur = curv->up; //get pointer of item      
      for(;cur!=NULL; cur=cur->next){ //Cycle through LSD items
        //Get to corresponding item in objects - room for more efficient cycling 
        object * cur_ABMAT = NULL;          
        for(int k=0; k<Pointer_individual[i].size(); k++){
          if (cur == Pointer_individual[i][k] ){
            cur_ABMAT = cur;
            Data_individual[i][k].push_back(VS(cur,Labels_individual[i].c_str()));
            break;
          }
        }
        if (cur_ABMAT == NULL){
          //need to add a new data series
          count_adds++;
          std::vector<double> pop_dist_full;
          for (int j=1;j<size_of_Data;j++){ //take care, start with 1 because time is from 1 on
            pop_dist_full.push_back(NAN); //using LSD NAN - quiet if possible
          }
            pop_dist_full.push_back(VS(cur,Labels_individual[i].c_str()));  //actual data-point
          Data_individual[i].push_back(pop_dist_full);
//           Pointer_individual[i].push_back(std::shared_ptr<object>(cur));
          Pointer_individual[i].push_back(cur);
        }
      }
      
      //Take care of "the graveyard", i.e. individuals that were no more living
      for (int k=0;k<Data_individual[i].size();k++){
        if(Data_individual[i][k].size()<size_of_Data){
          Data_individual[i][k].push_back(NAN); //Add a LSD NAN
        }
      }
      if (count_adds>0){      
        vlog("\nABMAT :   New individual variable(s) initialised. Type: '"); 
        vlog(Labels_individual.at(i).c_str()); 
        vlog("' Count : "); vlog(std::to_string(count_adds).c_str());
      }
    }
    return true;                 
  }
  
   //clear old data and set up containers for the data accordingly
  bool prepare_Analysis(){
    //size_of_Data = 0;    
    //Data_macro.clear();
    Data_macro.resize(Labels_macro.size());
    
    //Data_micro.clear();
    int size8=get_EightStats_head().size();
    Data_micro.resize(Labels_micro.size());
      for (int i=0;i<Labels_micro.size();i++){
       Data_micro[i].resize(size8); //for 8 stats
      }
    int sizeLM=get_L_Moments_head().size();      
    Data_micro_lmoments.resize(Labels_micro.size());
     for (int i=0;i<Labels_micro.size();i++){
       Data_micro_lmoments[i].resize(sizeLM);
     }
    //Data_individual.clear();
    Data_individual.resize(Labels_individual.size());    
    //Pointer_individual.clear();
    Pointer_individual.resize(Labels_individual.size());
    return true;    
  }  
  
  bool clear_Data(bool reload){
    /* Similar to prepare, only in addition trigers to load config files again. Not used automatically*/
    if (reload){
      Analysis_already_loaded = false;
      Intervals_already_loaded = false;
      Labels_individual.clear();
      Labels_macro.clear();
      Labels_micro.clear();
      Intervals.clear();      
      max_t_interval=1; //reset        
    }
//     use_grabbed_pars = false;
//     Parameters_grabbed.clear();
        
        size_of_Data = 0;

    Timeseries_individual.clear();
          Runs_individual.clear();
         RunsA_individual.clear();
      LMoments_individual.clear();    
          Data_individual.clear();
       Pointer_individual.clear();    

    Timeseries_macro.clear();
 CumTimeseries_macro.clear();
          Runs_macro.clear();
         RunsA_macro.clear();
      LMoments_macro.clear();
          Data_macro.clear();
   onlycompare_macro.clear();

    Timeseries_micro.clear();
          Runs_micro.clear();
         RunsA_micro.clear(); 
      LMoments_micro.clear();        
          Data_micro.clear();
    return true;                     
  }
  
  int get_CID(object * cur_r, int alt_id){
  /* Get the CID, important to allow being called by other things like 
    PajekToCpp
  */
    if(!Analysis_already_loaded){
      plog("\nABMAT :   Error! Trying to use ABMAT::get_CID without Analysis loaded");
      return -666;      
    }
    int CID;
    if (cur_r == NULL) {//use alt_id! e.g. if no ID provided
        CID = alt_id; //use passed through ID
    } else {
      CID = VS(cur_r,"ABMAT_ConfigID");
      if (CID < 0) {     //i.e. provided via LSD not grabbed
        CID = -CID;
      }
    }
    return CID;
  }
  

  bool create_results_folder(){
      //Create folders for results
    results_folder = "Results";    
//     plog("\nABMAT :   Direct call to makePath.");
    if (!makePath(results_folder.c_str())){ //Base path
      plog("\nABMAT :   ERROR: makePath() for '");
      plog(results_folder.c_str());
      plog("' not working!");
      return false; 
    }      
    #ifdef ABMAT_DOE_H
      if (DPM_choice>0){
#ifndef SUPPRESS_SUB_FOLDERS  //a mode with only one results folder for linux (in vm)
        results_folder+= "/";
        results_folder+=DPM_names[DPM_choice-1].c_str();         
//         snprintf(results_folder,sizeof(char)*256,"%s/%s",results_folder,DPM_names[DPM_choice-1].c_str());
#else
        results_folder+= "_";
        results_folder+=DPM_names[DPM_choice-1].c_str();
//         snprintf(results_folder,sizeof(char)*256,"%s_%s",results_folder,DPM_names[DPM_choice-1].c_str());
#endif        
      } else {
#ifndef SUPPRESS_SUB_FOLDERS  //a mode with only one results folder for linux (in vm)
        results_folder += "/no_name";              
//         snprintf(results_folder,sizeof(char)*256,"%s/%s",results_folder,"no-name");
#else
        results_folder += "_no_name";
//         snprintf(results_folder,sizeof(char)*256,"%s_%s",results_folder,"no-name");
#endif        
      }
    #else
#ifndef SUPPRESS_SUB_FOLDERS  //a mode with only one results folder for linux (in vm)
        results_folder += "/LSD_DOE";
//         snprintf(results_folder,sizeof(char)*256,"%s/%s",results_folder,"LSD_DOE");  //If no ABMAT-DOE used
#else
        results_folder += "_LSD_DOE";        
//         snprintf(results_folder,sizeof(char)*256,"%s_%s",results_folder,"LSD_DOE");  //If no ABMAT-DOE used
#endif        
    #endif 
#ifndef SUPPRESS_SUB_FOLDERS  //a mode with only one results folder for linux (in vm)   
//       plog("\nABMAT :   Direct call to makePath - critical!.");
      if (!makePath(results_folder.c_str())){ //Base path
        plog("\nABMAT :   ERROR: makePath() for '");
        plog(results_folder.c_str());
        plog("' not working!");
        return false; 
      }  
#endif
  return true;
  }
  
    std::string sim_is_done_file(int ConfigID){
      std::string buffer;
      buffer = results_folder;
      buffer += "/_done_files/CID_";
      buffer += std::to_string(ConfigID);
      buffer += "_I";
      // To also check if the analysis config was the same, the interval info is saved in addition. NOT the variables. This info is later useful to speed up data integration in grelt, e.g.
      for (int i=0;i<Intervals.size();i++){          //
        buffer+="_";
        buffer+=std::to_string(Intervals.at(i).at(0)+1);
        buffer+="to";
        buffer+=std::to_string(Intervals.at(i).at(1)+1);
      }
      return buffer;
    }
  
    bool mark_sim_as_done(int ConfigID){
    /* mark if file has been processed. NOTE: Also mark Intervals! 
    */
    //Create / check if exist folders for results
    if (!create_results_folder()){
      return false;
    }
    //Create / check if exist folder for under process
    std::string buffer = results_folder;
    buffer += "/_done_files";
    if (!makePath(buffer.c_str())){ //Base path
      plog("\nABMAT :   ERROR: makePath() for '");
      plog(buffer.c_str());
      plog("' not working!");
      return false; 
    } 
    
//     buffer += "/CID_";
//     buffer += std::to_string(ConfigID);
//     buffer += "I";
//     // To also check if the analysis config was the same, the interval info is saved in addition. NOT the variables. This info is later useful to speed up data integration in grelt, e.g.
//     for (int i=0;i<Intervals.size() && buffer.length()<100;i++){
//       buffer+="_";
//       buffer+=Intervals.at(i).at(0);
//       buffer+="-";
//       buffer+=Intervals.at(i).at(1);
//     }
    //Note: Not save for too long names... therefore limit it a bit.
    std::string file;
    file = sim_is_done_file(ConfigID);
    std::ofstream fs;  
    fs.open(file.c_str(),std::ios_base::out | std::ios_base::trunc);
    if (!fs.is_open()){
      plog("\nABMAT :   ERROR:  Could not open file to mark progress: ");
      plog(file.c_str());
    }
    fs.close();
    return (!fs.is_open());
  }
  
  std::string in_process_name(int ConfigID){
    std::string temp = results_folder;
    temp += "/_in_process";
    temp += "/_";
    temp += std::to_string(ConfigID);
    return temp;
  }
  
  bool mark_sim_as_in_process(int ConfigID){
    //Create / check if exist folders for results
    if (!create_results_folder()){
      return false;
    }
    //Create / check if exist folder for under process
    std::string buffer = results_folder;
    buffer += "/_in_process";
    if (!makePath(buffer.c_str())){ //Base path
      plog("\nABMAT :   ERROR: makePath() for '");
      plog(buffer.c_str());
      plog("' not working!");
      return false; 
    }          
    std::ofstream fs;     
    fs.open(in_process_name(ConfigID).c_str(),std::ios_base::out | std::ios_base::trunc);
    if (!fs.is_open()){
      plog("\nABMAT :   ERROR:  Could not open file to mark progress: ");
      plog(buffer.c_str());
      return false;
    }
    fs.close();
    return (!fs.is_open());
  }  
  
  bool save_stats(object * cur_r, int id){
  /* Cycle through the stats selected for analysis and save the results, 
    together  with the parameter configuration (comprehensive)
    
    The stats will always be saved in RESULTS/CFGNAME, except the "single" 
    keyword is signed.
    
    RESULTS/CFGNAME/Aggregate/parameters_CID.tsv #parameters
    RESULTS/CFGNAME/Aggregate/interval_INTERVAL_CID.tsv #stats for INTERVAL
    
    RESULTS/CFGNAME/Timeseries/CID_CID/name.tsv #holds the time series, e.g. CID_1/Attendance.tsv
    
  */
  
    //switch for additional transient analysis.
//     bool no_trans_analysis = Par_Analysis_Transient == 0 ? true : false;
    char add_interval[4];   //The interval specifier, from _I0 to _I99 possible
                            //To do: pannel data option.
    sprintf(add_interval,""); //will change later    

    //Get CID
    int CID = get_CID(cur_r,id);
    
    /* Structure of folders 
    |
    Results--|--ConfigName--|-- ...  
    ... |--Aggregate    --|--CID_1--|--[files]
        |                 |--CID_2--|--[files]
                          ...
        |--Time-Series  --|--CID_1--|--Individual--|--[files]
        |                 |         |--Micro--|--[files]
        |                 |         |--Macro--|--[files]
        |                 |--CID_2--|--Individual--|--[files]
        |                 |         |--Micro--|--[files]
        |                 |         |--Macro--|--[files]
        |--Configuration--|--CID_1--|--[files]
                          |--CID_2--|--[files]
                          ...
                          ...
        |--LSD         --|--CID_1--|--[files]
                       --|--CID_2--|--[files]
                          ...
    */
    

//create folder for Results/ConfigName
  if (!create_results_folder()){
    return false;
  }   
    
    
    //A subfolder for configuration files
//       char configurationf[256];
      std::string configurationf;
#ifndef SUPPRESS_SUB_FOLDERS  //a mode with only one results folder for linux (in vm)
//       snprintf(configurationf,sizeof(char)*256,"%s/Configuration",buffer.c_str());
      configurationf = results_folder.c_str();
      configurationf += "/Configuration";          
#else
//       snprintf(configurationf,sizeof(char)*256,"%s_Configuration",buffer.c_str());
      configurationf = results_folder.c_str();
      configurationf += "_Configuration";
#endif      
    
    //A subfolder for aggregate stats
//       char aggregates[256];
      std::string aggregates;
#ifndef SUPPRESS_SUB_FOLDERS  //a mode with only one results folder for linux (in vm)      
//       snprintf(aggregates,sizeof(char)*256,"%s/Aggregate",buffer.c_str());
      aggregates = results_folder.c_str();
      aggregates += "/Aggregate";
#else
//       snprintf(aggregates,sizeof(char)*256,"%s_Aggregate",buffer.c_str());
      aggregates = results_folder.c_str();
      aggregates += "_Aggregate";
#endif      
    
    //and another folder for individual time-series
//       char timeseries[256];
      std::string timeseries;
#ifndef SUPPRESS_SUB_FOLDERS  //a mode with only one results folder for linux (in vm)      
//       snprintf(timeseries,sizeof(char)*256,"%s/Timeseries",buffer.c_str());
      timeseries = results_folder.c_str();
      timeseries += "/Timeseries";
#else
//       snprintf(timeseries,sizeof(char)*256,"%s_Timeseries",buffer.c_str());
      timeseries = results_folder.c_str();
      timeseries += "_Timeseries";      
#endif
    
    //Create folder for LSD data saves (generic) and pass info to LSD.
#ifndef SUPPRESS_SUB_FOLDERS  //a mode with only one results folder for linux (in vm)    
      std::string lsd_alt_path;
      lsd_alt_path = results_folder.c_str();
      lsd_alt_path += "/LSD_files";
      //plog("\nABMAT :   Direct call to makePath");
      if (!makePath(lsd_alt_path.c_str())){ //Base path
        plog("\nABMAT :   ERROR: makePath() for '");
        plog(lsd_alt_path.c_str());
        plog("' not working!");
        return false; 
      }  
      results_alt_path(lsd_alt_path.c_str());
#else
      results_alt_path("Results");          
#endif      
    
    
    /**********************************************************/
    double internal_CID=-1;   //to provide a unique pointer 
    std::string internal_CID_Name="CID"; //default if not ABMAT_DOE, else ABMAT_CID 

#ifndef SUPPRESS_SUB_FOLDERS  //a mode with only one results folder for linux (in vm)    
    //first, save parameter configuration
    //plog("\nABMAT :   Direct call to makePath");
    vlog("\nABMAT :   Trying to make path : ");
    vlog(configurationf.c_str()); 
    if (!makePath(configurationf.c_str())){ //Base path
      plog("\nABMAT :   ERROR: makePath() for '");
      plog(configurationf.c_str());
      plog("' not working!");
      return false; 
    }
#endif    

#ifndef SUPPRESS_SUB_FOLDERS  //a mode with only one results folder for linux (in vm)          
//     snprintf(configurationf,sizeof(char)*256,"%s/CID_%i",configurationf,CID);
    configurationf += "/";
    configurationf += "CID_";
    configurationf += std::to_string(CID);
    //plog("\nABMAT :   Direct call to makePath");
    if (!makePath(configurationf.c_str())){ //Base path
      plog("\nABMAT :   ERROR: makePath() for '");
      plog(configurationf.c_str());
      plog("' not working!");
      return false; 
    }  
#else
//    snprintf(configurationf,sizeof(char)*256,"%s_CID_%i",configurationf,CID);
    configurationf += "_";
    configurationf += "CID_";
    configurationf += std::to_string(CID);
#endif    
                
    //Create, open and clear file
#ifndef SUPPRESS_SUB_FOLDERS  //a mode with only one results folder for linux (in vm)                    
//     snprintf(configurationf,sizeof(char)*256,"%s/Parameters.tsv",configurationf);
    configurationf += "/Parameters.tsv";    
#else
//     snprintf(configurationf,sizeof(char)*256,"%s_Parameters.tsv",configurationf);
    configurationf += "_Parameters.tsv";
#endif       
    std::ofstream save_file;
//     save_file.setf(std::ios::fixed);
    save_file.precision(std::numeric_limits<long double>::digits10 + 1);   
    save_file.open(configurationf.c_str(),std::ios_base::out | std::ios_base::trunc);   
    
    if (save_file){
      plog("\nABMAT :   Saving parameters of the simulation experiment (ABMAT) in file : ");
      plog(configurationf.c_str());
    } else {
      plog("\nABMAT :   Error: Could not open file for saving data.");
      return false;
    }
    /******************
           *                   Configuration        ************************/
          /* First, gather all the stats. Second, write the to the file */
    //The first line for the header, the second for the data.
    for (int line=0; line<2; line++){
      
      #ifdef ABMAT_DOE_H  
        //The Parameters
        for (int i=0; i<Labels.size();i++){
          if (i>0)
            save_file << "\t"; //Tab sep
          if (line == 0){
            save_file << Labels[i].c_str();
            if(i==0)
              internal_CID_Name = Labels[i];
          } else {
              if (use_grabbed_pars) {
                save_file << Parameters_grabbed[i];
                if (i==0)
                  internal_CID=Parameters_grabbed[i];
              } else {
                save_file << Parameters[Config_loaded-1][i]; //use the input matrix data
                if (i==0)
                  internal_CID=Parameters[Config_loaded-1][i];
              }
          }   
        }         
      #else
        //use without DOE
        for (int i=0; i<Labels_parameter.size();i++){
          if (i>0)
            save_file << "\t"; //Tab sep
          if (line == 0){
            save_file << Labels_parameter[i].c_str();
          } else {
            save_file << Data_parameter[i];
          }   
        }
        //No parameters specified. Use generic CID
        if (Labels_parameter.size() == 0){
          if (line == 0){
            save_file << "generic_CID";            
          } else {
            save_file << CID;
          }          
        }                    
      #endif    
      //Time needed & Total Steps computed
      if (line==0){
        save_file << "\ttime" << "\tsteps_computed";
      } else {            
        double elapsed = (finish.tv_sec - start.tv_sec);
        elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;                                                        
        save_file << "\t" << elapsed << "\t" << size_of_Data; 
      }
      
      save_file << "\n"; //Finished
    }
    save_file.close();
          

     /**********************************************************/
    
    //second, save the aggregate stats and also the pseudo aggregates (individual)
    //if (Labels_macro.size()>0) {
#ifndef SUPPRESS_SUB_FOLDERS  //a mode with only one results folder for linux (in vm)
      vlog("\nABMAT :   Trying to make path : ");
      vlog(aggregates.c_str());        
      if (!makePath(aggregates.c_str())){ //Base path
        plog("\nABMAT :   ERROR: makePath() for '");
        plog(aggregates.c_str());
        plog("' not working!");
        return false; 
      }  
//       snprintf(aggregates,sizeof(char)*256,"%s/CID_%i",aggregates,CID);
      aggregates += "/CID_";
      aggregates += std::to_string(CID);
      //plog("\nABMAT :   Direct call to makePath");
      vlog("\nABMAT :   Trying to make path : ");
      vlog(aggregates.c_str()); 
      if (!makePath(aggregates.c_str())){ //Base path
        plog("\nABMAT :   ERROR: makePath() for '");
        plog(aggregates.c_str());
        plog("' not working!");
        return false; 
      }      
#else
//       snprintf(aggregates,sizeof(char)*256,"%s_CID_%i",aggregates,CID);
      aggregates += "_CID_";
      aggregates += std::to_string(CID);
#endif      
  
       
      //We may have several intervals. Save the data here in one file for each.
      //Open multiple streams
      std::vector<std::ofstream> result_files;
      //std::vector< std::shared_ptr< std::ofstream > > result_files;
      std::vector<std::string> result_paths;
      for (int i=0; i<Intervals.size(); i++){    
//         char intervalf[256];
        std::string intervalf;
#ifndef SUPPRESS_SUB_FOLDERS  //a mode with only one results folder for linux (in vm)        
//         snprintf(intervalf,sizeof(char)*256,"%s/%i-%i",aggregates.c_str(),Intervals[i][0]+1,Intervals[i][1]+1);
        intervalf = aggregates.c_str();
        intervalf += "/";
        intervalf += std::to_string(Intervals[i][0]+1);
        intervalf += "-";
        intervalf += std::to_string(Intervals[i][1]+1);
        
        //plog("\nABMAT :   Direct call to makePath");
        vlog("\nABMAT :   Trying to make path : ");
        vlog(intervalf.c_str());         
        if (!makePath(intervalf.c_str())){ //Base path
          plog("\nABMAT :   ERROR: makePath() for '");
          plog(intervalf.c_str());
          plog("' not working!");
          return false; 
        }              
        std::string filename = intervalf + "/Aggregates.tsv";
#else
//         snprintf(intervalf,sizeof(char)*256,"%s_%i-%i",aggregates.c_str(),Intervals[i][0]+1,Intervals[i][1]+1);
        intervalf = aggregates.c_str();
        intervalf += "_";
        intervalf += std::to_string(Intervals[i][0]+1);
        intervalf += "-";
        intervalf += std::to_string(Intervals[i][1]+1);
        std::string filename = intervalf + "_Aggregates.tsv";
#endif        
        std::ofstream out;
//         out.setf(std::ios::fixed);
        out.precision(std::numeric_limits<long double>::digits10 + 1); 
        vlog("\nABMAT :   Trying to open ofstream : ");
        vlog(filename.c_str());   
        out.open(filename.c_str(),std::ios_base::out | std::ios_base::trunc);
        result_paths.push_back(filename);
        result_files.push_back(std::move(out));
        //result_files.push_back(std::make_shared<std::ofstream(filename.c_str(),std::ios_base::out | std::ios_base::trunc));                
        
        if (result_files.back()){
          plog("\nABMAT :   Saving parameters of the simulation experiment (ABMAT) in file : ");
          plog(result_paths.back().c_str());
        } else {
          plog("\nABMAT :   Error: Could not open file for saving data.");
          return false;
        }
      }
      
             
        //Statistics from now on. Saved in individual files for each interval.
        for (int cur_interval = 0; cur_interval < Intervals.size(); cur_interval++){
            //Check if second "round" performing on the transient is warranted.
          int start_time = Intervals[cur_interval][0];
          int end_time = Intervals[cur_interval][1];

          //Check if the interval is within the computed interval
          if (start_time > size_of_Data){
            plog("\nABMAT : Error: Current interval start after current time. Skipped.");
            continue;
          }

          if (end_time < 0){
            plog("\nABMAT : Info: Auto-adjusted interval selected.");
            plog("\nABMAT : Info: Target end: ");plog(std::to_string(-end_time+1).c_str());
            if (-end_time > size_of_Data){
              end_time = size_of_Data;
              plog("\nABMAT : Info: End adjusted to ");plog(std::to_string(end_time).c_str());
            } else {
              end_time = -end_time;
            }
          }

          if (end_time > size_of_Data){
            plog("\nABMAT : Error: Current interval ends after current time. Skipped.");
            continue;
          }
  
          //- todo: switch for pannel structure! - then var-names are same
          if (Intervals.size()>1 && !ABMAT_INTERVAL_PANELS){
            snprintf(add_interval,sizeof(char)*4,"_I%i",cur_interval+1); //to be clear          
          } else {
            sprintf(add_interval,""); //no differentiation
          }
         
         //Save the information to the stats vector of tuples double,string
          stats_vector stats_out;
          stats_vector stats_temp; //for temporary storrage
          
          
          if (ABMAT_INTERVAL_PANELS){
          stats_out.emplace_back(std::make_tuple(double(cur_interval+1),"Interval_ID"));
          }
          std::string obs_start = "Obs_start" + std::string(add_interval);
          std::string obs_end = "Obs_end" + std::string(add_interval);
          stats_out.emplace_back(std::make_tuple(double(start_time+1),obs_start));
          stats_out.emplace_back(std::make_tuple(double(end_time+1),obs_end));
                  
         /******************
           *                   MACRO STATISTICS        ************************/
          /* First, gather all the stats. Second, write the to the file */

          for (int i=0; i<Labels_macro.size();i++){

            //skip those only for comparitive reasons
            if (onlycompare_macro[i]){
              continue;
            }
          
            if (Initial_macro[i]){
              //save initial value
              stats_out.emplace_back(std::make_tuple(Data_macro[i].front(),Labels_macro_short[i] + "_fstV"));
            }
            if (Last_macro[i]){
              //save initial value
              stats_out.emplace_back(std::make_tuple(Data_macro[i].back(),Labels_macro_short[i] + "_lstV"));
            }
            if (cumulative_macro[i]){
              //save initial value
              double cumulative = 0.0;
              for (const auto& element : Data_macro[i]){
                cumulative += element;
              }
              stats_out.emplace_back(std::make_tuple(cumulative,Labels_macro_short[i] + "_cum"));
            }

            //Check if it is a constant. In this case, we may shorten the infos taken.
            if (Const_macro[i]) {
              double isConst=true;
              for (int j=0; j<Data_macro[i].size()-2;j++){
                if (Data_macro[i][j]!=Data_macro[i][j+1]){
                  isConst=false;
                  break;
                }
              }


              stats_out.emplace_back(std::make_tuple(isConst?1.0:0.0,Labels_macro_short[i] + "_isConst"));
            }
          
          //to do: if const, skip.
            if (Stats_macro[i]){          
              //8-Stats           
              stats_temp = calc_EightStats(&Data_macro[i][0], end_time+1, start_time);
              //adjust labels
              for (int l=0;l<stats_temp.size();l++){
                std::get<std::string>(stats_temp.at(l)) = Labels_macro_short[i] + "_" + std::get<std::string>(stats_temp.at(l)) + add_interval;
              }
              //add to general data frame
              stats_out.insert(std::end(stats_out),std::begin(stats_temp),std::end(stats_temp));
            }
            
            if (LMoments_macro[i]){
              //L-Moments
              stats_temp = calc_L_Moments(&Data_macro[i][0], end_time+1, start_time);
              //adjust labels
              for (int l=0;l<stats_temp.size();l++){
                std::get<std::string>(stats_temp.at(l)) = Labels_macro_short[i] + "_" + std::get<std::string>(stats_temp.at(l)) + add_interval;
              }
              //add to general data frame
              stats_out.insert(std::end(stats_out),std::begin(stats_temp),std::end(stats_temp));
            }             
            
            if (Runs_macro[i]){
              //Runs-Test
              stats_temp = calc_Runs_Stats(&Data_macro[i][0], end_time+1, start_time);
              //adjust labels
              for (int l=0;l<stats_temp.size();l++){
                std::get<std::string>(stats_temp.at(l)) = Labels_macro_short[i] + "_" + std::get<std::string>(stats_temp.at(l)) + add_interval;
              }
              //add to general data frame
              stats_out.insert(std::end(stats_out),std::begin(stats_temp),std::end(stats_temp));
            }
            if (RunsA_macro[i]){
              //Runs-Test ASYMMETRICAL
              stats_temp = calc_Runs_Stats(&Data_macro[i][0], end_time+1, start_time,false);
              //adjust labels
              for (int l=2;l<stats_temp.size();l++){ //tot and hi is already contained in symmetrical test, that is always run
                std::get<std::string>(stats_temp.at(l)) = Labels_macro_short[i] + "_" + std::get<std::string>(stats_temp.at(l)) + add_interval;
              }
              //add to general data frame
              stats_out.insert(std::end(stats_out),std::begin(stats_temp)+2,std::end(stats_temp));
            }

            //Get comparative stats
            for (auto compLabel : Compare_macro[i]){
              //find partner
              int partner;
              for (partner=0; partner< Labels_macro.size(); partner++){
                if (Labels_macro[partner] == compLabel){
                  break;
                }
              }
              if (Labels_macro[partner] != compLabel){
                PLOG("\nERROR! Partner '%s' for comparison with '%s' not available?!",compLabel.c_str(),Labels_macro[i].c_str());
                break;
              }

              stats_temp = calc_compare2(&Data_macro[i][0], &Data_macro[partner][0], end_time+1, start_time);
              //adjust labels
              for (int l=0;l<stats_temp.size();l++){
                std::get<std::string>(stats_temp.at(l)) = Labels_macro_short[i] + "_vs_" + Labels_macro_short[partner] + "_" + std::get<std::string>(stats_temp.at(l)) + add_interval;
              }
               //add to general data frame
              stats_out.insert(std::end(stats_out),std::begin(stats_temp),std::end(stats_temp));
            }
          }
          
          /******************
           *                INDIVIDUAL (Agent-lvl) STATISTICS       ***********/

          int indiv_end,indiv_start;
          for (int k=0; k<Labels_individual_short.size();k++){    //the individual vars      
  
            for(int i=0; i<Data_individual[k].size(); i++) { //the individual records for each var 
              //Adjust starting and ending, if necessary
              indiv_start=start_time;
              while(Data_individual[k][i][indiv_start]==NAN){
                indiv_start++;  
              }         
              indiv_end=end_time;
              while(Data_individual[k][i][indiv_end]==NAN){
                indiv_end--;  
              }
              
              //Add the stats
              
              if (Initial_individual[k]){
                //save initial value
                stats_out.emplace_back(std::make_tuple(Data_individual[k][i][0],Labels_individual_short[k] + "_" + std::to_string(i) + "_fstV"));
              }

              if (Last_individual[k]){
                //save initial value
                stats_out.emplace_back(std::make_tuple(Data_individual[k][i].back(),Labels_individual_short[k] + "_" + std::to_string(i) + "_lstV"));
              }

              //Check if it is a constant. In this case, we may shorten the infos taken.
              if (Const_individual[k]) {
                double isConst=true;
                for (int j=0; j<Data_individual[k][i].size()-2;j++){
                  if (Data_individual[k][i][j]!=Data_individual[k][i][j+1]){
                    isConst=false;
                    break;
                  }
                }

                stats_out.emplace_back(std::make_tuple(isConst?1.0:0.0,Labels_individual_short[k] + "_" + std::to_string(i) + "_isConst"));
              }
              
              if (Stats_individual[k]){
                //8-Stats
                stats_temp = calc_EightStats(&Data_individual[k][i][0], indiv_end, indiv_start); 
                //adjust labels
                for (int l=0;l<stats_temp.size();l++){
                  std::get<std::string>(stats_temp.at(l)) = Labels_individual_short[k] + "_" + std::to_string(i) + "_" + std::get<std::string>(stats_temp.at(l)) + add_interval;
                }
                //add to general data frame
                stats_out.insert(std::end(stats_out),std::begin(stats_temp),std::end(stats_temp));
              }
              
              if (LMoments_individual[k]){
                //L-Moments
                stats_temp = calc_L_Moments(&Data_individual[k][i][0], indiv_end, indiv_start); 
                //adjust labels
                for (int l=0;l<stats_temp.size();l++){
                  std::get<std::string>(stats_temp.at(l)) = Labels_individual_short[k] + "_" + std::to_string(i) + "_" + std::get<std::string>(stats_temp.at(l)) + add_interval;
                }
                //add to general data frame
                stats_out.insert(std::end(stats_out),std::begin(stats_temp),std::end(stats_temp));
              }
              
              if (Runs_individual[k]){
                //Runs-Test
                stats_temp = calc_Runs_Stats(&Data_individual[k][i][0], indiv_end, indiv_start); 
                //adjust labels
                for (int l=0;l<stats_temp.size();l++){
                  std::get<std::string>(stats_temp.at(l)) = Labels_individual_short[k] + "_" + std::to_string(i) + "_" + std::get<std::string>(stats_temp.at(l)) + add_interval;
                }
                //add to general data frame
                stats_out.insert(std::end(stats_out),std::begin(stats_temp),std::end(stats_temp));
              }
              if (RunsA_individual[k]){
                //Runs-Test ASYMMETRICAL
                stats_temp = calc_Runs_Stats(&Data_individual[k][i][0], indiv_end, indiv_start,false); 
                //adjust labels
                for (int l=2;l<stats_temp.size();l++){
                  std::get<std::string>(stats_temp.at(l)) = Labels_individual_short[k] + "_" + std::to_string(i) + "_" + std::get<std::string>(stats_temp.at(l)) + add_interval;
                }
                //add to general data frame
                stats_out.insert(std::end(stats_out),std::begin(stats_temp)+2,std::end(stats_temp));
              }
              
            }
          }
          
          /******************
           *                   MICRO STATISTICS       ************************/
          
          //The micro Statistics (agent-level but cross-section)          
          std::vector < std::string > micro_8_heads = get_EightStats_head(); //only get the heads from the EightStats.
          std::vector < std::string > micro_LM_heads = get_L_Moments_head(); //only get the heads from the LMoments.          
          
          std::string add_conditional=""; //info on conditional variable, if relevant

          for (int i=0; i<Labels_micro_short.size();i++){
  
            //check if conditional. In this case, adjust the label. The data is already adjusted (for each conditional is treated as separate entity)
            if (std::get<bool>(Conditional_micro.at(i)) ){
              add_conditional = "_" + std::get<3>(Conditional_micro.at(i)) + "_" + std::to_string(std::get<int>(Conditional_micro.at(i)));    //gretl does not like other symbols.
            }


            if (Stats_micro[i]) {
              for (int j=0; j<micro_8_heads.size(); j++){
                //8-Stats
                stats_temp = calc_EightStats(&Data_micro[i][j][0], end_time+1, start_time);
                //adjust labels
                for (int l=0;l<stats_temp.size();l++){
                  std::get<std::string>(stats_temp.at(l)) = Labels_micro_short[i] + add_conditional + "_" + std::get<std::string>(stats_temp.at(l)) + "_of_" + micro_8_heads[j] + add_interval;                 
                }
                //add to general data frame
                stats_out.insert(std::end(stats_out),std::begin(stats_temp),std::end(stats_temp));

              }
            }//(Stats_micro[i]) {;
                              
            if (LMoments_micro[i]) {
              for (int j=0; j<micro_LM_heads.size(); j++){
                //L-Moments
                stats_temp = calc_L_Moments(&Data_micro[i][j][0], end_time+1, start_time);
                //adjust labels
                for (int l=0;l<stats_temp.size();l++){
                  std::get<std::string>(stats_temp.at(l)) = Labels_micro_short[i] + add_conditional + "_" + std::get<std::string>(stats_temp.at(l)) + "_of_" + micro_LM_heads[j] + add_interval;                 
                }
                //add to general data frame
                stats_out.insert(std::end(stats_out),std::begin(stats_temp),std::end(stats_temp));

              }              
            }//(LMoments_micro[i]) {;
         
          }
          
          //Next, save the data to the files
                         
          //The first line for the header, the second for the data.
          for (int line=0; line<2; line++){

            for (int i = 0; i < stats_out.size(); i++ ) {                     
              if (i > 0){
                result_files[cur_interval] << "\t";
              }
              if (line == 0){
                  result_files[cur_interval] << std::get<std::string>(stats_out[i]);
              } else {
                  result_files[cur_interval] << std::get<double>(stats_out[i]); 
              }
            }
            result_files[cur_interval] << "\n"; //Finished row
            if (line==1) {
              result_files[cur_interval].close();      //free everything from memory
            }
          }
        } //cur_interval end
        result_files.clear(); //free memory
          
          
        /**********************************************************************/
         /******************
           *                  TIME SERIES DATA         ************************/    
        
    {
#ifndef SUPPRESS_SUB_FOLDERS  //a mode with only one results folder for linux (in vm)
      //plog("\nABMAT :   Direct call to makePath");      
      if (!makePath(timeseries.c_str())){ //Base path
        plog("\nABMAT :   ERROR: makePath() for '");
        plog(timeseries.c_str());
        plog("' not working!");
        return false; 
      }  
      //First, create a sub-folder indicating the config id.
//       snprintf(timeseries,sizeof(char)*256,"%s/CID_%i",timeseries,CID);
      timeseries += "/CID_";
      timeseries += std::to_string(CID);
      //plog("\nABMAT :   Direct call to makePath");
      if (!makePath(timeseries.c_str())){ //Base path
        plog("\nABMAT :   ERROR: makePath() for '");
        plog(timeseries.c_str());
        plog("' not working!");
        return false; 
      }        
#else
//       snprintf(timeseries,sizeof(char)*256,"%s_CID_%i",timeseries,CID);
      timeseries += "_CID_";
      timeseries += std::to_string(CID);
#endif
            
      std::ofstream save_file2;
//       save_file2.setf(std::ios::fixed);
      save_file2.precision(std::numeric_limits<long double>::digits10 + 1); 
      //Next, cycle through time-series selected for individual saving and save.
      
         /******************
           *                   MACRO STATISTICS        ************************/
      
      std::string buffer;
      
      for (int i=0; i<Labels_macro.size();i++){
        //skip those only for comparitive reasons
          if (onlycompare_macro[i]){
            continue;
          }

        //Test if it shall be saved.
        bool cumSave  = CumTimeseries_macro[i];
        bool save     = Timeseries_macro[i];

        for (int toSaveCum = 0; toSaveCum < 2; toSaveCum++ ){
          if ( (save && toSaveCum==0) || (cumSave && toSaveCum==1) ){
  #ifndef SUPPRESS_SUB_FOLDERS  //a mode with only one results folder for linux (in vm)
  //           snprintf(buffer,sizeof(char)*256,"%s/%s.tsv",timeseries,Labels_macro[i].c_str());
            buffer = timeseries.c_str();
            buffer += "/";
            if (toSaveCum == 1){
              buffer += "Cum_";
            }
            buffer += Labels_macro[i].c_str();
  #else
            buffer = timeseries.c_str();
            buffer += "_";
            if (toSaveCum == 1){
              buffer += "Cum_";
            }
            buffer += Labels_macro[i].c_str();
  //           snprintf(buffer,sizeof(char)*256,"%s_%s.tsv",timeseries,Labels_macro[i].c_str());
  #endif
            buffer += "_macro";
            buffer += ".tsv";
            save_file2.open(buffer.c_str(),std::ios_base::out | std::ios_base::trunc);

            if (save_file2){
              char msg[256];
              if (toSaveCum == 1){
                snprintf(msg,sizeof(char)*256,"\nABMAT :   Saving results (%s ; ABMAT - cummulative time series, macro) in file : ",Labels_macro[i].c_str());
              } else {
                snprintf(msg,sizeof(char)*256,"\nABMAT :   Saving results (%s ; ABMAT - time series, macro) in file : ",Labels_macro[i].  c_str());
              }
              plog(msg);
              plog(buffer.c_str());
            } else {
              plog("\nABMAT :   Error: Could not open file for saving data.");
              return false;
            }

            //Create Name of Time-Series as header
            save_file2 << (toSaveCum==1?"Cum_":"") << Labels_macro[i].c_str() << "_CID_" << std::to_string(CID) << "\n";
            //Write data
            double cumulative = 0.0;
            for (int j=0; j<Data_macro[i].size();j++){
              if (toSaveCum==0){
                save_file2 << Data_macro.at(i).at(j) << "\n"; //.at() is more save than []
              } else {
                cumulative += Data_macro.at(i).at(j);
                save_file2 << cumulative << "\n"; //.at() is more save than []
              }
            }
            save_file2.close();
          }
        }
      }
      
          /******************
           *                INDIVIDUAL (Agent-lvl) STATISTICS       ***********/
      std::string indiv_path; 
      //Next individual series
      if (Labels_individual.size()>0){
        //Create folder for inidividual files.
#ifndef SUPPRESS_SUB_FOLDERS  //a mode with only one results folder for linux (in vm)        
//         snprintf(indiv_path,sizeof(char)*256,"%s/Individual",timeseries.c_str());
        indiv_path = timeseries.c_str();
        indiv_path += "/Individual"; 
        //plog("\nABMAT :   Direct call to makePath");
        if (!makePath(indiv_path.c_str())){ //Base path
          plog("\nABMAT :   ERROR: makePath() for '");
          plog(indiv_path.c_str());
          plog("' not working!");
          return false; 
        }           
#else
//         snprintf(indiv_path,sizeof(char)*256,"%s_Individual",timeseries.c_str());
        indiv_path = timeseries.c_str();
        indiv_path += "_Individual"; 
#endif        
      }
      
      //for each statistic
      for (int k=0; k<Labels_individual.size();k++){
        //Test if it shall be saved.
        if (Timeseries_individual[k]){

            //Save individuals as columns
  #ifndef SUPPRESS_SUB_FOLDERS  //a mode with only one results folder for linux (in vm)
            buffer = indiv_path.c_str();
            buffer += "/";
            buffer += Labels_individual[k].c_str();
  //           snprintf(buffer,sizeof(char)*256,"%s/%s.tsv",indiv_path,Labels_individual[k].c_str());
  #else
  //           snprintf(buffer,sizeof(char)*256,"%s_%s.tsv",indiv_path,Labels_individual[k].c_str());
            buffer = indiv_path.c_str();
            buffer += "_";
            buffer += Labels_individual[k].c_str();
  #endif
            buffer += "_indiv";
            buffer += ".tsv";
            save_file2.open(buffer.c_str(),std::ios_base::out | std::ios_base::trunc);

            if (save_file2){
              char msg[256];
              snprintf(msg,sizeof(char)*256,"\nABMAT :   Saving results (%s: %i-%i ; ABMAT - Timeseries, individual) in file : ",Labels_individual[k].c_str(),1,Data_individual[k].size());
              plog(msg);
              plog(buffer.c_str());
            } else {
              plog("\nABMAT :   Error: Could not open file for saving data.");
              return false;
            }

            //Create Header
            //for each individual
            for(int i=0; i<Data_individual[k].size();i++) {
  //             snprintf(buffer,sizeof(char)*256,"Indiv_%i",i+1);
              save_file2 << Labels_individual[k] << "_CID_" << CID << "_Ind_" << i+1;
              if (i<Data_individual[k].size()-1){
                save_file2 << "\t";
              } else {
                save_file2 << "\n";
              }
            }

            //Parse data
            //for each entry (row!) -- Note: Thise presumes that the columns are fo equal length (guaranteed)
            char tmp[256];
            sprintf(tmp,"\nABMAT :    Size of data is %i x %i (cols x rows)",Data_individual[k].size(),Data_individual[k][0].size());
            vlog(tmp);
            for(int row=0; row<Data_individual[k][0].size();row++) {
              //for each individual aka column
              for(int i=0; i<Data_individual[k].size();i++) {
                save_file2 << Data_individual.at(k).at(i).at(row);
                if (i<Data_individual[k].size()-1){
                  save_file2 << "\t";
                } else {
                  save_file2 << "\n";
                }
              }
            }

            save_file2.close();
          }
        }
      
          /******************
           *                   MICRO STATISTICS       ************************/
           
      std::vector < std::string > micro_8_heads = get_EightStats_head(); //only get the heads from the EightStats.
      std::vector < std::string > micro_LM_heads = get_L_Moments_head(); //only get the heads from the LMoments.
      for (int i=0; i<Labels_micro.size();i++){
      
            std::string add_conditional="";
            std::string add_conditional_lab="";
            //check if conditional. In this case, adjust the label. The data is already adjusted (for each conditional is treated as separate entity)
            if (std::get<bool>(Conditional_micro.at(i)) ){
              add_conditional = "(" + std::get<3>(Conditional_micro.at(i)) + "=" + std::to_string(std::get<int>(Conditional_micro.at(i))) + ")";
              add_conditional_lab = "_c_" + std::get<3>(Conditional_micro.at(i)) + "_" + std::to_string(std::get<int>(Conditional_micro.at(i)));
            }
      
        //Test if it shall be saved.      
        if (Timeseries_micro[i]){
          buffer = timeseries.c_str();
#ifndef SUPPRESS_SUB_FOLDERS  //a mode with only one results folder for linux (in vm)          
          buffer += "/";
#else
          buffer += "_";
#endif          
          buffer += Labels_micro[i];
          buffer += add_conditional;
          buffer += "_micro"; 
          buffer += ".tsv";        
//           snprintf(buffer,sizeof(char)*256,"%s/%s.tsv",timeseries,Labels_micro[i].c_str());      
          save_file2.open(buffer.c_str(),std::ios_base::out | std::ios_base::trunc);
          if (save_file2){
            std::string msg = "\nABMAT :   Saving results (" + Labels_micro[i] + add_conditional + " ; ABMAT - Timeseries, micro) in file : " + buffer;
            plog(msg.c_str());            
          } else {
            plog("\nABMAT :   Error: Could not open file for saving data.");
            return false;
          }
          
      //head 
          int column = 0;
          if (Stats_micro[i]) {                                           
            for (int k=0;k<micro_8_heads.size();k++){
              if (0<column++){ save_file2 << "\t"; }
              save_file2 << Labels_micro[i].c_str() << add_conditional_lab << "_" << CID << "_" << micro_8_heads[k].c_str();
            }            
          }          
          if (LMoments_micro[i]) {                                
            for (int k=0;k<micro_LM_heads.size();k++){
              if (0<column++){ save_file2 << "\t"; }
              save_file2 << Labels_micro[i].c_str() << add_conditional_lab << "_" << CID << "_" << micro_LM_heads[k].c_str();
            }            
          }

          save_file2 << "\n";
                
      //data
          for (int j=0; j<Data_micro[i][0].size();j++){
            column = 0;
            if (Stats_micro[i]) {
              for (int k=0;k<micro_8_heads.size();k++){
                if (0<column++){ save_file2 << "\t"; }              
                save_file2 << Data_micro.at(i).at(k).at(j); //.at() is more save than []
              }
            }
            if (LMoments_micro[i]) {
              for (int k=0;k<micro_LM_heads.size();k++){
                if (0<column++){ save_file2 << "\t"; }              
                save_file2 << Data_micro_lmoments.at(i).at(k).at(j); //.at() is more save than []
              }
            }
            save_file2 << "\n";
          }      
          
          save_file2.close();
        }
      }
      
    }
    if (!mark_sim_as_done(CID)){
      plog("\nABMAT :   ERROR: Some problem with mark_sim_as_done() in save_stats()");
    }
    #ifdef ABMAT_DOE_H
      if (std::remove(in_process_name(CID).c_str())!=0){
      plog("\nABMAT :   ERROR: Could not remove in progress marker!");
      }
    #endif
     
    return clear_Data(); //important, otherwise memory leaks          
  }
  
    //Save the config from LSD to the std::vector.
  bool grab_Parameters_LSD(object * cur_r) //For use without DOE, grab par vals from LSD according to Config Specification of Pars
  {
    plog("\nABMAT :   ---\nABMAT :   Grabbing parameter values of the current simulation run for later analysis.");
    plog("\nABMAT :   Parameter\tValue");
    variable * curr;
    object * cur;
    char buffer[256];
    Data_parameter.clear(); //clear old information    
    for (int i=0; i<Labels_parameter.size();i++){
      //Search the object holding the variable with the label
      curr = cur_r->search_var(cur_r,Labels_parameter[i].c_str());
      cur = (object *)curr->up;
      //USE Lsd Macro to read value and save it back.          
      Data_parameter.push_back(VS(cur,Labels_parameter[i].c_str()));
      snprintf(buffer,sizeof(char)*256,"\nABMAT :   %s\t%g",Labels_parameter[i].c_str(),VS(cur,Labels_parameter[i].c_str()));
      plog(buffer);      
    }
    plog("\nABMAT :   -----\nABMAT :   ");
    return true;    
  }

}
                                                                        
