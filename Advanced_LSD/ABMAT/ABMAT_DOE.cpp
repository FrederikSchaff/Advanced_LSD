#include "ABMAT_DOE.h"
namespace ABMAT {

/* ========================================================================== */
   /*********************
   *  The Tools to load the DOE 
   *********************/

  bool add_DPM(const std::string& Path, const std::string& Name, int checkID, const std::string& Mode){
    if (!DPM_already_loaded){
      DPM_paths.push_back(Path.c_str()); 
      DPM_names.push_back(Name.c_str());
      DPM_modes.push_back(Mode.c_str());
      DPM_count++; 
      if(checkID != DPM_count){ 
        plog("\nABMAT_DOE :   ERROR, The Design Point Matrixes are not counted consequtively!");
        return false;
      }
      char buffer[128]; 
      snprintf(buffer,sizeof(char)*128,"\nABMAT_DOE :   .. (%i) '%s' from path '%s' with mode '%s'",\
        checkID,DPM_names[DPM_count-1].c_str(),DPM_paths[DPM_count-1].c_str(), DPM_modes[DPM_count-1].c_str());
      plog(buffer);          
    }
    return true;
  }
 
  bool load_DPM_cfg()
  {
    if (!DPM_already_loaded){
      //Reset
      DPM_paths.clear();
      DPM_names.clear();
      DPM_modes.clear();
      DPM_count=0;
      set_initialised = false;
      //Open DPM Config that points to single DPMs
      std::ifstream  DPM_CFG(ABMAT_DPM_CFG_PATH);      
      if (!DPM_CFG){
        plog("\nABMAT_DOE :   Error: The DPM Config File at path: ");
        plog(ABMAT_DPM_CFG_PATH);
        plog("' could not be opened!\nABMAT_DOE :   "); 
        return false;     
      } else {
        plog("\nABMAT_DOE :   Opened the DPM config file: '");
        plog(ABMAT_DPM_CFG_PATH);
        plog("'.\nABMAT_DOE :   Now adding DPMs ...");
      
        //Parse the information, skip comments.
        std::string line;
        while(safeGetline(DPM_CFG,line))
        {
          if (!(line.find("#")!=std::string::npos)&&!line.empty()){
            std::stringstream  lineStream(line);        
            std::string  path,name,id,mode;
            safeGetline(lineStream,path,',');
            safeGetline(lineStream,name,',');
            safeGetline(lineStream,id,',');
            safeGetline(lineStream,mode,',');
            if (strncmp(mode.c_str(),"single",6)==0) {
              if(!add_DPM(path, name, atoi(id.c_str()), mode)){
                return false;
              }
            } else {
              if (!add_DPM(path, name, atoi(id.c_str()))) { //default
                return false;
              }
            }           
          }
        }
        
        DPM_CFG.close();
      }
    } else {
      vlog("\nABMAT_DOE :   .. DPM configuration file already loaded / subsequent simulation.");
    }
    return true;
  }  
  

