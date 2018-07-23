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
fun_LSD_geo.cpp

This file contains all the LSD EQUATIONS that are part of the
template and can be left "as is" by users.
****************************************************/

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    In LSD Model, provide an object "Pop_Geo" which holds the following:

    ++Parameterisation++
    x_cols          Parameter   dimension of the geo-space (x)
    y_rows          Parameter   dimension of the geo-space (y)
    wrapping        Parameter   World-Wrapping, bit-code: 0 = None, 1 = left, 2 = right, 3 (1+2) left-right, 5 up, 7 down, 12 (5+7) up-down, 15 (1+2+5+7) torus (wrap completely).


    ++Mechanics++


    Under root, provide also:


/*----------------------------------------------------------------------------*/

EQUATION("diag_dist")
/* Once only, compute diagonal distance for performance reasons.*/
TRACK_SEQUENCE
    double diag_dist = sqrt( V("xn") * V("xn") + V("yn") * V("yn"));
    PARAMETER
RESULT(diag_dist)

FUNCTION("Gis_Init")
/* Initialise the population model. Called via fake-caller with c=Patch object */
TRACK_SEQUENCE
  int xn = V("xn");
  int yn = V("yn");

  TEST_MODE(xn<1 || yn<1){
    PLOG("\nERROR! Gis_Init: select xn,yn >= 1!");
    ABORT
    END_EQUATION(0.0);
  }

  int wrap = V("wrap");
  GIS_INIT(c->label,xn,yn,wrap)

  int n = COUNTS(c->up,c->label);
  //Add missing objects
  ADDNOBJS(c->up,c->label,xn*yn-n);

  GIS_IT_PATCH(cur_p_ext); //Get iterator to first patch
  //Cycle through patches in LSD and backend and link them.
  CYCLES(c->up,cur,c->label){
    WRITES(cur,GET_ID_LABEL(cur),cur_p_ext->ID);
    cur_p_ext->LSD_counterpart=cur;
    WRITES(cur,GET_VAR_LABEL(c,"_x"),cur_p_ext->x);
    WRITES(cur,GET_VAR_LABEL(c,"_y"),cur_p_ext->y);


    cur_p_ext=cur_p_ext->next; //move to next patch
  }

  VERBOSE_MODE(xn<6 && yn<6){
    int count = 0;
    PLOG("\nGIS @ LSD - testing patch assignments:\n")
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

// FUNCTION("Gis_LinkPatch")
// /* Link existing patch to existing backend-patch. Called via fake-caller with
// callee being the LSD object.  */
// TRACKSEQUENCE
//
//
// RESULT(0)

