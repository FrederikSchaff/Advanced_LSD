
/***************************************************
Advanced_LSD.cpp

****************************************************/

/** Declarations **/
std::string AdvLSD_FakeID_Label(object *callee);
std::string AdvLSD_FakeVar_Label(object *callee, const std::string& lab2);
double AdvLSD_FakeID(object *callee);
double AdvLSD_FakeVar(object *callee, const std::string& lab2);

/***********/

std::string AdvLSD_FakeID_Label(object *callee){
  TEST_IN(true)
    if (callee==NULL){
      PLOG("\nAdvanced_LSD.cpp      ERROR in AdvLSD_FakeID_Label(): pointer is NULL");
      return NULL;
    }
  TEST_OUT
    try{
      return std::string(std::string(callee->label)+"_ID");
    }
    catch (...){
        return "noLabelForCallee?";
    }
//   return std::string(std::string(callee->label)+"_ID");
}

//connect the callee label with a given string.
std::string AdvLSD_FakeVar_Label(object *callee, const std::string& lab2){
  TEST_IN(true)
    if (callee==NULL){
      PLOG("\nAdvanced_LSD.cpp      ERROR in AdvLSD_FakeVar_Label(): pointer is NULL");
      return NULL;
    }
  TEST_OUT
    try{
      return std::string(std::string(callee->label)+lab2);
    }
    catch (...){
        return "noLabelForCallee + lab?";
    }
//   return std::string(std::string(callee->label)+lab2);
}

double AdvLSD_FakeVar(object *callee, const std::string& lab2){
  return VS(callee,AdvLSD_FakeVar_Label(callee,lab2).c_str());
}

double AdvLSD_FakeID(object *callee){
  return VS(callee,AdvLSD_FakeID_Label(callee).c_str());
}

