/* ========================================================================== */
/*                                                                            */
/*   ABMAT_LSD.cpp                                                            */
/*   (c) 2016 frederik.schaff@fernuni-hagen.de                                */
/*                                                                            */
/*   Description                                                              */
/*                                                                            */
/* ========================================================================== */
#ifndef TRACK_SEQUENCE
  #define TRACK_SEQUENCE
  #define UNDEF_TRACK_SEQUENCE
#endif


#ifdef ABMAT_USE_ANALYSIS
EQUATION("ABMAT_INIT")
TRACK_SEQUENCE
/*  Initialise the ABMAT toolkit and loads the parameters from the design
    point matrix, if chosen in *.lsd model file. 
    
    Usage: Set 
    
    a) ABMAT_Switch = 0     Define alle parametervalues directly in LSD. Use LSD
                            seed, etc. ABMAT may still be used for analysis.
    b) ABMAT_Switch > 0     Load a single config from the design space.
    c) ABMAT_Switch < 0     Load the SET number -(ABMAT_Switch). In addition you
                            need to define the number of sets via ABMAT_Sets
    */  
  clock_gettime(CLOCK_MONOTONIC, &ABMAT::start);
  char buffer[128];
  if (!ABMAT::load_Intervals_cfg()){ //Load the intervals config file.    
    quit = 1;
    sim_num = 1;
    return (2.0);
  }
  if (ABMAT::Intervals_already_loaded){
#ifdef ABMAT_DOE_H 
      if(max_step > ABMAT::max_t_interval && V("ABMAT_Switch")<0){       
#else
      if(false){
#endif
        max_step = ABMAT::max_t_interval;
        plog("\nABMAT_LSD :   Reducing max_step according to max interval: ");
        plog(std::to_string(ABMAT::max_t_interval).c_str());            
      } else if(max_step < ABMAT::max_t_interval){
        #ifdef NO_WINDOW
        max_step = ABMAT::max_t_interval;
        plog("\nABMAT_LSD :   Increasing max_step according to max interval: ");
        plog(std::to_string(ABMAT::max_t_interval).c_str());
        #else
        plog("\nABMAT_LSD :   You need to increase max_step according to max interval: ");
        plog(std::to_string(ABMAT::max_t_interval).c_str());        
        quit=1;
        sim_num=1;
        return (2.0);
        #endif
      } 
    }  
  
  #ifdef ABMAT_DOE_H
  int ABMAT_Switch = int(V("ABMAT_Switch")); //The switch; 
  
  if (ABMAT_Switch==0){
    plog("\nABMAT_LSD :   Using manual parameter space (directly via LSD / *.lsd).");    
    WRITE("ABMAT_ConfigID",-seed+1); //the default, relavant for the results-file.
  
  } else {
    if (cur_sim==1) {
      plog("\nABMAT_LSD :   Using external Design Point Matrix .");
    }
    if (!ABMAT::load_DPM_cfg()) { //Load the design point matrix config file.
      sim_num = 1;
      quit = 1;
      return (2.0);
    }    
    //Specify the config    
    int temp_DPM=int(V("ABMAT_DPM"));
    if (cur_sim==1) {
      plog("\nABMAT_LSD :   Chosen Design Point Matrix is: ");
      plog(std::to_string(temp_DPM).c_str());
    }
    //Only if it was not yet initialised or differently (in interactive mode), 
    //set DPM Choice and reset DPM_already_loaded 
    if (temp_DPM != ABMAT::DPM_choice){
      ABMAT::DPM_already_loaded=false;
      ABMAT::set_initialised=false;
      ABMAT::DPM_choice=temp_DPM;
    }    
    int ABMAT_Sets = int(V("ABMAT_Sets"));
    
    //Load the config, which may entail the ABMAT Parameters.
    if (ABMAT_Switch>0){
      sim_num = 1; //Change number of sims (LSD Par) to 1.                     
      plog("\nABMAT_LSD :   Loading single config with CID : ");
      plog(std::to_string(ABMAT_Switch).c_str());
      if (!ABMAT::load_Config(p->up, ABMAT_Switch)){
        //Quit the process upon error.      
        quit = 1;
        return (2.0);
      }
      
      /* Check if config has already been processed. */
      if (ABMAT::check_if_processed(ABMAT_Switch)){
        plog("\nABMAT_LSD :    Note: The config was already processed with the same intervals. The ABMAT results will be re-written.");      
      }
    } else {
      if (cur_sim==1) {
        snprintf(buffer,sizeof(char)*128,"\nABMAT_LSD :   Loading Config from Set No %i/%i.",-ABMAT_Switch,ABMAT_Sets);
        plog(buffer);
      }

      if (!ABMAT::load_Config_via_Set(p->up, -ABMAT_Switch, ABMAT_Sets)){
        //Quit the process upon error or if all items already have been processed before.
        if (quit < 2){
          quit = 1;
          sim_num = 1;
        }        
        return (2.0);
      } else {
        //skipping now in AMBAT_DOE. As a consequence it is LSD specific... but it was anyway.
        cur_sim = ABMAT::cur_item_in_set-ABMAT::first_item_in_set+1; //Fix LSD sim-number to ABMAT sim-number. Important to trigger correct ending.
        snprintf(buffer,sizeof(char)*128,"\nABMAT_LSD :    ...Processing run %i/%i, config %i/%i.\nABMAT_DOE :   ",cur_sim,sim_num,ABMAT::Config_loaded,int(ABMAT::Parameters.size()));
        plog(buffer);
      }
    }
    //Write Back the choice CFG
    int temp_index = 0;
    if (!ABMAT::single_mode){
      temp_index = ABMAT::Config_loaded-1;
    }
    WRITE("ABMAT_ConfigID",ABMAT::Parameters[temp_index][0]);
    
    //Set ABMAT Parameters
    #ifdef ABMAT_USE_INDUCTIVE
      ABMAT::Par_MC_seq_tartgetPValue = V("ABMAT_SeqMC_Pv");
      ABMAT::Par_Besag_Clifford_Const = V("ABMAT_SeqMC_BCc");
      ABMAT::Par_MT_Seed = int(V("ABMAT_SeqMC_seed"));
    #endif

  }      
  #else
    plog("\nABMAT_LSD :   Only using ABMAT Analysis feature. DOE from LSD");
  #endif     
                            
  PARAMETER  //Only process once.
RESULT(0.0)

EQUATION("ABMAT_UPDATE")
/*
Update the ABMAT statistics and save them at the end of the run.
Can be called via fake_caller with callee NULL to save data without update.
*/  
TRACK_SEQUENCE


  if (t == 1){
    if (!ABMAT::load_Analysis_cfg()){ //Load the analysis config file.
      quit = 1;
      sim_num = 1;
      END_EQUATION(2.0);  //Preliminary end of equation, return 2.0
    }    
    #ifdef ABMAT_DOE_H
    ABMAT::grab_Config(p->up); //grab the configuration, overwriting parameter values.
    #else
    ABMAT::grab_Parameters_LSD(p->up);
    #endif 
  }

  if (c != NULL){
    if (!ABMAT::update_Data(p->up)){
      quit = 2;
      END_EQUATION(2.0);
    }
  }
  if (c == NULL || t == max_step || quit != 0){
  plog("\nABMAT LSD     :    ---------\nABMAT LSD     :    Simulation finished after ");
  plog(std::to_string(t).c_str());
  plog(" steps.");
  clock_gettime(CLOCK_MONOTONIC, &ABMAT::finish);  //Save time at end of sim
    #ifdef ABMAT_DOE_H
      ABMAT::save_stats(p->up);
    #else
      ABMAT::save_stats(NULL,seed-1);
    #endif
  }
RESULT(0.0)

#else
EQUATION("ABMAT_UPDATE")
PARAMETER
RESULT(0.0)
EQUATION("ABMAT_INIT")
PARAMETER
RESULT(0.0)
#endif

#ifdef UNDEF_TRACK_SEQUENCE
  #undef TRACK_SEQUENCE
#endif
