/*************************************************************
                                                    May 2018
  LSD Geography module - backend for LSD (least 7.0)
  written by Frederik Schaff, Ruhr-University Bochum

  Copyright Frederik Schaff
  This code is distributed under the GNU General Public License

  What it needs:

  in the LSD model file (.sim)

  - an LSD object "Patch" (label can differ and "Patch" is a placeholder here)
    The Patch object represents one cell in a regular grid, the geography
    Also, this "Patch" object needs to contain two parameters for its position:
    - "Patch_x"
    - "Patch_y"

  - LSD objects/parameters to set-up the gis:
    GIS_Model       Object      The GIS_Model Object is linked to the c++ class
                                that provides the GIS features.
    GIS_Init        Function    Initialises the GIS backend. See below.
    GIS_xn          Parameter   dimension of the geo-space (x)
    GIS_yn          Parameter   dimension of the geo-space (y)
    GIS_wrap        Parameter   World-Wrapping, bit-code:
                    there are 2^4 options. We use a bit-code (0=off):
                    0-bit: left     : 0=0 1=1
                    1-bit: right    : 0=0 1=2
                    2-bit: top      : 0=0 1=4
                    3-bit: bottom   : 0=0 1=8
                    Simply sum up the options selected and pass this as argument.
                    Examples: 0 = None, 1 = left, 2 = right, 3 (1+2) left-right,
                    5 up, 7 down, 12 (5+7) up-down, 15 (1+2+5+7) torus.

  How to use it:
  - Integration: The integration is explained in the readme file of AdvancedLSD
  - Initialisation: At the very beginning of the simulation, call via FakeCaller
    the Function GIS_Init, passing as argument any item "Patch" - the real label
    of the "Patch" will be used from the object passed.

  How it works:
  - Each LSD patch object is linked to a fixed vector situated in the
  - The patch is located on a coordinate xy system. (0,0) is bottom left,
    (xn-1,yn-1) is top right.

 *************************************************************/

/***************************************************
backend_gis.h

This file contains the declarations and new macros for the population model.
It also includes the other core-code files (all not fun_*)
****************************************************/

#include <vector>
#include <tuple>
#include <algorithm>
#include <functional>
#include <deque>

struct ext_gis_coords; //a special type holding x and y coords and distance information
struct coords;
struct int_coords;
class ext_gis_patch;  //GIS information for each patch that cannot reside in LSD
class ext_gis; //the main gis object
class ext_gis_rsearch; //a "radius" search object


//helper
double geo_distance(double x_1, double y_1, double x_2, double y_2, int wrap=0, double xn=0, double yn=0);
double geo_distance(ext_gis_coords a, ext_gis_coords b, int wrap=0, double xn=0, double yn=0);
double geo_distance(coords a, coords b, int wrap=0, double xn=0, double yn=0);

//sqrt(x)<sqrt(y) if x<y
double geo_pseudo_distance(double x_1, double y_1, double x_2, double y_2, int wrap=0, double xn=0, double yn=0);
int geo_pseudo_distance(int x_1, int y_1, int x_2, int y_2, int wrap=0, int yn=0, int xn=0);
double geo_pseudo_distance(coords a, coords b, int wrap=0, double xn=0, double yn=0);

struct Wrap{
    bool left;
    bool right;
    bool top;
    bool bottom;
   /*wrapping: there are 2^4 options. We use a bit-code (0=off):
    0-bit: left     : 0=0 1=1
    1-bit: right    : 0=0 1=2
    2-bit: top      : 0=0 1=4
    3-bit: bottom   : 0=0 1=8
    */
    Wrap(int wrap){
      if (wrap>7){bottom=true; wrap-=8;} else {bottom=false; }
      if (wrap>3){top=true;    wrap-=4;} else {top=false;    }
      if (wrap>1){right=true;  wrap-=2;} else {right=false;  }
      if (wrap>0){left=true;           } else {left=false;   }
    }
    Wrap(bool left, bool right, bool top, bool bottom) : left(left),right(right),top(top),bottom(bottom) {}
};

int transcode_Wrap(bool left, bool right, bool top, bool bottom){
  int wrap = 0;
  if (left)
    wrap+=1;
  if (right)
    wrap+=2;
  if (top)
    wrap+=4;
  if (bottom)
    wrap+=8;

  return wrap;
}

