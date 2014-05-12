eie-ic-renderer
===============
You can find my thesis in /finalreport.pdf

If you somehow get it to build and run on your computer, you can make things
happen by pressing the "K" key. This moves you forward a slide. "J" moves you
back a slide.

The rest of the controls are in the user guide of the thesis (page 85).
Alternatively, I've copy and pasted the user guide here. Apologies for the
dodgy formatting.

9 User Guide

9.1 Compilation

The source code repository for the project can be found at https://github.com/
n1kunj/eie-ic-renderer.
Compilation requires Visual Studio 2012, the Windows 8 SDK and the DirectX
SDK.

Loading the eie-ic-renderer.sln in the source folder will open Visual Studio, and
the code can be easily compiled. Ensure that 64-bit compilation is used, as the
32-bit version crashes due to SIMD alignment issues.

9.2 Running the Code

To run the code, eie-ic-renderer.exe needs to be run in the same directory as the
Media folder, which contains the shaders and the Lua scripts.

The initial load is quite slow as all the shaders need to be compiled at run
time. Once compilation is finished, the application window will appear and the
Lua script ”mountains.lua” will automatically be run. Dependent on GPU perfor-
mance, loading time could range from instant to 3 seconds. Within the application,
the controls are as follows:

W, S, A and D will move the camera forwards, backwards, left and right, re-
spectively.

Space and Ctrl will move the camera up and down, respectively.

Holding the Left Mouse Button will change the camera angle.

F will toggle locking the mouse to the window, allowing for the camera angle to be
changed without holding down the left mouse button. There is a minor bug
where, if the window is not positioned over the centre point of the screen,
mouse locking will not work correctly.

R will recompile all shaders. This will lock up the program for a short amount of
time.

Tilde will toggle between the developer console and the frame rate counter.

The developer console is a fully functional Lua interpreter, though only a small
number of libraries have been loaded in. From within the developer console, the
following commands can be used to change some settings:

SET.wireframe = {1,0} will switch wireframe on or off.

SET.cameraspeed = 100 will set the camera speed to 100m/s. The default
speed is 1000m/s.

SET.fxaa = {1,0} will turn FXAA on or off.

move(x,y,z) will set the camera position to (x,y,z). The y axis points directly
upwards.

print(x) will print the value or variable x to screen.

run(”water.lua”) will run the script water.lua in the Media\Lua folder, which
creates a simple water texture.