/* ========================================================================== */
/*                                                                            */
/*   ABMAT_StatTools.cpp                                                      */
/*   (c) 2018 F. Schaff                                                       */
/*                                                                            */
/*   Contains the methods to compute statistics, platform independent         */
/*                                                                            */
/* ========================================================================== */


#include "ABMAT_StatTools.h"

namespace ABMAT //a general namespace for the ABMAT tools
{

  bool is_const_dbl(const double x[], int start, int end, unsigned int interval_size) {    
    for (int i = 0; i< end; i++){
      if ( !isWithinPrecisionInterval(x[i],x[end],FLOAT_PREC_INTVL) ){ //always check against last to mitigate problem of trend.
        return false;  
      } 
    }
    return true;
  }

  double SQUARE(double x){
	 return x*x;
  }

  double ABS(double x){
  	return x>=0.0?x:-x;
  }

  std::vector <std::string> get_L_Moments_head(){ //helper function to get the head.
    double *temp;
    stats_vector tmp = calc_L_Moments(temp,0,0,true);
    std::vector <std::string> head;
    for (int i = 0; i< tmp.size(); i++){
      head.push_back(std::get<std::string>(tmp[i]) );
    }
    return head;
  }

  stats_vector calc_L_Moments(const double Data_in[], int n, int start, bool report_head_only){
     /* Calculate L-moment ratios [1] as in the Fortran Routine in [2]:
  
      [1] Hosking, J. R. M. (1990): L-Moments: Analysis and Estimation of
          Distributions Using Linear Combinations of Order Statistics.
          In: Journal of the Royal Statistical Society. Series B
          (Methodological) 52 (1), S. 105–124.
      [2] Wang, Q. J. (1996): Direct Sample Estimators of L Moments.
          In: Water Resour. Res. 32 (12), S. 3617–3619.
          
      In: Data Array Out: results as vector of tuples <value, label>     
     */
     
     /* Improve readability via reference variables */
    double L1;         
    double L2;
    double L3;
    double L4;
    
    double L_cv;
    double L_sk;
    double L_ku;
    
    int len_data = n-start;
     
    if (len_data>1 && !report_head_only){ //check that more than one value are present
    
      size_t tempData_size = len_data;
      double tempData[tempData_size];
      for (int i = 0; i < len_data;i++){
      tempData[i]=Data_in[i+start];
      }                                /* See Vogel and Fennessey 1993 */
    
      /* First, sort descending */
  
  
      std::sort(tempData,tempData+tempData_size);
      /* Next calculate Ls directly without betas*/
      L1=0;
      L2=0;
      L3=0;
      L4=0;
      double CL1,CL2,CL3,CR1,CR2,CR3;
      for (int i = 1; i<=len_data; i++){
        CL1 = i-1;
        CL2 = CL1*(i-1-1)/2;
        CL3 = CL2*(i-1-2)/3;
        CR1 = len_data-i;          //buggy!
        CR2 = CR1*(len_data-i-1)/2;
        CR3 = CR2*(len_data-i-2)/3;
        L1 += tempData[i-1];
        L2 += (CL1-CR1)*tempData[i-1];
        L3 += (CL2-2*CL1*CR1+CR2)*tempData[i-1];
        L4 += (CL3-3*CL2*CR1+3*CL1*CR2-CR3)*tempData[i-1];
      }
      double C1 = (double) len_data;
      double C2 = C1*(len_data-1)/2;
      double C3 = C2*(len_data-2)/3;
      double C4 = C3*(len_data-3)/4;
    
      L1 = L1/C1;
      L2 = L2/C2/2;
      L3 = L3/C3/3;
      L4 = L4/C4/4;
      
      /* Note by [2:3618]  Director estimators of L  moments as implemented 
      by  the FORTRAN subroutine listed  here require more  com-
      putation  than  the  indirect  estimatorso  A more  efficient  algo-
      rithm  is  possible by making use  ofmck  =  mcm-k•  but com-
      putation  time  is  rarely a problern  for  typical  flood  frequency 
      applica tionso 
      */
      
      /* L1 is mean */
      L_cv = (L1 == 0.0) ? 0.0 : L2/L1; //L-cv
      L_sk = (L2 == 0.0) ? 0.0 : L3/L2; //L-Skewness
      L_ku = (L2 == 0.0) ? 0.0 : L4/L2; //L-Kurtosis        
    } else {
      L1 = NADBL;
      L_cv = NADBL;
      L2 = NADBL;
      L_sk = NADBL;
      L_ku = NADBL;
    } 
    stats_vector L_Moments;
    L_Moments.emplace_back(std::make_tuple(L1, "L1" ));
    L_Moments.emplace_back(std::make_tuple(L_cv, "L_cv" ));
    L_Moments.emplace_back(std::make_tuple(L2, "L2" ));
    L_Moments.emplace_back(std::make_tuple(L_sk, "L_sk" ));
    L_Moments.emplace_back(std::make_tuple(L_ku, "L_ku" ));
    
    return L_Moments;
  }
  
