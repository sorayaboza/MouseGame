## When copying files onto new project
File > New > Project > GLUT Project
* Name your project
* Press Next
* Make sure it's located in `\291Spring26\common`
* Press Next then Finish

In your folder system, copy the following files from the folder you want copied:
* images
* models
* include
* src
* main
Paste these into your new folder you just made. Replace files in destination.

Back in CodeBlocks, right click your main folder (should be bolded under Workspace)
* Add files recursively > Select Folder > Ok > Ok

Since you just made a new folder, you also need to do the following:
Project > Properties > Project's build options... > Linker settings > Add > click blue folder icon > navigate to common/lib > add `libSOIL.a` and `glew32.lib` > Yes > Ok
* Select `..\common\lib\glew32.lib` and click the green arrow up button until it's at the top. Do the same with `..\common\lib\glew32.lib`.

If you get a .h error:
* File > New > Class > Create a dummy class (any name) > Yes > Yes (important that you press yes to adding files)
* Once this is done, you can delete the temp files from \src and \include.

`glut32.dll `and `glew32.dll` must be in the Windows folder. (C:\Windows)
