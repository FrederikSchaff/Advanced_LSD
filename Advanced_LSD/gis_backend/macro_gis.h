//Macro to initialise the GIS
// #define GIS_INITS(gis_obj,gis_patch,gis_xn,gis_yn,gis_wrap)\
//   ADDEXT2S(gis_obj,ext_gis); P_EXTS(gis_obj,ext_gis)->gis_init(gis_obj,gis_patch,gis_xn,gis_yn,gis_wrap);
#define GIS_INITS(gis_obj,gis_patch,gis_xn,gis_yn,gis_wrap)\
  ADDEXT2S(gis_obj,ext_gis(gis_obj,gis_patch,gis_xn,gis_yn,gis_wrap));
#define GIS_INIT(gis_patch,gis_xn,gis_yn,gis_wrap) GIS_INITS(p,gis_patch,gis_xn,gis_yn,gis_wrap)

//Macro to move from a given x,y pos stepwise.
#define GIS_MOVE(x,y,direction) P_EXTS(SEARCHS(root,"GIS_Model"),ext_gis)->move_LSD(x,y,direction)
  #define GIS_MOVES(gis_obj,x,y,direction) P_EXTS(gis_obj,ext_gis)->move_LSD(x,y,direction)

//Iterate through complete list of patches in ext obj
#define GIS_IT_PATCHS(o,name) ext_gis_patch* name = &(P_EXTS(o,ext_gis)->patches.at(0).at(0))
  #define GIS_IT_PATCH(name) GIS_IT_PATCHS(p,name)


//Macros to associate LSD agent objects with patches and move them in space (beam)

#define GIS_RANDOM_XS(gis_obj) uniform(0,P_EXTS(gis_obj,ext_gis)->xn)
#define GIS_RANDOM_X() GIS_RANDOM_XS(SEARCHS(root,"GIS_Model"))
#define GIS_RANDOM_YS(gis_obj) uniform(0,P_EXTS(gis_obj,ext_gis)->yn)
#define GIS_RANDOM_Y() GIS_RANDOM_YS(SEARCHS(root,"GIS_Model"))

#define GIS_ASSOC_INITS(gis_obj,obj,x,y) P_EXTS(gis_obj,ext_gis)->LSD_obj_pos_init( obj, (int)x, (int)y )
  #define GIS_ASSOC_INIT(obj,x,y) P_EXTS(SEARCHS(root,"GIS_Model"),ext_gis)->LSD_obj_pos_init( obj, (int)x, (int)y )
// #define GIS_ASSOC_INITRS(gis_obj,obj) GIS_ASSOC_INITS(gis_obj,obj,-1,-1)
//   #define GIS_ASSOC_INITR(obj) GIS_ASSOC_INIT(obj,-1,-1)


#define GIS_ASSOC_MOVES(gis_obj,obj,x_orig,y_orig,x_new,y_new) P_EXTS(gis_obj,ext_gis)->LSD_obj_pos_move( (int)x_orig, (int)y_orig, obj, (int)x_new, (int)y_new )
  #define GIS_ASSOC_MOVE(obj,x_orig,y_orig,x_new,y_new) P_EXTS(SEARCHS(root,"GIS_Model"),ext_gis)->LSD_obj_pos_move( (int)x_orig, (int)y_orig, obj, (int)x_new, (int)y_new )
#define GIS_ASSOC_MOVERS(gis_obj,obj,x_orig,y_orig) GIS_ASSOC_MOVES(gis_obj,obj,x_orig,y_orig,-1,-1)
  #define GIS_ASSOC_MOVER(gis_obj,obj,x_orig,y_orig) GIS_ASSOC_MOVE(obj,x_orig,y_orig,-1,-1)

#define GIS_ASSOC_REMOVES(gis_obj,obj,x,y) P_EXTS(gis_obj,ext_gis)->LSD_obj_pos_remove( (int)x, (int)y, obj )
  #define GIS_ASSOC_REMOVE(obj,x,y) P_EXTS(SEARCHS(root,"GIS_Model"),ext_gis)->LSD_obj_pos_remove( (int)x, (int)y, obj)


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
  PLOG("\n There are %i valid objects to scan.",temp_gis_search_obj.valid_objects.size()); \
  for (Xobj=temp_gis_search_obj.next();temp_gis_search_obj.it_valid != temp_gis_search_obj.valid_objects.end();Xobj=temp_gis_search_obj.next())

#define GIS_CYCLE_NEIGHBOURS_SIMPLES(gis_obj,Xobj,x,y,radius) \
  GIS_CYCLE_NEIGHBOURSS(gis_obj,Xobj,x,y,radius,0)

#define GIS_CYCLE_NEIGHBOURS_SIMPLE(Xobj,x,y,radius) \
  GIS_CYCLE_NEIGHBOURS_SIMPLES(SEARCHS(root,"GIS_Model"),Xobj,x,y,radius)

#define GIS_CYCLE_NEIGHBOURS_SORTS(gis_obj,Xobj,x,y,radius) \
  GIS_CYCLE_NEIGHBOURSS(gis_obj,Xobj,x,y,radius,1)

#define GIS_CYCLE_NEIGHBOURS_SORT(Xobj,x,y,radius) \
  GIS_CYCLE_NEIGHBOURS_SORTS(SEARCHS(root,"GIS_Model"),Xobj,x,y,radius)



  //Needed: Iterator like for moving through associated obj with given label
  //in given neighbourhood.