struct coords {
  double x;
  double y;
  coords(double x, double y) : x(x), y(y) {};
  friend bool operator <(const coords& a, const coords& b) {
    return std::tie(a.x, a.y) < std::tie(a.x, a.y);
  }
};

struct int_coords {
  int x;
  int y;
  int check_comp = 0;
  int_coords(int x, int y) : x(x), y(y) {};
  friend bool operator <(const int_coords& a, const int_coords& b) {
    return std::tie(a.x, a.y) < std::tie(a.x, a.y);
  }
};

//ext_gis_coords is a struct that holds x and y coords and optionally a distance
struct ext_gis_coords{
  int x,y;
  double pseudo_distance;
  double real_distance=-1;
  double distance(){
    if (real_distance-1) {
      real_distance = sqrt(pseudo_distance);
    }
    return real_distance;
  };
  ext_gis_coords(int x, int y, double pseudo_distance) : x(x), y(y), pseudo_distance(pseudo_distance) {}; //constructor
//   ext_gis_coords(int x, int y) : x(x), y(y), pseudo_distance(0) {}; //constructor
//   ext_gis_coords() : x(-1),y(-1),pseudo_distance(0) {};//default constructor

  bool operator<(const ext_gis_coords &b) const
  {
    return pseudo_distance < b.pseudo_distance;
  }
};


//ext_gis_patch is the smallest "unit" in the gis layer.
//Currently it is of fixed size. Later on this may change.
//Currently the x,y position refer to the left border, later a right border may be added
class ext_gis_patch {
  public:
    /*generic*/
    object* LSD_counterpart = NULL; //Link to LSD object holding this external object

    int x,y; //x and y position, ranging from 0 to xn-1 , the "left" or "lower" border
    int ID; //unique ID - x+xn*(y) currently

    //Pointers to neighbours, lattice layout
    ext_gis_patch* up = NULL;
    ext_gis_patch* down = NULL;
    ext_gis_patch* left = NULL;
    ext_gis_patch* right = NULL;

    ext_gis_patch* next = NULL; //treat as one-dim row.

    std::deque<object*> LSD_agents; //link to LSD objects that reside at the current patch.
    bool remove_LSD_agent(object* obj_to_remove);
    void add_LSD_agent(object* obj_to_add);

};

/* A class to search a given set of patches in a radious around the origin.
  The search shall be complete. In future (!), it should only consider such places
  that can actually be reached. For now, everything within the boundaries is
  reported as option. Wraping conditions are taken care of, though.

  Info: https://en.wikipedia.org/wiki/Flood_fill
  */




class ext_gis {
  public:
    /*generic*/
    object* LSD_counterpart; //Link to LSD object holding this external object
    char patch_label[ MAX_ELEM_LENGTH ]; //the label of the LSD object, e.g. "Patch" or "Patch_Type_A"

    bool wrap_left,wrap_right,wrap_top,wrap_bottom; //do we have world wrapping?
    int code_Wrap;
    int xn,yn; //size of lattice
    std::vector<std::vector<ext_gis_patch>>  patches;

    ext_gis_patch* get_patch_at(int x, int y); //if it doesn't exist returns NULL

    object* LSD_by_coords(int x, int y); //returns the corresponding LSD patch, if it exists
    object* LSD_by_coords(ext_gis_coords); //returns the corresponding LSD patch, if it exists
    object* LSD_by_coords(coords); //returns the corresponding LSD patch, if it exists
    object* LSD_by_coords(int_coords); //returns the corresponding LSD patch, if it exists

    ext_gis(object* counterpart, char const *_patch_label, int xn, int yn, int code_Wrap);
    ext_gis_patch* newPatch(object* LSD_Patch);

    coords random_position(); //produce a random position on the lattice
    int random_x(); //produce a random position on the lattice
    int random_y(); //produce a random position on the lattice

    //utilities to move through patches
    ext_gis_patch* move_single(ext_gis_patch* pos, char direction); //move "u"p, "d"own, "r"ight or "l"eft, if possible. else return NULL.
    ext_gis_patch* move(ext_gis_patch* pos, const std::string& direction, bool complete=false); //move "u"p, "d"own, "r"ight or "l"eft, if possible. else return NULL.
    object* move_LSD(int x, int y, const std::string& direction, bool complete=false); //move "u"p, "d"own, "r"ight or "l"eft, if possible. else return NULL.

