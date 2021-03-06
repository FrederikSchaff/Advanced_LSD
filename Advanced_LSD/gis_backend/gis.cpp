/*************************************************************
                                                    May 2018
  gis.cpp : see gis.h for infos

  LSD Geography module - backend for LSD (least 7.0)
  written by Frederik Schaff, Ruhr-University Bochum

	Copyright Frederik Schaff
  This code is distributed under the GNU General Public License

 *************************************************************/

#include "gis.h"

// /**
//  *define statics
//  */
//
//  object* ext_gis::LSD_counterpart;
//  char ext_gis::patch_label[ MAX_ELEM_LENGTH ];
//  bool ext_gis::wrap_left,ext_gis::wrap_right,ext_gis::wrap_top,ext_gis::wrap_bottom;
//  int ext_gis::xn,ext_gis::yn;
//  std::vector<std::vector<ext_gis_patch>>  ext_gis::patches;
//
// /****/

  ext_gis::ext_gis(object* counterpart, char const *_patch_label, int _xn, int _yn, int wrap){
  /* Initialise the GIS and create the patches in the backend. The linking
     to the LSD objects happens later, using an LSD function. This makes it,
     e.g., possible to have a sparse matrix in LSD (not all patches exist). In
     general it is tried to keep the backend as detached from LSD as possible.

     An extension (later) is possible, such that the size of the lattice can
     grow endogenously.

     It might be a good idea to use the EIGEN library than, which can deal with
     sparse matrixes and is included in LSD already.
  */
    snprintf(patch_label,	MAX_ELEM_LENGTH, "%s",_patch_label); //copy the label
    LSD_counterpart=counterpart;
    xn=_xn;
    yn=_yn;
    code_Wrap=wrap;
    /*wrapping: there are 2^4 options. We use a bit-code (0=off):
    0-bit: left     : 0=0 1=1
    1-bit: right    : 0=0 1=2
    2-bit: top      : 0=0 1=4
    3-bit: bottom   : 0=0 1=8
    */
    if (wrap>7){wrap_bottom=true; wrap-=8;} else {wrap_bottom=false; }
    if (wrap>3){wrap_top=true;    wrap-=4;} else {wrap_top=false;    }
    if (wrap>1){wrap_right=true;  wrap-=2;} else {wrap_right=false;  }
    if (wrap>0){wrap_left=true;           } else {wrap_left=false;   }



          VERBOSE_MODE(true){
            PLOG("\nGeography Model :   Initialising lattice of patches '%s'",patch_label);
            PLOG("\nGeography Model :   Lattice size is %i (%i x %i)",xn*yn,xn,yn);
            PLOG("\nGeography Model :   Wrapping bit-code translates to:");
            PLOG("\nGeography Model :   wrap-left: %i, wrap-right: %i, wrap-top %i, wrap-bottom %i",wrap_left,wrap_right,wrap_top,wrap_bottom);

          }

    // create alle the elements in the lattice, uninitialised
    patches.clear();
    // WRONG // patches.reserve(xn*sizeof(std::vector<ext_gis_patch,const yn));
    patches.resize(xn);           //performance: Better reserve all space necessary. don't know how as the vec already exists.
    for (int x=0; x<xn; x++){
      patches.at(x).resize(yn);
    }

    ext_gis_patch *ptrPatch;
    ext_gis_patch *ptrPatch_last;
    int ID = 0;
    for (int x=0; x<xn; x++){
      patches.at(x).resize(yn);
      for (int y=0; y<yn; y++){
        ptrPatch = &patches.at(x).at(y);
        ptrPatch->ID = ID++;
        ptrPatch->x = x;
        ptrPatch->y = y;

        //link up/down/left/right
        if      (x>0)         { ptrPatch->left  = &patches.at(x-1).at(y);   }
        else if (wrap_left)   { ptrPatch->left  = &patches.at(xn-1).at(y);  }

        if      (x<xn-1)      { ptrPatch->right = &patches.at(x+1).at(y);   }
        else if (wrap_right)  { ptrPatch->right = &patches.at(0).at(y);     }

        if      (y>0)         { ptrPatch->down    = &patches.at(x).at(y-1);   }
        else if (wrap_bottom) { ptrPatch->down    = &patches.at(x).at(yn-1);  }

        if      (y<yn-1)      { ptrPatch->up  = &patches.at(x).at(y+1);   }
        else if (wrap_top)    { ptrPatch->up  = &patches.at(x).at(0);     }

        if (!(x==0 && y==0 )){
          ptrPatch_last->next = ptrPatch; //except for last in row, add "next"
        }
        ptrPatch_last = ptrPatch;

      }
    }

    //link those elements using the left / right / up / down pointers.
    //would a function be more performant? This is more flexible, we can break some links
    TEST_MODE(true && xn<5 && yn<5){ //checked and ok
      PLOG("\nGeography Model :   Testing lattice. Format: (x,y |left,right,up,down)\n");
      ext_gis_patch *ptrPatch;
      for (int y=yn-1; y>=0; y--){
        for (int x=0; x<xn; x++){
            ptrPatch = &patches.at(x).at(y);
            PLOG("(%i,%i |%i,%i,%i,%i) -",ptrPatch->x,ptrPatch->y,ptrPatch->left!=NULL,ptrPatch->right!=NULL,ptrPatch->up!=NULL,ptrPatch->down!=NULL);
        }
        PLOG("\n");
      }
    }



}