   stats_vector calc_Runs_Stats(const double Data_in[], int n, int start, bool symmetrical){
    /* On the individual time series level, a run is a series of runs above/below 
    the mean. In a first step the time series is re-coded as -1/+1 series. In a 
    next step it is transformed, either symmetrically (-1/+1 as change) or 
    asymmetrically (-1/+1 with distinct meaning), depending on the choice. Then,
    statistics of the runs-distribution are gathered.
    
    In the symmetrical case, a run of "-1" of length 4 is the equivalent of a run
    of "1" of length "4" - both add to the "count" of runs of length 4.
    
    In the asymmetrical case, runs of "-1" are counted as negative and runs 
    of "1" as positive first. Then, we use this information to "split" the 
    distribition in a "below mean" and ">= mean" part.
    
    NOTE: The runs-stats returns a tuple pointer (vector <double> vector 
      <string>) for the statistic and its post-script 
      
    */          
    stats_vector Runs_Stats; //temporary construct although maybe passed via move internally / through optimisation    
    int len_data = n - start;
  	/* It is important we make a local copy, because the data is shifted */
    if (len_data>1){ //check that more than one value are present
      
      // Calculate the runs statistic of the input data.
      std::vector < double > Runs;
      
        //calculate the mean to differentiate between -1/head/success and 1/tail/fail            
      double mean=0;           
    	for (int i = 0; i < len_data; i++){    		
        mean+=Data_in[start+i];
    	}
      mean /= (double) (len_data);
        //derive the (a)symmetrical runs statistic
      int success = Data_in[start]<mean?-1.0:1.0;
      int former;
      double count=0.0;
      double head_runs = 0.0;                     
      for (int i = 1; i < len_data; i++){
        count++; //count is the length of the last run, at least 1.
        former = success;
        success=Data_in[start+i]<mean?-1.0:1.0;
        if (success != former || i==len_data-1){
          if (!symmetrical){
            //If not symmetrical, -1/heads/success/below mean is treated as 
            //  negative length
            count *= success; 
          }
          Runs.push_back(count);
          if (former == -1) {
            head_runs++; //number of runs below mean
          }
          count=0.0; //reset count to zero
        }
      }
      
      //Save information on the number of runs
      int Runs_tot = Runs.size();
      Runs_Stats.emplace_back(std::make_tuple(double(Runs_tot),"Runs_tot"));
      Runs_Stats.emplace_back(std::make_tuple(Runs_tot-head_runs,"Runs_hi"));
      
      std::sort(Runs.begin(),Runs.end()); //sort ascending
      
//       //Only for inspection
//       std::cout << "The runs data" << std::endl;
//       char msg[64];
//       int j;
//       for (int i = 0; i < Runs_tot;){      
//         for (j = i; j < i+4 && j < Runs_tot; j++) {
//         snprintf(msg,sizeof(char)*64,"%-10g\t", Runs[j]);
//         std::cout << msg;
//         }
//         std::cout <<  std::endl;
//         i=j;
//         if (j>10 && j < Runs_tot-10) {
//           std::cout << "... (total : " << Runs_tot << ")" << std::endl;
//           j=Runs_tot-10;
//           i=j;
//         }
//       }
      
      int first = 0;      
      std::string interscript = "";
      if (!symmetrical){
        interscript ="hi_";
        //find position where the "runs" change to negative, to split it.
        //we save the index of the position that is first in "positive" / >= mean"
        while (first<Runs_tot) {
          if (Runs.at(first)>0){ //> would is sufficient for a run is at least of length 1
            break;
          }
          Runs.at(first)*=-1; //make it positive
          first++;
        }  
      }
      
      for (int twice=0;twice<2;twice++){      
      
        //Calculate statistics
        stats_vector temp = calc_EightStats(&Runs[0],Runs_tot, first);      
        for (int i=0;i<temp.size();i++){
          Runs_Stats.emplace_back(std::make_tuple(std::get<double>(temp.at(i)), ("Runs_" + interscript + std::get<std::string>(temp.at(i)) ) ) );  
        }                                          
        
        temp = calc_L_Moments(&Runs[0],Runs_tot, first);      
        for (int i=1;i<temp.size();i++){ //skip L1 as contained in 8stats
          Runs_Stats.emplace_back(std::make_tuple(std::get<double>(temp.at(i)), ("Runs_" + interscript + std::get<std::string>(temp.at(i)) ) ) );  
        }
       
        if (symmetrical || twice == 1){
          break; //only once
        } else {
           Runs_Stats.emplace_back(std::make_tuple(Runs_tot-first-1,"Runs_lo"));
           interscript = "lo_";
           Runs_tot=first+1;
           first=0;
            
        } 
      }
      
    }
    return Runs_Stats; //to do!
  
  }