    //utilities to associate LSD objects with patches (other than LSD_Patch)
      //default new pos is randomised
      //returns LSD Patch object associated with position
    object* LSD_obj_pos_init(object* LSD_obj,int x=-1, int y=-1);
    object* LSD_obj_pos_move(int x_orig, int y_orig, object* LSD_obj, int x_new=-1, int y_new=-1);
    object* LSD_obj_pos_remove(int x, int y, object* LSD_obj);
};
//Initialise statics
//   ext_gis::LSD_counterpart = NULL;
//   ext_gis::
//   ext_gis::
//   ext_gis::
/* An iterator type to get the pair of coordinates in a vector of coordinates
  and return the associated LSD object, if any.
  See: https://stackoverflow.com/a/27307613/3895476
*/
// struct ext_gis_coords_it : public std::vector<ext_gis_coords*>::iterator {
//     using std::vector<ext_gis_coords*>::iterator::iterator;
//     object* operator*() { return LSD_by_coords(std::vector<ext_gis_coords*>::iterator::operator*()); }
// };


class ext_gis_rsearch {
  //later it shall be a member class of ext_gis.

  public:
    //from member
    ext_gis* target;

    //own

    coords origin; //where the search starts
    object* last; //the current object selected
    double last_distance; //distance from last object to origin of search


    int type; //Type of search. Default is 0, not sorted. 1 is sorted. more to come.
    double radius;        //default: complete
    double pseudo_radius;

    std::vector<ext_gis_coords> valid_objects; //distance , coords
    std::vector<ext_gis_coords>::iterator it_valid; //simple forward iterator
    //std::vector <std::vector <bool> > search_space; //xy search space

    ext_gis_rsearch(ext_gis* _target, coords _origin, double _radius=-1, int _type=0) : target(_target), origin(_origin), radius(_radius), type(_type)
    {
      pseudo_radius = radius*radius;
      init();
    } //each time a new search is started we create a new object.
//    ext_gis_rsearch(ext_gis* _target, int _origin_x, int _origin_y, double _radius=-1, int _type=0); //each time a new search is started we create a new object.
//     ext_gis_rsearch(){
//       PLOG("\next_gis_rsearch() empty default initialisation called.");
//     }; //default, does not initialise stuff.

    object* next();  //provide next LSD patch object in search radius,
                    //or NULL if done.

//  To do: Iterate by label
//     //Provision to cycle through non-patch objects located in search space by label
//     std::vector<object*>::iterator it_rsearch_labelled_obj;

  private:
    //See: https://stackoverflow.com/a/7330341/3895476
    // You may not "delegate" constructors. Cost me a full day or so...
    void init(){
      VERBOSE_MODE(false){
        PLOG("\nGeography Model :   ext_gis_rsearch::ext_gis_rsearch(): Initialising");
        PLOG("\n target LSD Object holding lattice is: %s", target->LSD_counterpart->label);
        PLOG("\n Search is centered at: %i, %i and radius is: %g.",origin.x,origin.y,radius);
        PLOG("\n Type of search is %i",type);
      }

      if (radius < 0){
        radius = max(target->xn,target->yn)*2; //complete
        pseudo_radius = radius*radius; //we work with pseudo distance to not sqrt()
      }

      valid_objects.clear(); //clear list
      switch (type) {
        case 0: init_ssimple(); break;
        case 1: init_ssimple(true); break;//sorted
        default: init_ssimple(); break; //if wrong argument is supplied. add check later
      }
      VERBOSE_MODE(false){
        PLOG("\nGeography Model :   ext_gis_rsearch::ext_gis_rsearch(): Initialising done. Check.");
        PLOG("\n There are %i options.",valid_objects.size());
      }
      it_valid  = valid_objects.begin(); //initialise the iterator used in next()
      VERBOSE_MODE(false && valid_objects.size()>0){
        PLOG("\n First option is located at (%i, %i) with distance to origin: %g",it_valid->x,it_valid->y,it_valid->distance());
      }
    }

    void init_ssimple(bool to_sort=false); //initialise the rsearch upon creation
                 //packed in an extra function to make improvements more easy.


};