/*
    Movement.
*/

//Simply returns the patch in direction, if it exists. Von Neuman lattice.
ext_gis_patch* ext_gis::move_single(ext_gis_patch* pos, char direction){ //move "u"p, "d"own, "r"ight or "l"eft, if possible. else return NULL.
  switch (direction) {

    case 'd': return pos->down;
    case 'D': return pos->down;

    case 'l': return pos->left;
    case 'L': return pos->left;

    case 'u': return pos->up;
    case 'U': return pos->up;

    case 'r': return pos->right;
    case 'R': return pos->right;

  }
  PLOG("\nERROR Geography Model :   ext_gis::move(): Wrong kind of input '%c'. Only one of 'dDlLuUrR' allowed.",direction);
  return NULL;
}

//moves through a number of steps provided as string of characters, as long as it is valid.
//Stops if no more movement is possible, returning last position in reach (default) or NULL (complete=true)
ext_gis_patch* ext_gis::move(ext_gis_patch* pos, const std::string& direction, bool complete){
  ext_gis_patch* last_pos;
  for (auto dir : direction){
    last_pos = pos;
    pos = move_single(pos, dir);
    if (pos==NULL){
      if (!complete){
        return last_pos; //Part of movement possible
      } else {
        return NULL; //Not all movement possible
      }
    }
  }
  return pos; //All movement possible
}

object* ext_gis::move_LSD(int x, int y, const std::string& direction, bool complete){ //move "u"p, "d"own, "r"ight or "l"eft, if possible. else return NULL.
  TEST_MODE(true){ //Allow turning off for reasons of performance
    if (x<0 || y<0 || x>= xn || y >= yn){
      PLOG("\nERROR Geography Model :   ext_gis::move_LSD(): Error, start position (%i,%i) is out of range",x,y);
      return NULL;
    }
  }

  ext_gis_patch* newPos = move(&patches.at(x).at(y), direction, complete);
  if (newPos == NULL){ return NULL; } else {return newPos->LSD_counterpart;}
}

/*
    Radius Search
*/