  bool load_DPM()
  {
    //Reset some vars
    use_grabbed_pars = false;
    Parameters_grabbed.clear();  
    if (DPM_choice == 0 || DPM_choice > DPM_count) {
      plog("\nABMAT_DOE :   Error! No valid DPM_path defined to load the DPM\nABMAT_DOE :   ");
      return false;
    } else {
      if (!DPM_already_loaded){
        single_mode = false; //default
        plog("\nABMAT_DOE :   Loading the Configuration Matrix: '");
        Config_Name = DPM_names[DPM_choice-1]; //Save
        plog(DPM_names[DPM_choice-1].c_str());
        plog("' from path '");
        plog(DPM_paths[DPM_choice-1].c_str());
        plog("' with mode '");
        plog(DPM_modes[DPM_choice-1].c_str());
        plog("'\nABMAT_DOE :   ");
        //Switch in case of single mode! 
        if (strncmp(DPM_modes[DPM_choice-1].c_str(),"single",6)==0){
          single_mode=true;
        }
        std::string filename_agg;     
//         char filename_agg[192];
//         snprintf(filename_agg,sizeof(char)*192,"%s%s%s%s",DPM_paths[DPM_choice-1].c_str(),"DPM_",DPM_names[DPM_choice-1].c_str(),".tsv");
        filename_agg = DPM_paths[DPM_choice-1].c_str();
        filename_agg += "DPM_";
        filename_agg += DPM_names[DPM_choice-1].c_str();
        if (single_mode){
          filename_agg += "_";
          filename_agg += std::to_string(Config_loaded);                //single file
        }
        filename_agg += ".tsv";    
        std::ifstream  fileP(filename_agg.c_str()); //Read File in tsv format, with header-line
        if (!fileP.is_open()){
          plog("\nABMAT_DOE :   Error: The specific DPM at path: ");
          plog(filename_agg.c_str());
          plog("' could not be opened!");
          return false;      
        } else {
          plog("\nABMAT_DOE :   Opened file : ");
          plog(filename_agg.c_str());
        }  
        //read the header
        std::string label;
        safeGetline(fileP, label, '\n'); 
        //create a std::string std::vector with the header
        std::istringstream gccNeedsThisOnASeperateLine{ label }; //http://www.cplusplus.com/reference/sstream/istringstream/istringstream/                                                                             
        std::vector<std::string> locLabels = { std::istream_iterator<std::string>{ gccNeedsThisOnASeperateLine }, std::istream_iterator<std::string>{} };         
        //Add it to the local file.
        Labels.clear(); //clear it first
        for (int i=0; i<locLabels.size();i++)
          Labels.push_back(locLabels[i]);
       
        //Read the remainer and parse it to a 2d std::vector of doubles
        Parameters.clear();//clear it first
        Processed_cids.clear();
        do {
      	    std::vector<double> input(Labels.size());
      	    for(int i = 0; i < input.size(); i++){ 
              fileP >> input[i];
            }
            if(!fileP.fail()) //control for empty line at end of file
      	     Parameters.push_back(input); 
             Processed_cids.emplace_back(int(input[0]),false); //add the ABMAT_CID and mark it as not processed.    
      	} while(fileP.ignore(std::numeric_limits<std::streamsize>::max(), '\n'));
      DPM_already_loaded = true;
      if (!create_results_folder()){
        return false;
      } //Set-Up
      } else {
      plog("\nABMAT_DOE :   .. DPM already loaded / subsequent simulation.");
      }
      return true; 
    }
  }
  
  bool load_processed_files(){
  /* Try if a file with information on processed files exists. If yes, use it.
     if not, go the (very!) slow way to check each file.
     
     To make it faster, there is a vector of tuples Processed_cids(int, bool) 
     that holds first the CID (increasing order) and second the status of being
     processed (false: Not yet, true: either now or in the current run, or (if
     not) successfully in the past). This vector is initialised when the config
     is loaded and updated every time when a config is processed (also not  
     necessary).
     
     To initialise the vector, an external file (via a pyhton script) is used, 
     because filesystem c++17 / experimental c++14 is not yet implemented in LSD
  */
    if (!processed_obs_loaded){ //guard, only make once when runnin
      processed_obs_loaded = true; //also if not possible, do not try again.
      std::string CIDs_processed = "Results/" + Config_Name + "/CIDs_processed.csv";
      vlog("\nPath to CIDs_processed is: "); vlog(CIDs_processed.c_str());
      std::ifstream processed(CIDs_processed.c_str());    
      if (processed.good() ){
      //Parse the information, skip comments.
        std::string line;
        while(safeGetline(processed,line))
        {
          if (!(line.find("#")!=std::string::npos)&&!line.empty()){
            std::stringstream  lineStream(line);
            std::string  buffer; 
            std::vector <int> Obs_info; //CID, intervals start, end (iteration)
            bool first = true;       
            while (safeGetline(lineStream,buffer, ',') ){

              if (!(buffer.find("#")!=std::string::npos)&&!buffer.empty()){
                try {
                  if (first)
                    Obs_info.push_back(std::stoi(buffer) ); //convert strings to int and add to obs
                  else
                    Obs_info.push_back(std::stoi(buffer) -1 ); //convert strings to int and add to obs, Intervals are internally storred with -1 (t_0 = 0)                                  
                }
                catch (...)  {
              	  plog("\nABMAT_DOE :   Error: An error occured in ABMAT.cpp:load_processed_files. Perhaps the CIDs_processed is not nice.\n");
                }
              }
              first = false;                            
            }

//             std::string buff = "\n";
//             for (int i=0;i<Obs_info.size();i++){buff += std::to_string(Obs_info[i]); buff += ",";}
//             plog(buff.c_str());

//             processed_obs.push_back(Obs_info); //add to database
            //see https://stackoverflow.com/a/37455203/3895476 for the following
            auto it = std::find_if(Processed_cids.begin(), Processed_cids.end(), [&Obs_info](const std::tuple<int,bool>& e) {return std::get<0>(e) == Obs_info[0];});
            if (it != Processed_cids.end()) {
//               plog("\n\nFound");
              //check intervals
              
              if (Obs_info[0]==get<0>(*it)) /*CID relevant*/ {
//                 plog("\n Found2");
                for (int i=1,j=0; i<Obs_info.size(); i+=2,j++) { //i cycles through Obs_info and j through Intervals
//                   std:string bbb="\nChecking if: ";
//                   bbb+= std::to_string(Obs_info[i]); bbb+= " is equal to ";
//                   bbb+= std::to_string(Intervals[j][0]);
//                   bbb+= " ; and if: ";
//                   bbb+= std::to_string(Obs_info[i+1]); bbb+= " is equal to ";
//                   bbb+= std::to_string(Intervals[j][1]);
//                   plog(bbb.c_str());
                   
                  if (Obs_info[i]!=Intervals[j][0] || Obs_info[i+1]!=Intervals[j][1]) {
//                     plog("\n\tIt is not");                    
                    continue; //check next option.
                  }
//                   plog("\n\tIt is indeed!");
                  get<1>(*it)=true; //if still fine, mark as already processed                  
                }
              }
              
            }
            
            //
          }
        }
        processed_obs_file=true;            
        processed.close(); //not necessary?!
      } else {
        processed_obs_file = false;        
      }        
    }
    return  processed_obs_file; //True if there exists a db-file of processed files and it has been loaded successfully
  }
  
