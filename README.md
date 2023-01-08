# Energy Management System Display module firmware

See my blog at https://miscsolutions.wordpress.com/2022/10/02/home-energy-management-system-part-1-objectives/ for more details.

# Build instructions

In Eclipse IDE for C/C++ developers, create a workspace and import the following projects into it:
- This project
- Branch 3.5-picoW of project CoreN2G from https://github.com/Duet3D/CoreN2G/tree/3.5-picoW
- Branch 3.4 of FreeRTOS from https://github.com/Duet3D/FreeRTOS
- Branch 3.4 of RRFLibraries from https://github.com/Duet3D/RRFLibraries

Update submodule lvgl of this project by running **git submodule update**

You should then be able to build this project in Eclipse.
