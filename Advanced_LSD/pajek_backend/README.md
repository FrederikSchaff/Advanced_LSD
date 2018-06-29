# PajekFromCpp

Migrated from the git snapshot: https://github.com/FrederikSchaff/PajekFromCpp 

Stand alone c++ source code to create pajek .net and .paj (including timeline feature for PajekToScgAnim) files from any simulation in c/c++.

This code is generated and tested for the use with LSD by Marco Valente (http://www.labsimdev.org/Joomla_1-3/, https://github.com/marcov64/Lsd), a c++ based framework for agent-based modeling. It is not a substitute to the network functionality implemented by Marcelo Pereira in current snapshots of LSD. It is only ment to be used to export network-data from a simulation in such a manner that it may be analysed and visualised with pajek or other software that can make use of the .paj or .net format.  The intention is to provide full flexibility and all the options that pajek by Andrej Mrvar (http://mrvar.fdv.uni-lj.si/pajek/) .paj (and .net) format provides. In addition, the .paj files created may be used with PajekToScgAnim by Darko Brvar: - PajekToSvgAnim (by Darko Brvar): http://mrvar.fdv.uni-lj.si/pajek/PajekToSvgAnim/PajekToSvgAnim.zip - test datasets: http://mrvar.fdv.uni-lj.si/pajek/PajekToSvgAnim/AnimData.zip - manual: http://freeweb.siol.net/dbrvar2/PajekToSvgAnim11081.pdf 

### Usage

####Inclusion in LSD: 

now part of LSD_Advanced!
Add somewhere before ==MODELBEGIN==: ``` #define MODULE_PAJEK ```

###How to use it

##### General Info
- Errors and Infos are printed as default. Turn off via ```#define PAJEK_MSG_OFF``` .

- There are two modes available: single snapshot mode and append mode:

  - single snapshot mode: Each snapshot is saved in a single .net

  - append mode: 

    a) A (set of) *.paj files is created, holding all the snapshots. Via ```#define PAJEK_MAX_SNAPSHOTS``` 

       the number of snapshots per file *_part*.pay is defined. (default 200) 

    b) ALTERNATIVELY, if ==PAJEK_FORCE_COMPLETE== is defined true (default), a single *.paj file is created holding all snapshots and at the beginning of this file a special snapshot is created, holding all the time-line information, thus making it suitable to work with e.g. PajekToSvgAnim

- All the ==#define== are set in the Pajek.h file.

- Note: Currently a lot of information is static and needs to be provided in advance (like max number of vertices/edges, max number of snapshots, ...) - this will be changed in future (early... )

#####Usage (within LSD - inside EQUATIONS)

1. At the beginning of a new simulation run, initialise a new network via one of the commands:

   ```PAJEK_INIT; //Initialise standard, using seed as suffix for networks and putting all files in a folder "Network/Default_NW", there will be one big file containt all the informatio"```

    ```PAJEK_INIT_S; //same, but for each single time-step a file is created```

   ```PAJEK_INIT_X(ID,folder,suffix); //Initialise, provide explicit information for simulation ID, folder (Network/folder) for storring the snapshots and suffix for the single files.```

   ```PAJEK_INIT_XS(ID,folder,suffix); //as before, but with single snapshots per timestep stored.```

2. Add different kinds of arcs/edges if wanted via:
   ```PAJEK_INIT_NEW_RELATION(name,isedge);```. 
   The minimum is one kind of relation.