void ext_gis_rsearch::init_ssimple(bool to_sort){
  //we make it rather simple. we specify the extreme boarders, defined by the
  //centered rectangle with radius (around the center) and als
  //checking for wraping.
  //then we calculate the distances for each potential candidate in this subset

  //define border. Negative numbers yet allowed, for they simplify distance
  //calculations

  //05.07.2018 15:00:38 Here was an error. I need to keep the distance
  //(if wrapping ok) but change the coords to allow look-up

  //25-june-18
  // also check that in case of wrapping each point is tested once.
  // and that we do not end up in non-valid places when no wrapping.

  //13-august-18
  // added wrapping considered in distance calculations for list. - this was buggy

  //added option to search complete
  const bool verbose_mode=false;
  VERBOSE_MODE(verbose_mode){
    PLOG("\nGeography Model :   ext_gis_rsearch::init_ssimple(): Defining Set.");
  }

  //In
  int xn = target->xn - 1;          //note: Locally without offset!
  int x0 = 0;
  int x = origin.x;

  int yn = target->yn - 1;
  int y0 = 0;
  int y = origin.y;


  bool wrap_left = target->wrap_left;
  bool wrap_right = target->wrap_right;
  bool wrap_top = target->wrap_top;
  bool wrap_bottom = target->wrap_bottom;
  int code_Wrap = transcode_Wrap(wrap_left,wrap_right,wrap_top,wrap_bottom);

    //check if everything is covered first
  //this is a simplified check mainly intended to find per se a compl. search
  //initialised with radius = -1
  int adj_radius = (int) ceil(radius);
  if (radius >= max(target->xn,target->yn)*2) {
//     PLOG("\nCHECK: Creating complete search object");  //checked
    for (int tx=x0; tx <= xn; tx++) {
      for (int ty=y0; ty <= yn; ty++) {
        valid_objects.emplace_back(tx,ty,geo_pseudo_distance(tx,ty,x,y,code_Wrap,xn,yn)); //add to valid objects
      }
    }

    //alternatively, use square approximation to gather superset of options,
    //add valid objects from this superset
    //also, takes care of wrapping conditions
  } else {

    //x_l is left without wrapping, x_l_w>x0 is with potential wrapping
    int x_l = x - adj_radius;
    int x_l_w = x0; //off default - this value is impossible
    if (x_l<x0) {
      x_l_w = xn - (x_l + x0)-1;
      x_l = x0;
    }
    int x_r = x + adj_radius;
    int x_r_w = xn; //off default
    if (x_r>xn) {
      x_r_w = x0 + (x_r - xn)-1;
      x_r = xn;
    }

    int y_b = y - adj_radius;
    int y_b_w = y0; //off default
    if (y_b<y0) {
      y_b_w = yn - (y_b + y0)-1;
      x_l = x0;
    }
    int y_t = y + adj_radius;
    int y_t_w = yn; //off default
    if (y_t>yn) {
      y_t_w = x0 + (y_t - yn)-1;
      y_t = yn;
    }

    //tried with set and failed...


    //Add inner points
    int checker = 0;
    for (int tx=x_l;tx<=x_r;tx++){
      for (int ty=y_b;ty<=y_t;ty++){
//         valid_objects.emplace_back(ext_gis_coords(tx,ty,geo_pseudo_distance(tx,ty,x,y,code_Wrap,xn,yn) )); //add to valid objects
        if (std::find_if(valid_objects.begin(),valid_objects.end(),[&tx,&ty](auto const &t1){
            return std::tie(t1.x,t1.y)==std::tie(tx,ty); //sort ascending in distance
          })==valid_objects.end()){
          int pd = geo_pseudo_distance(tx,ty,x,y,code_Wrap,xn,yn);
          if (pd <= adj_radius) {
            valid_objects.emplace_back(tx,ty,double(pd)); //add to valid objects
          }
        }
      }
    }

    //if wrapping to left, add points
    if (wrap_left == true && x_l_w != x0){
      for (int tx=x_l_w;tx<=xn;tx++){
        for (int ty=y_b;ty<=y_t;ty++){
          if (std::find_if(valid_objects.begin(),valid_objects.end(),[&](auto const &t1){
              return std::tie(t1.x,t1.y)==std::tie(tx,ty); //sort ascending in distance
            })==valid_objects.end()){
            int pd = geo_pseudo_distance(tx,ty,x,y,code_Wrap,xn,yn);
            if (pd <= adj_radius) {
              valid_objects.emplace_back(tx,ty,double(pd)); //add to valid objects
            }
          }
        }
      }
    }

    //if wrapping to right, add points
    if (wrap_right == true && x_r_w != xn){
      for (int tx=x0;tx<=x_r_w;tx++){
        for (int ty=y_b;ty<=y_t;ty++){
          if (std::find_if(valid_objects.begin(),valid_objects.end(),[&](auto const &t1){
              return std::tie(t1.x,t1.y)==std::tie(tx,ty); //sort ascending in distance
            })==valid_objects.end()){
            int pd = geo_pseudo_distance(tx,ty,x,y,code_Wrap,xn,yn);
            if (pd <= adj_radius) {
              valid_objects.emplace_back(tx,ty,double(pd)); //add to valid objects
            }
          }
        }
      }
    }

    if (wrap_top == true && y_t_w != y0) {
      for (int tx=x_l;tx<=x_r;tx++){
        for (int ty=y0;ty<=y_t_w;ty++){
          if (std::find_if(valid_objects.begin(),valid_objects.end(),[&](auto const &t1){
              return std::tie(t1.x,t1.y)==std::tie(tx,ty); //sort ascending in distance
            })==valid_objects.end()){
            int pd = geo_pseudo_distance(tx,ty,x,y,code_Wrap,xn,yn);
            if (pd <= adj_radius) {
              valid_objects.emplace_back(tx,ty,double(pd)); //add to valid objects
            }
          }
        }
      }
    }

    if (wrap_bottom == true && y_b_w != yn) {
      for (int tx=x_l;tx<=x_r;tx++){
        for (int ty=y_b_w;ty<=yn;ty++){
          if (std::find_if(valid_objects.begin(),valid_objects.end(),[&](auto const &t1){
              return std::tie(t1.x,t1.y)==std::tie(tx,ty); //sort ascending in distance
            })==valid_objects.end()){
            int pd = geo_pseudo_distance(tx,ty,x,y,code_Wrap,xn,yn);
            if (pd <= adj_radius) {
              valid_objects.emplace_back(tx,ty,double(pd)); //add to valid objects
            }
          }
        }
      }
    }

//
//
//
//
//
//
//
//
//
//     //Out
//
//     //default is complete x/y
//     int x_l = x0;
//     int x_r = xn;
//     if (adj_radius < xn*2){   //check if everything is covered by default.
//
//       x_l = x - adj_radius;
//       if (!wrap_left && x_l < x0){ //if no wrapping allowed
//         x_l = x0;
//       }
//
//       x_r = x + adj_radius;
//       if (!wrap_right && x_r > xn){ //if no wrapping allowed
//         x_r = xn;
//       }
//
//       //only relevant for wrapping, check that nothing is covered more than once
//       if (x_l < x0 && (xn - (-x_l) ) <= x_r ){
//         x_l = - (xn - x_r - 1); //end one right of x_r
//       }
//
//       //only relevant for wrapping, check that nothing is covered more than once
//       if (x_r > xn && (x_r - xn)  >= x_l){
//         x_r = xn + x_l - 1; //end one left of x_l
//       }
//
//     }
//
//     int y_d = y0;
//     int y_u = yn;
//     if (adj_radius < yn*2){
//       int y_d = y - adj_radius;
//       if (!wrap_bottom && y_d < y0){
//         y_d = y0;
//       }
//
//       int y_u = y + adj_radius;
//       if (!wrap_top && y_u > yn){
//         y_u = yn;
//       }
//
//       //only relevant for wrapping, check that nothing is covered more than once
//       if (y_d < y0 && ( yn - (- y_d) ) <= y_u ){
//         y_d = - (yn - y_u - 1); //end one above of x_u
//       }
//
//       //only relevant for wrapping, check that nothing is covered more than once
//       if ( y_u > yn && (y_u - yn) >= y_d){
//         y_u = yn + y_d - 1; //end one below of y_d
//       }
//
//     }
//
//     //next, create a list of the potential candidates
//     //05.07.2018 15:03:30 But change negative coords (wrapping checked before)
//     int total = 0;
//     for (int tx=x_l; tx<=x_r; tx++){ //by column
//       for (int ty=y_d; ty<=y_u; ty++){ //elements in column
//         total++; //checked
//         int t_pd = geo_pseudo_distance(x,y,tx,ty,code_Wrap,xn,yn);
//         if (t_pd<=pseudo_radius){
//           if (tx<x0){
//             tx = xn + tx; //temp.x < 0!
//           } else if (tx>xn-1){
//             tx -= xn;
//           }
//           if (ty<y0){
//             ty = yn + ty;
//           } else if (ty>yn-1){
//             ty -= yn;
//           }
//           valid_objects.push_back(ext_gis_coords(tx,ty,t_pd) ); //add to valid objects
//   //         PLOG("\n ADDING an item.");
//         } else {
//   //         PLOG("\n NOT Adding an item");
//         }
//       }
//     }
//     VERBOSE_MODE(true){
//       PLOG("\n there are in %i options valid from %i checked.",valid_objects.size(),total);
//     }
  }

  if (to_sort==true){
    std::sort(std::begin(valid_objects),std::end(valid_objects));
   }

}


