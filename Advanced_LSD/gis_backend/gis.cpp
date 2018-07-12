/*************************************************************
                                                    May 2018
  LSD Geography module - backend for LSD (least 7.0)
  written by Frederik Schaff, Ruhr-University Bochum

  for infos on LSD see ...

	Copyright Frederik Schaff
  This code is distributed under the GNU General Public License

  The complete package has the following files:
  [0] readme.md         ; readme file with instructions and information
                          on the underlying model.
  [1] fun_templ_geo.cpp ; a template file for the user model, containing the
                          links to the population model.
  [2] fun_LSD_geo.cpp   ; contains the LSD Equations for the population model.
  [3] backend_geo.h     ; contains the c++ declarations and new macros.
  [4] backend_geo.cpp   ; contains the c++ core code for the pop backend.
  [5] backend_compability.h ; helper to link with other modules.


  The package relies on LSD debug module by
    F. Schaff. For further informations see: ...

 *************************************************************/

/***************************************************
backend_geo.cpp

This file contains the core code of the population backend.
****************************************************/

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



          VERBOSE_IN(true)
            PLOG("\nGeography Model :   Initialising lattice of patches '%s'",patch_label);
            PLOG("\nGeography Model :   Lattice size is %i (%i x %i)",xn*yn,xn,yn);
            PLOG("\nGeography Model :   Wrapping bit-code translates to:");
            PLOG("\nGeography Model :   wrap-left: %i, wrap-right: %i, wrap-top %i, wrap-bottom %i",wrap_left,wrap_right,wrap_top,wrap_bottom);

          VERBOSE_OUT

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
    TEST_IN(true && xn<5 && yn<5) //checked and ok
      PLOG("\nGeography Model :   Testing lattice. Format: (x,y |left,right,up,down)\n");
      ext_gis_patch *ptrPatch;
      for (int y=yn-1; y>=0; y--){
        for (int x=0; x<xn; x++){
            ptrPatch = &patches.at(x).at(y);
            PLOG("(%i,%i |%i,%i,%i,%i) -",ptrPatch->x,ptrPatch->y,ptrPatch->left!=NULL,ptrPatch->right!=NULL,ptrPatch->up!=NULL,ptrPatch->down!=NULL);
        }
        PLOG("\n");
      }
    TEST_OUT



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
  TEST_IN(true) //Allow turning off for reasons of performance
    if (x<0 || y<0 || x>= xn || y >= yn){
      PLOG("\nERROR Geography Model :   ext_gis::move_LSD(): Error, start position (%i,%i) is out of range",x,y);
      return NULL;
    }
  TEST_OUT

  ext_gis_patch* newPos = move(&patches.at(x).at(y), direction, complete);
  if (newPos == NULL){ return NULL; } else {return newPos->LSD_counterpart;}
}

/*
    Radius Search
*/

void ext_gis_rsearch::init(ext_gis* _target, ext_gis_coords _origin, double _radius, int _type){
  VERBOSE_IN(false)
    PLOG("\nGeography Model :   ext_gis_rsearch::ext_gis_rsearch(): Initialising");
    PLOG("\n target LSD Object holding lattice is: %s", _target->LSD_counterpart->label);
    PLOG("\n Search is centered at: %i, %i and radius is: %g.",_origin.x,_origin.y,_radius);
    PLOG("\n Type of search is %i",_type);
  VERBOSE_OUT

  target = _target;
  origin = _origin;
  radius = _radius;
  type = _type;
//   last = NULL;
//   last_distance = -1;

  valid_objects.clear(); //clear list
  switch (type) {

    case 0: init_ssimple(); break;
    case 1: init_ssimple(true); break;//sorted
    default: init_ssimple(); break; //if wrong argument is supplied. add check later

  }
  VERBOSE_IN(false)
    PLOG("\nGeography Model :   ext_gis_rsearch::ext_gis_rsearch(): Initialising done. Check.");
    PLOG("\n There are %i options.",valid_objects.size());
  VERBOSE_OUT
  it_valid  = valid_objects.begin(); //initialise the iterator used in next()
  VERBOSE_IN(false && valid_objects.size()>0)
    PLOG("\n First option is located at (%i, %i) with distance to origin: %g",it_valid->x,it_valid->y,it_valid->distance);
  VERBOSE_OUT
}

