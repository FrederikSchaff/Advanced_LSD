//Macros for easy usage
//This does not utilise all options yet.

  //Initialisation
#define PAJEK_INIT pajek::pajek_init(seed-1,true,"Default_CFG","net") //Initialise Pajek, all prior information is lost, Standard is append mode and seed as name.
#define PAJEK_INIT_S pajek::pajek_init(seed-1,false,"Default_CFG","net") //Initialise Pajek, all prior information is lost, Standard is append mode and seed as name.
#define PAJEK_INIT_X(id,folder,suffix) pajek::pajek_init((int)id,true,folder,suffix) //provide some optional information
#define PAJEK_INIT_XS(id,folder,suffix) pajek::pajek_init((int)id,false,folder,suffix) //single files mode, provide some optional information

#define PAJEK_INIT_NEW_RELATION(name,isedge) pajek::pajek_init_KindsOfRelation(name,isedge) //Add a new relation with name "name" and declare it as edge (isedge=true) or arc (isedge=false)

  //Adding Nodes
#define PAJEK_VERTICE(ID,name,value) pajek::pajek_vertices_add( (int) ID, name,  /*int shape=*/0, /*char const *colour=*/"Black", /*double x_fact=*/1.0, /*double y_fact=*/1.0, value, /*double x_pos=*/-1.0, /*double y_pos=*/-1.0, /*int start=*/-1, /*int end=*/-1 )
#define PAJEK_VERTICE_X(ID,name,value,shape,colour,x_size,y_size) pajek::pajek_vertices_add( (int) ID, name,  (int) shape, colour, x_size, y_size, value, /*double x_pos=*/-1.0, /*double y_pos=*/-1.0, /*int start=*/-1, /*int end=*/-1 )
#define PAJEK_VERTICE_XP(ID,name,value,shape,colour,x_size,y_size,x_coor,y_coor) pajek::pajek_vertices_add( (int) ID, name,  (int) shape, colour, x_size, y_size, value, x_coor, y_coor, /*int start=*/-1, /*int end=*/-1 )
#define PAJEK_VERTICE_XPT(ID,name,value,shape,colour,x_size,y_size,x_coor,y_coor,start,end) pajek::pajek_vertices_add( (int) ID, name,  (int) shape, colour, x_size, y_size, value, x_coor, y_coor, (int) start, (int) end )


  //Special: Add all objects of given tpye as nodes
#define PAJEK_VERTICE_SPECIALS(parent,obj_label,id_var_label,val_var_label,val_factor) \
  object *cur_paj_par; \
  CYCLES(parent,cur_paj_par,obj_label){ \
    PAJEK_VERTICE(VS(cur_paj_par,id_var_label),obj_label,VS(cur_paj_par,val_var_label)*val_factor ); \
  }

#define PAJEK_VERTICE_SPECIAL(obj_label,id_var_label,val_var_label,val_factor) PAJEK_VERTICE_SPECIALS(p,obj_label,id_var_label,val_var_label,val_factor)

  //Adding Links
#define PAJEK_EDGE(source,target,name,value) pajek::pajek_arcs_add(true,(int)source,(int)target,value,name) //Add a simple edge
#define PAJEK_ARC(source,target,name,value) pajek::pajek_arcs_add(false,(int)source,(int)target,value,name) //Add a simple arc
#define PAJEK_ARC_X(source,target,name,value,width,colour) pajek::pajek_arcs_add(false,(int)source, (int) target,value,name,  (int) width, colour)   //More info (width and colour)
#define PAJEK_EDGE_X(source,target,name,value,width,colour) pajek::pajek_arcs_add(true,(int)source, (int) target,value,name,  (int) width, colour)
#define PAJEK_ARC_XT(source,target,name,value,width,colour,start,end) pajek::pajek_arcs_add(false,(int)source, (int) target,value,name,  (int) width, colour, (int) start, (int) end )
#define PAJEK_EDGE_XT(source,target,name,value,width,colour,start,end) pajek::pajek_arcs_add(true,(int)source, (int) target,value,name,  (int) width, colour, (int) start, (int) end )

  //Saving / Writing data
#define PAJEK_SNAPSHOT_ZERO pajek::pajek_snapshot(0,false) //make a single snapshot from the initial configuration
#define PAJEK_SNAPSHOT pajek::pajek_snapshot(-1,t==max_step?true: (quit==0?false:true) ) //make a single snapshot, continous time. Also covers final snapshot.