object* ext_gis::LSD_by_coords(int x, int y){ //returns the corresponding LSD patch, if it exists
  if(x<0 || y < 0 || x > xn-1 || y > yn-1){
    PLOG("\nERROR Geography Model :   ext_gis:: LSD_by_coords : wrong x/y: %i,%i!",x,y);
    return NULL;
  }
  return patches.at(x).at(y).LSD_counterpart;
}

object* ext_gis::LSD_by_coords(ext_gis_coords in){ //returns the corresponding LSD patch, if it exists
  return ext_gis::LSD_by_coords(in.x,in.y);
}
object* ext_gis::LSD_by_coords(coords in){ //returns the corresponding LSD patch, if it exists
  return ext_gis::LSD_by_coords(in.x,in.y);
}
object* ext_gis::LSD_by_coords(int_coords in){ //returns the corresponding LSD patch, if it exists
  return ext_gis::LSD_by_coords(in.x,in.y);
}

double geo_distance(double x_1, double y_1, double x_2, double y_2, int wrap, double xn, double yn){
  double temp = geo_pseudo_distance (x_1,y_1,x_2,y_2, wrap, yn, xn);
  return temp>0?sqrt(temp):0.0;
}

double geo_distance(ext_gis_coords a, ext_gis_coords b, int wrap, double xn, double yn){
  return geo_distance((double) a.x, (double) a.y, (double) b.x, (double) b.y, wrap, yn, xn);
}

