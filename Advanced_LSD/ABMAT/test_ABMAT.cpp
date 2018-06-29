/* ========================================================================== */
/*                                                                            */
/*   test_ABMAT.cpp                                                               */
/*   (c) 2012 Author                                                          */
/*                                                                            */
/*   Description                                                              */
/*                                                                            */
/* ========================================================================== */
#include <random>
//#include "../../src/fun_head_fast.h"
  #include "ABMAT_StatTools.cpp"
  
  #define TEST_DATA_N 100000

void print_stats(double data[], int n){
    char msg[64];
    ABMAT::stats_vector testL = ABMAT::calc_L_Moments(data,n);
    ABMAT::stats_vector test8 = ABMAT::calc_EightStats(data,n); 
    ABMAT::stats_vector test_runs = ABMAT::calc_Runs_Stats(data, n, 0, true);
    ABMAT::stats_vector test_runs_asym = ABMAT::calc_Runs_Stats(data, n, 0, false);
    
    std::cout << "The test data" << std::endl;
    int j;
    for (int i = 0; i < n;){      
      for (j = i; j < i+4 && j < n; j++) {
      snprintf(msg,sizeof(char)*64,"%-10g\t", data[j]);
      std::cout << msg;
      }
      std::cout <<  std::endl;
      i=j;
      if (i>50) {
        std::cout << "... (total : " << n << ")" << std::endl;
        break;
        }        
    }       
    
    std::cout << "A bunch of statistics" << std::endl;
    
    for (int i = 0; i < testL.size(); i++) {
      snprintf(msg,sizeof(char)*64,"%-10s %g", std::get<std::string>(testL[i]).c_str(), std::get<double>(testL[i]));    
      std::cout << msg <<  std::endl;
      std::get<std::string>(testL[i]) = std::get<std::string>(testL[i]) + "Test";
      std::cout << std::get<std::string>(testL[i]).c_str() << std::endl;
    }
    
    for (int i = 0; i < test8.size(); i++) {
      snprintf(msg,sizeof(char)*64,"%-10s %g", std::get<std::string>(test8[i]).c_str(), std::get<double>(test8[i]));
      std::cout << msg <<  std::endl;
    }
    
    for (int i = 0; i < test_runs.size(); i++) {
      snprintf(msg,sizeof(char)*64,"%-10s %g", std::get<std::string>(test_runs[i]).c_str(), std::get<double>(test_runs[i]));      
      std::cout << msg <<  std::endl;
    }
    
    std::cout << "And the asymmetric runs test" << std::endl;
    for (int i = 0; i < test_runs_asym.size(); i++) {
      snprintf(msg,sizeof(char)*64,"%-10s %g", std::get<std::string>(test_runs_asym[i]).c_str(), std::get<double>(test_runs_asym[i]));      
      std::cout << msg <<  std::endl;
    }
    
    std::cout << "Only print the head" <<  std::endl;
    
    std::vector < std::string > head = ABMAT::get_EightStats_head();
    for (int i = 0; i < head.size(); i++){
      std::cout << head[i] <<  std::endl;
    }
    
}



int main()
{
                                    
//create test dataset                                    
std::mt19937 mt_unif(43);
std::uniform_real_distribution<> unif(0, 1);
double test_DATA[TEST_DATA_N];
for (int i=0;i<TEST_DATA_N;i++){
  test_DATA[i] = unif(mt_unif);
}

print_stats(test_DATA,TEST_DATA_N);

  return 0;
}