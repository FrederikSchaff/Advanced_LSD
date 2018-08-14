# Notice 

This is current work in progress and under heavy development. It can at most be considered an early alpha.
If you are interested in this work and would perhaps like to contribute, do not hesitate to contact me (frederik.schaff@rub.de).

## Recent changes

- The pajek module is now fully functional (see the separate readme in "Advanced_LSD\pajek_backend").

# Advanced_LSD

This is both, a template and a set of utilities/modules for LSD (GIS, Population, Analysis, Pajek, ...)
LSD is a c++-based agent-based modelling framework by Marco Valente, see: <https://github.com/marcov64/Lsd>

## Content

This is only an overview. Inside the main program folder there is another folder "Advanced_LSD" which holds the single modules (as folders) and the means to integrate them into LSD. Inside each of the single folders there is a readme with some more information.

- ABMAT - ABM Analysis Toolkit
  The ABMAT Module aims at facilitating the analysis of complex ABMs by gathering descriptive statistics at the end of the single runs, which are then saved for latter analysis instead of all the time-series information. This reduces the amount of disk space needed and also improves the number of measurements that can routinely be made and considered for the analysis. 
- pop_backend - a population model and backend to track heritage networks and allow faster access of agents
- gis_backend - a GIS backend to provide a geographic representation in LSD and routines to work on it.
- pajek_backend - a backend providing utility to create pajek project files (.paj format) holding network information (in .net format) without any restrictions. This is purely for ex-post analytical usage and the data structures cannot be used for the model itself. 
- tools - some additional tools for debugging and validation of complex LSD models
  

## Usage 

In order to use it, clone the current version of LSD 7.1 to your desktop and clone the folder 'Advanced_LSD' into the 'Work' folder in LSD. You may then select the model.The fun_templ_advancedLSD.cpp can be used as a template. It provides all the utility necessary.

Note: Two libraries, random and eigen, need to be included prior to fun_head_fast.h. Therefore, fun_head_fast.h is also included via Advanced_LSD.h.

##Misc

The model is not yet documented very much and the different "Modules" are in a rather raw state, too. But we are currently developing it heavily.

Any comments or questions are welcome. Please drop an E-Mail at Frederik.Schaff@rub.de

## To Do's

Actually there is a lot of potential improvement and things that are currently going to be improved. Here, I will only list some of the things that can expected to be done in the next weeks.

- [ ] Change all the backends to a MACRO structure, such that they become easier to use and the usage can be documented easier. Also, this way the usage is more in line with standard LSD usage.
- [x] Update the pajek backend to STL container usage and make it dynamic.

## Note on the License

I do not know much about licensing so it might be good to declare the intentions here. The file 'fun_templ_advancedLSD.cpp' is only a template showing how the Advanced_LSD tools can be linked to ones own model. Obviously, the model of anybody using this project shall not fall under the GPL. The same holds for other files in the main folder. The real "content" of this project resides in the subfolder Advanced_LSD (including, e.g., Advanced_LSD.h). The license only applies to this folder.

I chose the GPL license because LSD is also licensed under GPL.