double geo_distance(int_coords a, int_coords b, int wrap, double xn, double yn){
  return geo_distance((double) a.x, (double) a.y, (double) b.x, (double) b.y, wrap, yn, xn);
}

double geo_distance(coords a, coords b, int wrap, double xn, double yn){
  return geo_distance(a.x, a.y, b.x, b.y, wrap, yn, xn);
}

//Better: Make a template function of it!
double geo_pseudo_distance(double x_1, double y_1, double x_2, double y_2, int _wrap, double xn, double yn){
/* Calculate the pseudo distance between to patches of land using Pythagorean Theorem
  For wrapping, it is assumed that one wants to move from x_1 to x_2.
  yn and xn are inclusive here.
  y0 and x0 are assumed to be 0.
*/

  double a_sq = x_1-x_2;
  double b_sq = y_1-y_2;

  if (_wrap > 0){
    Wrap_OLD wrap(_wrap); //translate wrapping info
    if (wrap.right && x_1>x_2){
      double alt_a = xn - x_1 + x_2;
      if (alt_a < a_sq){
        a_sq=alt_a;
      }
    } else if (wrap.left) {
      double alt_a = xn - x_2 + x_1;
      if (alt_a < a_sq){
        a_sq=alt_a;
      }
    }

    if (wrap.top && y_1 > y_2){
      double alt_b = yn - y_1 + y_2;
      if (alt_b < b_sq){
        b_sq=alt_b;
      }
    } else if (wrap.bottom){
      double alt_b = yn - y_2 + y_1;
      if (alt_b < b_sq){
        b_sq=alt_b;
      }
    }
  }

  a_sq *= a_sq;
  b_sq *= b_sq;
  return a_sq + b_sq;
}
int geo_pseudo_distance(int_coords a, int_coords b, int _wrap, int yn, int xn){
  return geo_pseudo_distance(a.x, a.y, b.x, b.y, _wrap, yn, xn);
}

