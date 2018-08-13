//a simple test program
#include "Pajek_new.cpp"
#include <random>

int main(){
  std::random_device rd; // to seed mt
  std::mt19937 prng(rd()); //the mt prng
  std::uniform_int_distribution<int> distr(0,2);
  std::uniform_real_distribution<double> uniform(0,1);
  std::uniform_int_distribution<int> distr_x(0,7);
  std::uniform_int_distribution<int> distr_y(0,5);
  std::uniform_int_distribution<int> distr_patch(0,48);

    //simple test that should work
  PAJ_MAKE_AVAILABLE
  PAJ_INIT_ANIM("Network","TestSet",1,"Test Set")
  for (int t = 1; t<5; t++){
    PAJ_STATIC("Network","TestSet",1,"Test Set") //create static info allongside
    int pid = 0;
    for (double x=0; x<8; x++) {
      for (double y=0; y<6; y++) {
        pid++;
        PAJ_ADD_V_C(t,pid,"Patch",1,x,y,"box",10,10,distr(prng)<1?"Black":(distr(prng)<2?"Red":"Blue"))

        PAJ_S_ADD_V_C(t,pid,"Patch",1,x,y,"box",10,10,distr(prng)<1?"Black":(distr(prng)<2?"Red":"Blue"))
      }
    }
    PAJ_ADD_V_C(t,1,"Dog",1,distr_x(prng),distr_y(prng),"triangle",1,1,"Green")
    PAJ_ADD_V_C(t,1,"Person",1,distr_x(prng),distr_y(prng),"ellipse",2,2,"Gray")
    PAJ_ADD_A(t,1,"Dog",distr_patch(prng),"Patch",1,"Arc")
    PAJ_ADD_A(t,1,"Person",1,"Dog",1,"Edge")

    PAJ_S_ADD_V_C(t,1,"Dog",1,distr_x(prng),distr_y(prng),"triangle",1,1,"Green")
    PAJ_S_ADD_V_C(t,1,"Person",1,distr_x(prng),distr_y(prng),"ellipse",2,2,"Gray")
    PAJ_S_ADD_A(t,1,"Dog",distr_patch(prng),"Patch",1,"Arc")
    PAJ_S_ADD_A(t,1,"Person",1,"Dog",1,"Edge")
    PAJ_S_SAVE
  }

  PAJ_SAVE


//   pajek_core_object.is_normed=false;
//   pajek_core_object.norm_coords();
}