3. Each time step, you need to add information on all active nodes and/or links. Also, make sure that for links the target and source nodes need to exist.

   1. Each time you add network links use the according commands:
      ```PAJEK_EDGE(source,target,name,value); ```
      ```PAJEK_EDGE_X(source,target,name,value,width,colour); ```
      ```PAJEK_EDGE_XT(source,target,name,value,width,colour,start,end);``` 
      You may substitute EDGE by ARC to make the links directed (from source to target). The "X" Option allows you to add a second attribute (integer >= 0) as information, initially interpreted as width in pajek, and a third one (discrete string, e.g. "Yellow" - don't miss the parenthesis!) interpreted as colour in pajek. See below for the string values. The "XT" option allows to specify different start and end times. I do not remember how it works... better to use the standard versions and create (new) arcs each time they exist - you cannot know when they cede to exist in front anyway.

   2. Each time you add network nodes use one of the following commands:
      ```#define PAJEK_VERTICE(ID,name,value)```

      ```#define PAJEK_VERTICE_X(ID,name,value,shape,colour,x_size,y_size)```
      ```#define PAJEK_VERTICE_XP(ID,name,value,shape,colour,x_size,y_size,x_coor,y_coor)```
      ```#define PAJEK_VERTICE_XPT(ID,name,value,shape,colour,x_size,y_size,x_coor,y_coor,start,end)```

      The name can be unique or, more likely, a non unique label. In this case the zero-padded ID will be added to the names, such that they are unique again (relevant for pajek). The shape can be defined using an integer and translates as follows:

      ```
      case 0: return "ellipse"; case 1: return "box"; case 2: return "diamond";
      case 3: return "triangle"; case 4: return "cross"; case 5: return "empty";
      case 6: return "house"; case 7: return "man"; case 8: return "woman"; 
      default: return "ellipse";
      ```

      Note that pajek SvgToAnim does not support woman/man symbol. As default this is changed to diamond(man) and triangle(woman). If you want to explicitly use woman/man shapes, ```#define PAJEK_NOTUSE_WOMANMAN true``` before including the LSD_Advanced stuff.

4. At the end of a time-period, use the command:

   ```PAJEK_SNAPSHOT;``` 
   It will automatically determine if this is just data added to the complete list of vertices/edges or if the simulation is at end and the data shall be saved, either because ```t==max_t``` or ```quit != 0```. 

5. If you want to take a snapshot of an initial configuration, you can use the command:
   ```PAJEK_SNAPSHOT_ZERO;```

   But make sure to use it only once and before using ```Pajek_Snapshot;``` 

See the description of the #defines here and the pajek_ functions in the accompanying .cpp file for more information. Note: Not all functionality is documented here (yet)

Notes regarding PajekToSVGAnim:
Always make sure that at least one arc/edge exists at each time-slice!

### Colours (from Pajek manual, p. 90)

You can assign the colours by using the name as code
| | | | | | | | |
| - | - | - | - | - | - | - | ------ |
| 0 - Cyan | 1 - Yellow |  2 - LimeGreen | 3 - Red | 4 - Blue | 5 - Pink | 6 - White | 7 - Orange | 8 - Purple || 9 - CadetBlue | 10 - TealBlue | 11 - OliveGreen | 12 - Gray  | 13 - Black |  14 - Maroon | 15 - LightGreen | 16 - LightYellow |
| 17 - Magenta | 18 - MidnightBlue | 19 - Dandelion | 20 - WildStrawberry | 21 - ForestGreen | 22 - Salmon | 23 - LSkyBlue | 24 - GreenYellow |
| 25 - Lavender | 26 - LFadedGreen | 27 - LightPurple | 28 - CornflowerBlue | 29 - LightOrange | 30 - Tan | 31 - LightCyan | 32 - Gray20 |
| 33 - Gray60 | 34 - Gray40 | 35 - Gray75 | 36 - Gray10 | 37 - Gray85 | 38 - Gray30 | 39 - Gray70 |



### To do's
- Convert to more convenient class structure and using vectors instead of arrays
- Provide thorough comments in the code
- Provide a real manual

###Misc
The code is provided "as is". You may reach out to the author via E-Mail at: [frederik.schaff@fernuni-hagen.de](frederik.schaff@fernuni-hagen.de)

For further information on Pajek see also:

Mrvar, Andrej; Batagelj, Vladimir (2016): Analysis and visualization of large
networks with program package Pajek. In: Complex Adaptive Systems Modeling 4(1),
 S. 1-8. DOI: 10.1186/s40294-016-0017-8.

Nooy, Wouter de; Mrvar, Andrej; Batagelj, Vladimir (2011): Exploratory social
network analysis with Pajek. Rev. and expanded 2nd ed. England, New York:
Cambridge University Press (Structural analysis in the social sciences, 34).
