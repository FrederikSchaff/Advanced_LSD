/*************************************************************
                                                    May 2018
  LSD Geography module - backend for LSD (least 7.0)
  written by Frederik Schaff, Ruhr-University Bochum

  for infos on LSD see ...

	Copyright Frederik Schaff
  This code is distributed under the GNU General Public License

  What it does:

  Link each LSD object "Patch"

  wrapping: there are 2^4 options. We use a bit-code (0=off):
    0-bit: left     : 0=0 1=1
    1-bit: right    : 0=0 1=2
    2-bit: top      : 0=0 1=4
    3-bit: bottom   : 0=0 1=8

    Simply sum up the options selected and pass this as argument.

  currently, only fixed size patches are possible (EIGEN library allows for
  sparse matrices!). However, we do not need to link the backend (i.e. potential
  world size) to the LSD patches.

  The patch is located on a coordinate xy system. (0,0) is bottom left,
    (xn,yn) is top right.

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

//#include <random>    //needs to be before fun_head...? - problems with abs() def.

struct ext_gis_coords{
  int x,y;
  double distance;
  ext_gis_coords(int x=-1, int y=-1, double distance=-1.0); //constructor
};

class ext_gis_patch {
  public:
    /*generic*/
    object* LSD_counterpart = NULL; //Link to LSD object holding this external object

    int x,y; //x and y position, ranging from 0 to xn-1
    int ID; //unique ID - x+xn*(y)

    //Pointers to neighbours, lattice layout
    ext_gis_patch* up = NULL;
    ext_gis_patch* down = NULL;
    ext_gis_patch* left = NULL;
    ext_gis_patch* right = NULL;

    ext_gis_patch* next = NULL; //treat as one-dim row.

    std::vector<object*> LSD_agents; //link to LSD objects that reside at the current patch.
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
    int xn,yn; //size of lattice
    std::vector<std::vector<ext_gis_patch>>  patches;

    object* LSD_by_coords(int x, int y); //returns the corresponding LSD patch, if it exists
    object* LSD_by_coords(ext_gis_coords); //returns the corresponding LSD patch, if it exists

    ext_gis(object* counterpart, char const *_patch_label, int xn, int yn, int wrap);
    ext_gis_patch* newPatch(object* LSD_Patch);

    ext_gis_patch* move_single(ext_gis_patch* pos, char direction); //move "u"p, "d"own, "r"ight or "l"eft, if possible. else return NULL.
    ext_gis_patch* move(ext_gis_patch* pos, const std::string& direction, bool complete=false); //move "u"p, "d"own, "r"ight or "l"eft, if possible. else return NULL.
    object* move_LSD(int x, int y, const std::string& direction, bool complete=false); //move "u"p, "d"own, "r"ight or "l"eft, if possible. else return NULL.
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

    ext_gis_coords origin; //where the search starts
    object* last; //the current object selected
    double last_distance; //distance from last object to origin of search


    int type; //Type of search. Default is 0, i.e. if an object can be reached is not checked.
    double radius;

    std::vector <ext_gis_coords> valid_objects; //distance , coords
    std::vector<ext_gis_coords>::iterator it_valid; //simple forward iterator
    //std::vector <std::vector <bool> > search_space; //xy search space

    ext_gis_rsearch(ext_gis* _target, ext_gis_coords _origin, double _radius, int _type=0); //each time a new search is started we create a new object.
    ext_gis_rsearch(ext_gis* _target, int _origin_x, int _origin_y, double _radius, int _type=0); //each time a new search is started we create a new object.
    ext_gis_rsearch(){}; //default, does not initialise stuff.

    object* next(); //provide next LSD patch object in search radius,
                    //or NULL if done

    //Provision to cycle through non-patch objects located in search space by label
    std::vector<object*>::iterator it_rsearch_labelled_obj;

  private:
    //See: https://stackoverflow.com/a/7330341/3895476
    // You may not "delegate" constructors. Cost me a full day or so...
    void init(ext_gis* _target, ext_gis_coords _origin, double _radius, int _type); //each time a new search is started we create a new object.
    void init(ext_gis* _target, int _origin_x, int _origin_y, double _radius, int _type); //each time a new search is started we create a new object.

    void init_ssimple(bool sorted=false); //initialise the rsearch upon creation
                 //packed in an extra function to make improvements more easy.


};

//helper
double geo_distance(double x_1, double y_1, double x_2, double y_2);
double geo_distance(ext_gis_coords a, ext_gis_coords b);


//Macro to initialise the GIS
// #define GIS_INITS(gis_obj,gis_patch,gis_xn,gis_yn,gis_wrap)\
//   ADDEXT2S(gis_obj,ext_gis); P_EXTS(gis_obj,ext_gis)->gis_init(gis_obj,gis_patch,gis_xn,gis_yn,gis_wrap);
#define GIS_INITS(gis_obj,gis_patch,gis_xn,gis_yn,gis_wrap)\
  ADDEXT2S(gis_obj,ext_gis(gis_obj,gis_patch,gis_xn,gis_yn,gis_wrap));
#define GIS_INIT(gis_patch,gis_xn,gis_yn,gis_wrap) GIS_INITS(p,gis_patch,gis_xn,gis_yn,gis_wrap)

//Macro to move from a given x,y pos stepwise.
#define GIS_MOVE(x,y,direction) P_EXTS(SEARCH("GIS_Model"),ext_gis)->move_LSD(x,y,direction)
#define GIS_MOVES(gis_obj,x,y,direction) P_EXTS(gis_obj,ext_gis)->move_LSD(x,y,direction)

//Iterate through complete list of patches in ext obj
#define GIS_IT_PATCHS(o,name) ext_gis_patch* name = &(P_EXTS(o,ext_gis)->patches.at(0).at(0))
#define GIS_IT_PATCH(name) GIS_IT_PATCHS(p,name)

//to do: MOVE MACROS

//to do: SEARCH LSD objects in neighbourhood by label. Wrapper using existing
//rsearch!

// GIS_

//Macro to search in a given radius around a given position. It provides a CYCLE like behaviour.
// Create temp search object, initialise the search (define search set), provide iterator through the set
//the search includes the obj. itself.

//externals
ext_gis_rsearch temp_gis_search_obj;

#define GIS_CYCLE_NEIGHBOURSS(gis_obj,Xobj,x,y,radius,type) \
  temp_gis_search_obj = ext_gis_rsearch(P_EXTS(gis_obj,ext_gis),x,y,radius,type); \
  PLOG("\n NNN size is: %i",temp_gis_search_obj.valid_objects.size()); \
  for (Xobj=temp_gis_search_obj.next();Xobj!=NULL;Xobj=temp_gis_search_obj.next())

#define GIS_CYCLE_NEIGHBOURS_SIMPLES(gis_obj,Xobj,x,y,radius) \
  GIS_CYCLE_NEIGHBOURSS(gis_obj,Xobj,x,y,radius,0)

#define GIS_CYCLE_NEIGHBOURS_SIMPLE(Xobj,x,y,radius) \
  GIS_CYCLE_NEIGHBOURS_SIMPLES(SEARCH("GIS_Model"),Xobj,x,y,radius)

#define GIS_CYCLE_NEIGHBOURS_SORTS(gis_obj,Xobj,x,y,radius) \
  GIS_CYCLE_NEIGHBOURSS(gis_obj,Xobj,x,y,radius,1)

#define GIS_CYCLE_NEIGHBOURS_SORT(Xobj,x,y,radius) \
  GIS_CYCLE_NEIGHBOURS_SORTS(SEARCH("GIS_Model"),Xobj,x,y,radius)