  std::vector <std::string> get_EightStats_head(){ //helper function to get the head.
    double *temp;
    stats_vector tmp = calc_EightStats(temp,0,0,true);
    std::vector <std::string> head;
    for (int i = 0; i< tmp.size(); i++){
      head.push_back(std::get<std::string>(tmp[i]) );
    }
    return head;
  }

  stats_vector calc_EightStats(const double Data_in[], int n, int start, bool report_head_only){
  /* Calculate some basic stats:
  	minV = minVimum
  	maxV = maximum
  	lq = (.25) order stat (floor)
  	median =(.5) order stat OR avg of floor(.5) + ceil(.5)
  	uq = (.75) order stat (ceil)
  	mean = mean
  	SD =  standard deviation
  	MAD = mean absolute deviation
  
  	The outputs are provided as pointers (via &minv=EIGHT[0], e.g.)
   Input: array of double valued data, Data_in[]
  				int n: length of the array
  				int start: the first point of data that shall be used
  					(default would be zero)
            
    Optional: Only report the sequence and labels of the stats taken.
  */
   
    double minV;
    double lq;    
    double median;
    double uq;    
    double maxV;  
    double mean;  
    double RMSE;  
    double MAE;   
    
    int len_data = n - start;
  	/* It is important we make a local copy, because the data is shifted */
    if (len_data>1 && !report_head_only){ //check that more than one value are present
      
      size_t Data_size = len_data;
    
    	double Data[Data_size];           
    	for (int i = 0; i < len_data; i++){
    		Data[i]=Data_in[start+i];
    	}
    
      std::sort(Data,Data+Data_size);
    	minV = Data[0];
    	maxV = Data[len_data-1];
    	int index = (int) (len_data/4);
    	lq = Data[index];
    	index = (int) (len_data*3/4);
    	uq = Data[index];
    
    	if (len_data%2 == 0){
    		index = (int) len_data/2-1;
    		median = (Data[index]+Data[index+1])/2;
    	} else {
    		median = Data[(int)(len_data-1)/2];
    	}
    
    	mean = 0.0;
    	for (int i=0; i<len_data;i++){
    		mean += Data[i];
    	}
    	mean /= (double)(len_data);
    	MAE = 0.0;
    	RMSE = 0.0;
    	for (int i=0; i<len_data;i++){
    		MAE += ABS(Data[i]-mean);
    		RMSE += SQUARE(Data[i]-mean);
    	}
    	MAE /= (double)len_data;
    	RMSE /= (double)len_data;
    	RMSE=RMSE > 0? sqrt(RMSE) : 0.0;
    } else {
      minV = NADBL;
      maxV = NADBL;
      lq = NADBL;
      uq = NADBL;
      median = NADBL;
      mean = NADBL;
      MAE = NADBL;
      RMSE = NADBL;
  	}
    
    stats_vector Eight_Stats;
    Eight_Stats.emplace_back(std::make_tuple(minV, "min" ));
    Eight_Stats.emplace_back(std::make_tuple(lq, "lq" ));
    Eight_Stats.emplace_back(std::make_tuple(median, "med" ));
    Eight_Stats.emplace_back(std::make_tuple(uq, "uq" ));
    Eight_Stats.emplace_back(std::make_tuple(maxV, "max" ));
    Eight_Stats.emplace_back(std::make_tuple(mean, "mean" ));
    Eight_Stats.emplace_back(std::make_tuple(MAE, "MAD" ));
    Eight_Stats.emplace_back(std::make_tuple(RMSE,"sd" ));
    
    return Eight_Stats; 
  }


