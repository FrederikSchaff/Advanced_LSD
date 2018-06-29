/* The head file for the ABMAT project */

//First headers


#ifdef ABMAT_USE_ANALYSIS
  #include "ABMAT.h"
  #include "ABMAT_StatTools.h"
  
  #ifdef ABMAT_USE_DOE
    #include "ABMAT_DOE.h"
  #endif
  
  #ifdef ABMAT_USE_INDUCTIVE
    #include "ABMAT_Inductive.h"
  #endif
  
#endif



//Then content. This way the order of loading the headers is irrelevant. 

#ifdef ABMAT_USE_ANALYSIS
  #include "ABMAT.cpp"
  #include "ABMAT_StatTools.cpp"  

  #ifdef ABMAT_USE_DOE
    #include "ABMAT_DOE.cpp"
  #endif
  
  #ifdef ABMAT_USE_INDUCTIVE
    #include "ABMAT_Inductive.cpp"
  #endif  
  
#endif


 