int geo_pseudo_distance(int x_1, int y_1, int x_2, int y_2, int _wrap, int yn, int xn){
/* Calculate the pseudo distance between to patches of land using Pythagorean Theorem
  For wrapping, it is assumed that one wants to move from x_1 to x_2.
  yn and xn are inclusive here.
  y0 and x0 are assumed to be 0.
*/

  int a_sq = x_1-x_2;
  int b_sq = y_1-y_2;

  if (_wrap > 0){
    Wrap_OLD wrap(_wrap); //translate wrapping info
    if (wrap.right && x_1>x_2){
      int alt_a = xn - x_1 + x_2;
      if (alt_a < a_sq){
        a_sq=alt_a;
      }
    } else if (wrap.left) {
      int alt_a = xn - x_2 + x_1;
      if (alt_a < a_sq){
        a_sq=alt_a;
      }
    }

    if (wrap.top && y_1 > y_2){
      int alt_b = yn - y_1 + y_2;
      if (alt_b < b_sq){
        b_sq=alt_b;
      }
    } else if (wrap.bottom){
      int alt_b = yn - y_2 + y_1;
      if (alt_b < b_sq){
        b_sq=alt_b;
      }
    }
  }

  a_sq *= a_sq;
  b_sq *= b_sq;
  return a_sq + b_sq;
}


double geo_pseudo_distance(coords a, coords b, int wrap, double xn, double yn){
  return geo_pseudo_distance((double) a.x, (double) a.y, (double) b.x, (double) b.y, wrap, yn, xn);
}

object* ext_gis_rsearch::next(){
  //a simple iterator through the vector of pointers initialised before
  //if agents=true is given, provide the list of LSD objects currently linked
  //to the patch.
  VERBOSE_MODE(false){
    PLOG("\nGeography Model :   ext_gis_rsearch::next(): Producing next LSD object.");
    PLOG("\n total options: %i",valid_objects.size());
  }
  if (it_valid == valid_objects.end()){
    VERBOSE_MODE(false){
      PLOG("\n We have tested all %i valid objects.",valid_objects.size());
    }
    return NULL;
  } else
  {
    last = target->LSD_by_coords(*it_valid);
//     last_distance = it_valid->distance();  //no need to calculate it
    VERBOSE_MODE(false){
      PLOG("\nOption is %i,%i, distance %g",it_valid->x,it_valid->y,it_valid->distance());
      PLOG("\nLSD Counterpart is %s, %g,%g, id %g",last->label,GET_VAR(last,"_x"),GET_VAR(last,"_y"),GET_ID(last));
    }
    it_valid++;
  }
  return last;
}

void ext_gis_patch::add_LSD_agent(object* obj_to_add){
  VERBOSE_MODE(false){
    PLOG("\nGeography Model :   : ext_gis_patch::add_LSD_agent : Currently there are %i objects at patch w. ID %i. Now adding object %s with ID %g",LSD_agents.size(),ID,obj_to_add->label,GET_ID(obj_to_add));
  }
  LSD_agents.push_back(obj_to_add);
}