void ext_gis_rsearch::init(ext_gis* _target, int _origin_x, int _origin_y, double _radius, int _type){
  ext_gis_coords _origin(_origin_x,_origin_y);
  init(_target, _origin, _radius, _type);
}

ext_gis_rsearch::ext_gis_rsearch(ext_gis* _target, int _origin_x, int _origin_y, double _radius, int _type){
  init(_target, _origin_x, _origin_y, _radius, _type);
}

ext_gis_rsearch::ext_gis_rsearch(ext_gis* _target, ext_gis_coords _origin, double _radius, int _type){
  init(_target,_origin,_radius,_type);
}



void ext_gis_rsearch::init_ssimple(bool sorted){
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

  VERBOSE_IN(false)
    PLOG("\nGeography Model :   ext_gis_rsearch::init_ssimple(): Defining Set.");
  VERBOSE_OUT

  //In
  int xn = target->xn - 1;          //note: Locally without offset!
  int x0 = 0;
  int x = origin.x;

  int yn = target->yn - 1;
  int y0 = 0;
  int y = origin.x;

  bool wrap_left = target->wrap_left;
  bool wrap_right = target->wrap_right;
  bool wrap_top = target->wrap_top;
  bool wrap_bottom = target->wrap_bottom;

  int adj_radius = (int) ceil(radius);

  //Out

  //default is complete
  int x_l = x0;
  int x_r = xn;
  if (adj_radius < xn){   //check if everything is covered by default.

    x_l = x - adj_radius;
    if (!wrap_left && x_l < x0){ //if no wrapping allowed
      x_l = x0;
    }

    x_r = x + adj_radius;
    if (!wrap_right && x_r > xn){ //if no wrapping allowed
      x_r = xn;
    }

    //only relevant for wrapping, check that nothing is covered more than once
    if (x_l < x0 && (xn - (-x_l) ) <= x_r ){
      x_l = - (xn - x_r - 1); //end one right of x_r
    }

    //only relevant for wrapping, check that nothing is covered more than once
    if (x_r > xn && (x_r - xn)  >= x_l){
      x_r = xn + x_l - 1; //end one left of x_l
    }

  }

  int y_d = y0;
  int y_u = yn;
  if (adj_radius < yn){
    int y_d = y - adj_radius;
    if (!wrap_bottom && y_d < y0){
      y_d = y0;
    }

    int y_u = y + adj_radius;
    if (!wrap_top && y_u > yn){
      y_u = yn;
    }

    //only relevant for wrapping, check that nothing is covered more than once
    if (y_d < y0 && ( yn - (- y_d) ) <= y_u ){
      y_d = - (yn - y_u - 1); //end one above of x_u
    }

    //only relevant for wrapping, check that nothing is covered more than once
    if ( y_u > yn && (y_u - yn) >= y_d){
      y_u = yn + y_d - 1; //end one below of y_d
    }

  }

  //next, create a list of the potential candidates
  //05.07.2018 15:03:30 But change negative coords (wrapping checked before)
  ext_gis_coords temp;
  int total = 0;
  int tx,ty;
  for (temp.x=x_l; temp.x<=x_r; temp.x++){ //by column
    for (temp.y=y_d; temp.y<=y_u; temp.y++){ //elements in column
      total++; //checked
      temp.distance = geo_distance(origin,temp);
      tx=temp.x;
      ty=temp.y;
      if (temp.distance<=radius){
        if (tx<x0){
          tx = xn + tx; //temp.x < 0!
        } else if (tx>xn-1){
          tx -= xn;
        }
        if (ty<y0){
          ty = yn + ty;
        } else if (ty>yn-1){
          ty -= yn;
        }
        valid_objects.push_back(ext_gis_coords(tx,ty,temp.distance) ); //add to valid objects
//         PLOG("\n ADDING an item.");
      } else {
//         PLOG("\n NOT Adding an item");
      }
    }
  }

  if (sorted){
    std::sort(std::begin(valid_objects),std::end(valid_objects), [](auto const &t1, auto const &t2){
    return t1.distance < t2.distance; //sort ascending in distance
    });
   }
  VERBOSE_IN(false)
    PLOG("\n there are in %i options valid from %i checked.",valid_objects.size(),total);
  VERBOSE_OUT

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

double geo_distance(double x_1, double y_1, double x_2, double y_2){
/* Calculate the distance between to patches of land using Pythagorean Theorem*/
    double a_sq = x_1-x_2;
    a_sq *= a_sq;
    double b_sq = y_1-y_2;
    b_sq *= b_sq;

  return sqrt( a_sq + b_sq );
}

double geo_distance(ext_gis_coords a, ext_gis_coords b){
  return geo_distance((double) a.x, (double) a.y, (double) b.x, (double) b.y);
}

object* ext_gis_rsearch::next(){
  //a simple iterator through the vector of pointers initialised before
  //if agents=true is given, provide the list of LSD objects currently linked
  //to the patch.
  VERBOSE_IN(false)
    PLOG("\nGeography Model :   ext_gis_rsearch::next(): Producing next LSD object.");
    PLOG("\n total options: %i",valid_objects.size());
  VERBOSE_OUT
  if (it_valid == valid_objects.end()){
    VERBOSE_IN(false)
      PLOG("\n We have tested all %i valid objects.",valid_objects.size());
    TEST_OUT
    return NULL;
  } else
  {
    last = target->LSD_by_coords(*it_valid);
    last_distance = it_valid->distance;
    VERBOSE_IN(false)
      PLOG("\nOption is %i,%i, distance %g",it_valid->x,it_valid->y,it_valid->distance);
      PLOG("\nLSD Counterpart is %s, %g,%g, id %g",last->label,GET_VAR(last,"_x"),GET_VAR(last,"_y"),GET_ID(last));
    VERBOSE_OUT
    it_valid++;
  }
  return last;
}

ext_gis_coords::ext_gis_coords(int _x, int _y, double _distance){
  x = _x;
  y = _y;
  distance = _distance;
}

void ext_gis_patch::add_LSD_agent(object* obj_to_add){
  VERBOSE_IN(false)
    PLOG("\nGeography Model :   : ext_gis_patch::add_LSD_agent : Currently there are %i objects at patch w. ID %i. Now adding object %s with ID %g",LSD_agents.size(),ID,obj_to_add->label,GET_ID(obj_to_add));
  VERBOSE_OUT
  LSD_agents.push_back(obj_to_add);
}

bool ext_gis_patch::remove_LSD_agent(object* obj_to_remove){
  //https://stackoverflow.com/a/26567766/3895476
  {
    VERBOSE_IN(false)
      PLOG("\nGeography Model :   : ext_gis_patch::remove_LSD_agent : Currently there are %i objects at patch w. ID %i. Now removing object %s with ID %g",LSD_agents.size(),ID,obj_to_remove->label,GET_ID(obj_to_remove));
    VERBOSE_OUT
    auto it = std::find(LSD_agents.begin(), LSD_agents.end(), obj_to_remove);
    if (it != LSD_agents.end()) {
      VERBOSE_IN(false)
        PLOG("\n\t... success");
      VERBOSE_OUT
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

ext_gis_coords ext_gis::random_position(){ //produce a random position on the lattice
  return ext_gis_coords(random_x(),random_y() );
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