  stats_vector calc_assoc(const double a[], const double b[], int n, int start){
  /* Calculates the simple gamma and tau correlation coefficients (more formaly
    "association", as only the direction is considered, not the magnitude). */
    
    double elements = n-start;
    double gamma; //discard ties
    double tau_a,tau_b,tau_c;   //correct for ties
    stats_vector gamma_out;

   if (elements < 2) {
	   /* not enough data */
      plog("\nNot enough data for calc_xcorr!");
      return gamma_out; //empty
    }
    
    if ( !is_const_dbl(a, start, n-1) && !is_const_dbl(b, start, n-1) ) {
   
      double concordant = 0.0, discordant = 0.0, tie = 0.0, tie_a = 0.0, tie_b = 0.0, tie_ab = 0.0;
      
      for (int i = start; i< n-1; i++){
        
        if ( isWithinPrecisionInterval(a[i],a[i+1],FLOAT_PREC_INTVL) || isWithinPrecisionInterval( b[i],b[i+1],FLOAT_PREC_INTVL) ) {
          continue; //skip this one    
        } else if ( (a[i]>a[i+1] && b[i]>b[i+1] ) || (a[i]<a[i+1] && b[i]<b[i+1] ) ) {
          concordant++;
        } else if ( (a[i]>a[i+1] && b[i]<b[i+1] ) || (a[i]<a[i+1] && b[i]>b[i+1] ) )  {
          discordant++;
        } else {
          //A tie: within a or b only, or between a and b (can also be within a and/or b)
          bool t_a = a[i]==a[i+1];
          bool t_b = b[i]==b[i+1];
          if (t_a){
            tie_a++;
          }
          if (t_b){
            tie_b++;
          }
//           if (t_a && t_b){
//             tie_ab++;
//           }
//           tie++; //unspecified ties
        }   
                
      }
      
      
      if (concordant+discordant != 0){
          double S = (concordant-discordant);
          gamma = S / (concordant + discordant) ;
          tau_a = S / ( (elements) * (elements-1) );
          tau_b = S /  sqrt(  double(concordant +  discordant + tie_a) * double(concordant +  discordant + tie_b)   );
          //tau_c
      } else {
          gamma = 0.0;
          tau_a = 0.0;
          tau_b = 0.0;
      }
      gamma_out.emplace_back(std::make_tuple(gamma, "gamma" ));
      gamma_out.emplace_back(std::make_tuple(tau_a, "tau_a" ));
      gamma_out.emplace_back(std::make_tuple(tau_b, "tau_b" ));
//       gamma_out.emplace_back(std::make_tuple(tau_c, "tau_c" ));
    } else {
      gamma_out.emplace_back(std::make_tuple(NADBL, "gamma" ));
      gamma_out.emplace_back(std::make_tuple(0.0, "tau_a" ));
      gamma_out.emplace_back(std::make_tuple(0.0, "tau_b" ));
    }
  
    return gamma_out;
  }
  
  
  /* Standard product moment correlation, analogue to gretls gretl_corr */ 
  stats_vector calc_xcorr (const double x[], const double y[], int n, int start){
      double elements = n-start;
      double sx, sy, sxx, syy, sxy, den, xbar, ybar;
      double cval;
      stats_vector cval_out;
  
      if (elements < 2) {
  	   /* not enough data */
        plog("\nNot enough data for calc_xcorr!");
        return cval_out; //empty
      }
          
      if (is_const_dbl(x, start, n-1) || is_const_dbl(y, start, n-1) ) {
      	/* correlation between any x and a constant is
      	   undefined */
        return cval_out; //empty
      }
  
      sx = sy = 0.0;
  
      for (int t=start; t<n; t++) {
        sx += x[t];
        sy += y[t];
      }
  
      xbar = sx / n;
      ybar = sy / n;
      sxx = syy = sxy = 0.0;
  
      for (int t=start; t<n; t++) {
  	    sx = x[t] - xbar;
  	    sy = y[t] - ybar;
  	    sxx += sx * sx;
  	    syy += sy * sy;
  	    sxy += sx * sy;
      }
  
      if (sxy != 0.0) {
        den = sxx * syy;
        if (den > 0.0) {    //should be unnecessary, for the series are not constant.
  	     cval = sxy / sqrt(den); //correlated (pos or neg)
        } else {
  	     cval = NADBL; //ndef
  	   }
      } else {
        cval = 0.0;  //Uncorrelated perfect
      }
  
      cval_out.emplace_back(std::make_tuple(cval, "xcor" ));
      return cval_out;
  }

