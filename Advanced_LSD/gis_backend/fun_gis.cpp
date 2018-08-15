/*************************************************************
                                                    May 2018
  fun_gis.cpp : see gis.h for infos

  LSD Geography module - backend for LSD (least 7.0)
  written by Frederik Schaff, Ruhr-University Bochum

	Copyright Frederik Schaff
  This code is distributed under the GNU General Public License

 *************************************************************/


FUNCTION("GIS_Init")
/* Initialise the population model. Called via fake-caller with c=Patch object */
TRACK_SEQUENCE
  int xn = V("GIS_xn");
  int yn = V("GIS_yn");

  TEST_MODE(xn<1 || yn<1){
    PLOG("\nERROR! Gis_Init: select xn,yn >= 1!");
    ABORT2
    END_EQUATION(0.0);
  }

  int wrap = V("GIS_wrap");
  GIS_INIT(c->label,xn,yn,wrap)

  int n = COUNTS(c->up,c->label); //count existing patches
  ADDNOBJS(c->up,c->label,xn*yn-n); //Add missing objects

  GIS_IT_PATCH(cur_p_ext); //Get iterator to first patch

    //Cycle through patches in LSD and backend and link them.
    //Note: It is still possible to provide user defined c-extension for the
    //patch
  CYCLES(c->up,cur,c->label){
    WRITES(cur,GET_ID_LABEL(cur),cur_p_ext->ID);
    cur_p_ext->LSD_counterpart=cur;
    WRITES(cur,GET_VAR_LABEL(c,"_x"),cur_p_ext->x);
    WRITES(cur,GET_VAR_LABEL(c,"_y"),cur_p_ext->y);
    cur_p_ext=cur_p_ext->next; //move to next patch
  }

  VERBOSE_MODE(xn<6 && yn<6){
    int count = 0;
    PLOG("\nGIS @ LSD - visually testing patch assignments:\n")
    CYCLES(c->up,cur,c->label){
      int ID = VS(cur,GET_ID_LABEL(cur));
      int x = GET_VAR(cur,"_x");
      int y = GET_VAR(cur,"_y");
      PLOG("(%i,%i |%i,%i,%i,%i) -",x,y,GIS_MOVES(p,x,y,"l")!=cur, GIS_MOVES(p,x,y,"r")!=cur, GIS_MOVES(p,x,y,"u")!=cur, GIS_MOVES(p,x,y,"d")!=cur);  //cur - no move.
      //to do above
      PLOG("%05i: (%i,%i) ",ID,x,y);
      count++;
      if (count%xn == 0) {PLOG("\n");}
    }
  }

PARAMETER
RESULT(0)

