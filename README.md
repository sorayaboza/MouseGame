### Game Credits
Game made by **Soraya Boza** and **Harshil Taunk**.

<img width="400" height="225" alt="Mouse Heist Demo" src="https://github.com/user-attachments/assets/9a19e920-8b20-46a0-92a8-993593c2ce39" />


#### Soraya Boza
* **Roles**: Team lead, programmer, artist, tester, game designer.
* **Team leaf / Game designer**: Made all creative decisions (except for the idea to add an ability).
* **Programmer**: Worked on the skybox, player movement, abilities, food system, terrain system, and camera movement.
* **Artist**: Created all of the assets for this game, except for the music (royalty free), models (which I got from [here](https://www.quaddicted.com/files/idgames2/planetquake/q2pmp/), the Tygris, Nightcrawler, and Awolf) and the watermelon, cheese, donut, and milk pngs.
* **Tester**: Tested the game at each stage.

#### Harshil Taunk
* **Roles**: Programmer, Debugger and Tester
* Programmer: Worked on the game state machine, save/load system, enemy AI (patrol/chase/distracted states with vision cones), the in-game HUD, different level/ pause screens, sound system integration, mouse-look camera, the MEGA-HIT mechanic.
* Debugger: Fixed UI fading between levels, score bug, ENTER-lag between levels, stacked music tracks on state transitions, and the hole alignment with the art.
* Tester: Playtested levels while debugging to catch collision, spawn, and alignment issues.

### Setup for running the game

#### Required installations

Make sure [MinGW](https://sourceforge.net/projects/mingw/) is installed.
* Install all but FORTRAN.
* Installation -> Update catalogue

This project was made in CodeBlocks. Download CodeBlocks [here](https://www.codeblocks.org/).
* Settings > Compiler > Toolchain executables
* Instead of `c://minGW`, click the 3 dots, redirect to `C://MinGW`
* Change the mingw32 files at the bottom to 
     mingw32-gcc.exe, 
     mingw32-c++.exe,
     mingw32-g++.exe

Place files in the `windows files` folder in `C:\Windows`.

Place the `common` folder in the same location as your base folder. For example, if you keep cloned repositories in the `Desktop/Projects` folder,
then it would look like:
`Desktop/Projects/MouseGame`
`Desktop/Projects/common`

#### Using CodeBlocks

Now that you have CodeBlocks and MinGW installed, follow these steps to ensure this game runs properly.

In CodeBlocks:
File > New > Project > GLUT Project
* Name your project
* Press Next
* Make sure it's located in `\[folder name]\common`
* Press Next then Finish

In your folder system, copy the following files from the folder you want copied:
* images
* include
* models
* sounds
* src
* ikpFlac.dll
* ikpMP3.dll
* irrKlang.dll
* main
Paste these into your new folder you just made. Replace files in destination.

Back in CodeBlocks, right click your main folder (should be bolded under `Workspace`).
* Add files recursively > Select Folder > Ok > Ok

Since you just made a new folder, you also need to do the following:
Project > Properties > Project's build options... > Linker settings > Add > click blue folder icon > navigate to common/lib > add `libSOIL.a`, `glew32.lib` and `irrKlang.lib`> Yes > Ok
* Select `..\common\lib\glew32.lib` and click the green arrow up button until it's at the top. Do the same with `..\common\lib\libSOIL.a` and `irrKlang.lib`, but make sure that `libSOIL.a` and `glew32.lib` go on top of `irrKlang.lib`.
* Ok > Ok > etc. to exit.

Then:
* File > New > Class > Create a dummy class (any name) > Yes > Yes (important that you press yes to adding files)
* Once this is done, you can delete the temp files from \src and \include.

Reminder:
`glut32.dll `and `glew32.dll` must be in the Windows folder. (C:\Windows)

After that, you should be able to build with no problems!