  /* calculate l1 (MAE) and root of L2 (RMSE) norms */
  stats_vector calc_norm_diffs (const double x[], const double y[], int n, int start){
      double elements = n-start;
      double AE;
      double MAE=0.0, RMSE=0.0;

      for (int t=start;t<n; t++) {
        AE = x[t]-y[t];
        if (AE < 0){
          AE = -AE;
        }
        MAE += AE;
        RMSE += AE*AE;
      }

      MAE /= (elements);
      RMSE /= (elements);

      if (RMSE > 0){
        RMSE = sqrt(RMSE);
      }

      stats_vector errs;
      errs.emplace_back(std::make_tuple(MAE, "MAE" ));
      errs.emplace_back(std::make_tuple(RMSE, "RMSE" ));

      return errs;
  }

  stats_vector calc_compare2(const double x[], const double y[], int n, int start){
  /* Compare two time-series */

    stats_vector compare;
    stats_vector diffs = calc_norm_diffs(&x[0], &y[0], n, start);
    compare.insert(std::end(compare),std::begin(diffs),std::end(diffs));
    stats_vector xcorr = calc_xcorr(&x[0], &y[0], n, start);
    compare.insert(std::end(compare),std::begin(xcorr),std::end(xcorr));
    stats_vector assic = calc_assoc(&x[0], &y[0], n, start);
    compare.insert(std::end(compare),std::begin(assic),std::end(assic));

    return compare;
  }

  std::vector <std::string> get_compare2_head(){ //helper function to get the head.
    double tempa[2] = {1,2};
    double tempb[2] = {3,4};
    stats_vector tmp = calc_compare2(&tempa[0],&tempb[0],2,0);
    std::vector <std::string> head;
    for (int i = 0; i< tmp.size(); i++){
      head.push_back(std::get<std::string>(tmp[i]) );
    }
    return head;
  }

}//Namespace end