  bool check_if_processed(int id){
  
    vlog("\nABMAT_DOE :   Checking if config was processed. Marker file is: ");
    if (load_processed_files()){
      //Check if the file is within the processed once
      auto it = std::find_if(Processed_cids.begin(), Processed_cids.end(), [&id](const std::tuple<int,bool>& e) {return std::get<0>(e) == id;});
            if (it != Processed_cids.end()) {
//               std:string buffer = "\nConfig "; buffer += std::to_string(get<0>(*it)); buffer+= " found and status is:"; buffer+= get<1>(*it)? "True" : "False";
//               plog(buffer.c_str());
              if (get<1>(*it)){
                return true;
              } //in case the file has been processed and this has been recognised in the CIDs_processed.csv, return true. Otherwise check manually if file is in _done_files. 
            } else {
              plog("\nABMAT_DOE :   Error: An error occured in ABMAT.cpp:check_if_processed(). The config running is not part of the DOE.\n");
              return false; 
            }    
    }
             
    
    vlog(sim_is_done_file(id).c_str());    
    std::ifstream infile(sim_is_done_file(id));    
    return infile.good();
    //https://stackoverflow.com/a/19841704/3895476
  }    

  
  
  //Load a specific config. Also load the Config File, if not yet done. 
  bool load_Config(object * cur_r, int ConfigID)
  {
    Config_loaded=ConfigID; //Note: This is the internal ID!    
    if (true) {//(!single_mode){
      if (!load_DPM()) {
        return false;
      }
      //Check that ConfigID matches
      if (ConfigID<=0){
        plog("\nABMAT_DOE :   Error: ConfigID provided < 0\nABMAT_DOE :   ");
        return false;
      } else if (!single_mode && ConfigID>Parameters.size()){
        plog("\nABMAT_DOE :   Error: ConfigID provided (");
        plog(std::to_string(ConfigID).c_str());
        plog(") > Number of Configs: ");
        plog(std::to_string(Parameters.size()).c_str());
        return false;
      }
      if (!single_mode){
        if (Parameters.at(ConfigID-1).at(0)!=ConfigID){
          plog("\nABMAT_DOE :    BE AWARE: The config ID does not match the internal id (run & set). This is fine IF an offset is present.");              
        }      
      }
      if(!mark_sim_as_in_process(Parameters.at(ConfigID-1).at(0))){
        return false; //This marker will be deleted in ABMAT.cpp after all results have been saved (more or less successful)
      }
      //Search for the LSD Object holding the Variable Label[i] and write the value
      //Parameter[ConfigID][i] in it, using the LSD macros.
      vlog("\nABMAT_DOE :   Loading the Parameter values:   ");
      variable * curr;
      object * cur;
      char buffer[164];
      for (int i=0; i<Labels.size();i++){
        int temp_index=0;
        if (!single_mode) {
          temp_index=ConfigID-1;
        }
        snprintf(buffer,sizeof(char)*164,"\nABMAT_DOE :   %s\t%10000.00000g",Labels[i].c_str(),Parameters[temp_index][i]);
        vlog(buffer); 
        curr = cur_r->search_var(cur_r,Labels[i].c_str());
        cur = (object *)curr->up;
        WRITES(cur,Labels[i].c_str(),Parameters[temp_index][i]);          
      }
      return true;
    }    
  }
  
