//a simple test program
#include "Pajek_new.cpp"


int main(){

    //simple test that should work
  PAJ_MAKE_AVAILABLE
  PAJ_INIT_ANIM("Network","TestSet",3,"Test Set")
  for (int t = 1; t<5; t++){
    for (int id=0; id<6; id++){
      PAJ_ADD_V_XY(t,id,"mopped",1,rel_pos(id+1,6),.25)
      PAJ_ADD_V_XY(t,id,"Huhn",1,rel_pos(id+1,6),.75)
      if (t==2 && id == 3) {
        PAJ_ADD_V_XY(t,id,"Hahn",2,rel_pos(id+1,6),.5)
        continue;
      }
      PAJ_ADD_V_XY(t,id,"Hahn",1,rel_pos(id+1,6),.5)
    }
    for (int id=0; id<6; id++){
      int target = id+1<6?id+1:0;
      PAJ_ADD_E_W(t,id,"mopped",target,"mopped",1,"Edge",0)
      PAJ_ADD_E_W(t,id,"mopped",target,"mopped",-1,"elseEdge",5)
      PAJ_ADD_A(t,target,"mopped",id,"Huhn",1,"Arc")
      PAJ_ADD_A(t,target,"mopped",id,"mopped",-1,"elseArc")
      if (t==2 ) {
        PAJ_ADD_A(t,target,"Hahn",id,"mopped",2,"seltsam");
        continue;
      }
      PAJ_ADD_A(t,target,"Hahn",id,"mopped",1,"seltsam");
    }
  }
  PAJ_SAVE

	/*  //Test case with some errors
	Pajek myp("Network","Net",1,"network name");


	myp.add_vertice(0,   0,       "car",   11,  0.5,0.5,"ellipse",1,1,"RED","Linus");
	myp.add_vertice(0,   0,       "car",   11,  0.5,0.5,"ellipse",1,1,"RED","Luise");
	myp.add_vertice(0,   111,     "bus",   11,  0.5,0.5,"ellipse",1,1,"RED");
	myp.add_vertice(0,   222,     "car",   11,  0.5,0.5,"ellipse",1,1,"RED");

	myp.add_relation(0,111,"car",222,"car",false,"ARC",100,2.0,"Red");
	myp.add_relation(0,222,"car",0,"car",false,"noEDGE",100,2.0,"Black");
  myp.add_relation(0,222,"car",111,"car",true,"EDGE",100,2.0,"Black");

	myp.add_vertice(1,   0,       "car",   11,  0.5,0.5,"ellipse",1,1,"RED");
	myp.add_vertice(1,   11,     "car",   11,  0.5,0.5,"ellipse",1,1,"RED");
	myp.add_vertice(1,   22,     "car",   11,  0.5,0.5,"ellipse",1,1,"RED");
	myp.add_vertice(1,   111,     "bus",   11,  0.5,0.5,"ellipse",1,1,"RED");
	myp.add_vertice(1,   222,     "car",   11,  0.5,0.5,"ellipse",1,1,"RED");

	myp.add_relation(1,0,"car",11,"car",false,"ARC",100,2.0,"Black");
	myp.add_relation(1,111,"bus",222,"car",false,"ARC",100,2.0,"Red");
	myp.add_relation(1,22,"car",11,"car",true,"EDGE",100,2.0,"Black");
	myp.add_relation(1,11,"car",22,"car",true,"EDGE",100,2.0,"Black");
  myp.add_relation(1,11,"car",22,"car",false,"EDGE",100,2.0,"Black");

  myp.add_vertice(2,   0,       "car",   11,  0.5,0.5,"ellipse",1,1,"RED");
	myp.add_vertice(2,   11,     "car",   11,  0.5,0.5,"ellipse",1,1,"RED");
	myp.add_vertice(2,   22,     "car",   11,  0.5,0.5,"ellipse",1,1,"RED");
	myp.add_vertice(2,   111,     "bus",   11,  0.5,0.5,"ellipse",1,1,"RED");
	myp.add_vertice(2,   222,     "car",   11,  0.5,0.5,"ellipse",1,1,"RED");
  myp.add_vertice(2,   223,     "car",   11,  0.5,0.5,"ellipse",1,1,"RED");
  myp.add_vertice(2,   224,     "car",   11,  0.5,0.5,"ellipse",1,1,"RED");

	myp.add_relation(2,0,"car",11,"car",false,"ARC",100,2.0,"Black");
	myp.add_relation(2,111,"bus",222,"car",false,"ARC",100,2.0,"Red");
  myp.add_relation(2,111,"bus",223,"car",false,"ARC",100,2.0,"Red");
  myp.add_relation(2,111,"bus",224,"car",false,"ARC",100,2.0,"Red");
	myp.add_relation(2,22,"car",11,"car",true,"EDGE",100,2.0,"Black");
	myp.add_relation(2,11,"car",22,"car",true,"EDGE",100,2.0,"Black");
  myp.add_relation(2,11,"car",22,"car",false,"EDGE",100,2.0,"Black");

  myp.add_vertice(3,   0,       "car",   11,  0.5,0.5,"ellipse",1,1,"RED");
  myp.add_vertice(3,   11,     "car",   11,  0.5,0.5,"ellipse",1,1,"RED");
  myp.add_relation(3,0,"car",11,"car",false,"ARC",100,2.0,"Black");

  myp.add_vertice(4,   0,       "car",   11,  0.5,0.5,"ellipse",1,1,"RED");
  myp.add_vertice(4,   11,     "car",   11,  0.5,0.5,"ellipse",1,1,"RED");
  myp.add_relation(4,0,"car",11,"car",false,"ARC",100,2.0,"Black");
  myp.add_relation(4,0,"car",11,"car",true,"EDGE",100,2.0,"Black");

  myp.add_vertice(5,   0,       "car",   11,  0.5,0.5,"ellipse",1,1,"RED");
  myp.add_vertice(5,   11,     "car33",   11,  0.5,0.5,"ellipse",1,1,"RED");
  myp.add_vertice(5,   112,     "mopped",   11,  0.5,0.5,"ellipse",1,1,"RED");
  myp.add_relation(5,0,"car",11,"car",false,"ARC",100,2.0,"Black");

  myp.printall();
  myp.save_to_file();
  */


}
