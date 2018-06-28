/* ========================================================================== */
/*                                                                            */
/*   ABMAT_StatTools.cpp                                                      */
/*   (c) 2018 F. Schaff                                                       */
/*                                                                            */
/*   Contains the methods to compute statistics, platform independent         */
/*                                                                            */
/* ========================================================================== */

#ifndef ABMAT_STATTOOLS_H //guard
#define ABMAT_STATTOOLS_H  

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

#define NADBL std::numeric_limits<double>::max()     // A double not a number
#define FLOAT_PREC_INTVL 5 //the number of "steps" allowed between items to be "equal"

namespace ABMAT //a general namespace for the ABMAT tools
{

    /* Solving Floating Point Issues */
  // from: https://stackoverflow.com/a/41405501/3895476
  
    //implements ULP method
    //use this when you are only concerned about floating point precision issue
    //for example, if you want to see if a is 1.0 by checking if its within
    //10 closest representable floating point numbers around 1.0.
    template<typename TReal>
    static bool isWithinPrecisionInterval(TReal a, TReal b, unsigned int interval_size = 1)
    {
        TReal min_a = a - (a - std::nextafter(a, std::numeric_limits<TReal>::lowest())) * interval_size;
        TReal max_a = a + (std::nextafter(a, std::numeric_limits<TReal>::max()) - a) * interval_size;
    
        return min_a <= b && max_a >= b;
    }
  
  /*---------------------------*/
    bool is_const_dbl(const double x[], int start, int end, unsigned int interval_size = 1);


  typedef std::tuple < double, std::string > stats_tuple; //A new type for stats.
  typedef std::vector < stats_tuple > stats_vector; //To hold stats of some sort
  
  double ABS(double x);
  double SQUARE(double x); 

  std::vector <std::string> get_EightStats_head(); //helper function to get the head.
  std::vector <std::string> get_L_Moments_head(); //helper function to get the head.
      
  stats_vector calc_L_Moments   (const double Data_in[], int n, int start=0, bool report_head_only=false);
  stats_vector calc_Runs_Stats  (const double Data_in[], int n, int start=0, bool symmetrical=true);
  stats_vector calc_EightStats  (const double Data_in[], int n, int start=0, bool report_head_only=false);


  /* simple stats */
  stats_vector calc_assoc       (const double a[], const double b[], int n, int start=0);
  stats_vector xcalc_corr       (const double x[], const double y[], int n, int start=0);
  stats_vector calc_norm_diffs  (const double x[], const double y[], int n, int start=0);

  //all three
  stats_vector calc_compare2    (const double x[], const double y[], int n, int start=0);
  std::vector <std::string> get_compare2_head(); //helper function to get the head.
}
#endif