  //Save the config from LSD to the std::vector.
  bool grab_Config(object * cur_r)
  {
    use_grabbed_pars = true; //set pars to be grabbed
    variable * curr;
    object * cur;    
    for (int i=0; i<Labels.size();i++){
      //Search the object holding the variable with the label
      curr = cur_r->search_var(cur_r,Labels[i].c_str());
      cur = (object *)curr->up;
      //USE Lsd Macro to read value and save it back.          
      Parameters_grabbed.push_back(VS(cur,Labels[i].c_str()));
    }
    return true;    
  }
  
  bool initialise_Set(object * cur_r, int SetID, int SetNum){
    if (!set_initialised) {
      if (!load_DPM()) //Load the config, if necessary
        return false;
      
      if (SetID > SetNum){
        plog("\nABMAT_DOE :   Error: The Number of sets is lower than the selected ID");
        return false;
      }
      
      int max_sets = floor(Parameters.size()/2.0); //At least one item per set
      if (SetNum > max_sets){
        plog("\nABMAT_DOE :   Error: Wrong number of sets. Make it at most: ");
        char buffer[32]; snprintf(buffer,sizeof(char)*32,"%i!\nABMAT_DOE :   ",max_sets);
        plog(buffer);    
        return false;
      }

      
      int max_n_per_set = ceil(Parameters.size()/double(SetNum));
      char buffer[128];
      snprintf(buffer,sizeof(char)*128,"\nABMAT_DOE :   Max n per set is: %i", max_n_per_set);
      plog(buffer);
        
      first_item_in_set = 1; //first config in firs set.
      
      //Shift to first config in selected set
      while (first_item_in_set < (SetID-1)*max_n_per_set){      
        first_item_in_set += max_n_per_set;
      }
      //Calculate the size of the set. Control for small last set
      cur_set_size = int( min(first_item_in_set+max_n_per_set-1,Parameters.size()) \
                          - (first_item_in_set-1) );
      last_item_in_set = first_item_in_set+cur_set_size-1;
      sim_num = cur_set_size; //Set the total number of simulations (LSD PAr) accordingly
      cur_item_in_set=first_item_in_set-1; //(Pre-)Initialise
      set_initialised=true;
    }
    return true;
  }
  
  //Load a specific config conditional on the set and the simulation number. 
  bool load_Config_via_Set(object * cur_r, int SetID, int SetNum)
  {
      //Check if set can be initialised successfully, if not return false, if subsequent return true
      if (!initialise_Set(cur_r, SetID, SetNum)){
        return false;
      }  
      
    
    cur_item_in_set++; //increase the simulation count       
    if (cur_item_in_set>last_item_in_set){
      plog("\n-----------------\nABMAT_DOE :    INFO: All items in the set have already been processed. Simulation sample now stopped.");
      quit = 2; //hard quit
      cur_sim = sim_num;
      return false;        
    }
    
    //Shift to correct config within set
    //tempConfigID += (ABMAT_cur_sim-1); //??
    
    /* With a loop, try to load the config. If it was already processed, skip to next. */
    int temp_test =  cur_item_in_set;      
    while (check_if_processed(cur_item_in_set)){
//       vlog("\nABMAT_DOE :    Skipping current Config (");
//       vlog(std::to_string(tempConfigID).c_str());
//       vlog("). It was already processed.");
      cur_item_in_set++;
      if (cur_item_in_set>last_item_in_set){
        plog("\n-----------------\nABMAT_DOE :    INFO: All configs in the set have already been processed. The sample is at an end.\n-----------------\n");     
        return false; //premature ending
      }      
    }
    if (temp_test < cur_item_in_set){
      std::string buffer = "\nABMAT_DOE :    INFO: Skipped configs " + std::to_string(temp_test) + " .. " + std::to_string(cur_item_in_set-1) + ". These are already done. Now doing: " + std::to_string(cur_item_in_set);
      plog(buffer.c_str());
    }
        
    
    if (!load_Config(cur_r, cur_item_in_set)){
      return false;
    }
    return true;
  }

/* ========================================================================== */
} //Namespace end