bool ext_gis_patch::remove_LSD_agent(object* obj_to_remove){
  //https://stackoverflow.com/a/26567766/3895476
  {
    VERBOSE_MODE(false){
      PLOG("\nGeography Model :   : ext_gis_patch::remove_LSD_agent : Currently there are %i objects at patch w. ID %i. Now removing object %s with ID %g",LSD_agents.size(),ID,obj_to_remove->label,GET_ID(obj_to_remove));
    }
    auto it = std::find(LSD_agents.begin(), LSD_agents.end(), obj_to_remove);
    if (it != LSD_agents.end()) {
      VERBOSE_MODE(false){
        PLOG("\n\t... success");
      }
      LSD_agents.erase(it);
      return true;
    }
  }
//   for (int i=0; i<=LSD_agents.size();i++){
//     if (LSD_agents[i] == obj_to_remove){
//       LSD_agents.erase(LSD_agents.begin()+i);
//       return true;
//     }
//   }
    PLOG("\nGeography Model :   : ext_gis_patch::remove_LSD_agent :  ERROR! Did not find this object under the associated ones!");
  return false; //Obj was not associated to patch
}

coords ext_gis::random_position(){ //produce a random position on the lattice
  return coords(random_x(),random_y() );
}
int ext_gis::random_y(){ //produce a random position on the lattice
  return uniform_int(0,yn-1);
};
int ext_gis::random_x(){ //produce a random position on the lattice
  return uniform_int(0,xn-1);
};

ext_gis_patch* ext_gis::get_patch_at(int x, int y){
  if (x >= 0 && x < xn && y>=0 && y < yn ){
    return &patches.at(x).at(y);
  } else {
    PLOG("\nERROR Geography Model :   ext:gis::get_patch_at : Dimensions supplied are wrong. (%i,%i) is outside of boundary 0 <= y < %i and 0 <= x <= %i!",x,y,xn,yn);
    return NULL;
  }
}

// Utility to associate locations to LSD agent objects

object* ext_gis::LSD_obj_pos_init(object* LSD_obj, int x, int y){
  if (x==-1 || y==-1){
    PLOG("\nERROR Geography Model :   ext_gis:: LSD_obj_pos_init : Wrong x/y values: %i,%i",x,y);
    return NULL;
  }
  ext_gis_patch* ptr_GIS_patch = get_patch_at(x,y);
  if (ptr_GIS_patch == NULL){
    PLOG("\nERROR Geography Model :   ext_gis:: LSD_obj_pos_init : There is no such patch!");
    return NULL; //this patch does not exist
  } else { //add LSD obj.
    ptr_GIS_patch->add_LSD_agent(LSD_obj);
    return ptr_GIS_patch->LSD_counterpart;
  }
}

object* ext_gis::LSD_obj_pos_move(int x_orig, int y_orig, object* LSD_obj, int x_new, int y_new){
  ext_gis_patch* ptr_GIS_patch_orig = get_patch_at(x_orig,y_orig);
  if (x_new==-1){
    x_new=random_x();
  }
  if (y_new==-1){
    y_new=random_y();
  }
  ext_gis_patch* ptr_GIS_patch_new = get_patch_at(x_new,y_new);
  if (ptr_GIS_patch_orig == NULL){
    PLOG("\nERROR Geography Model :   ext_gis::LSD_obj_pos_move : Origin not found!");
    return NULL; //this patch does not exist
  } else if (ptr_GIS_patch_new == NULL){
    PLOG("\nERROR Geography Model :   ext_gis::LSD_obj_pos_move : New Pos not found!");
    return NULL; //this patch does not exist
  } else {
    if (! ptr_GIS_patch_orig->remove_LSD_agent(LSD_obj)){ //remove and check if success
      PLOG("\nERROR Geography Model :   ext_gis:: LSD_obj_pos_move : could not remove old association!");
      return NULL;
    } else {
      ptr_GIS_patch_new->add_LSD_agent(LSD_obj);
      return ptr_GIS_patch_new->LSD_counterpart;
    }
  }
}

object* ext_gis::LSD_obj_pos_remove(int x, int y, object* LSD_obj){
    ext_gis_patch* ptr_GIS_patch = get_patch_at(x,y);
    if (! ptr_GIS_patch->remove_LSD_agent(LSD_obj)){ //remove and check if success
      PLOG("\nERROR Geography Model :   ext_gis:: LSD_obj_pos_move : could not remove the association!");
      return NULL;
    } else {
      return ptr_GIS_patch->LSD_counterpart;
    }
}

