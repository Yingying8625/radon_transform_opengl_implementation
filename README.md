# radon_transform_opengl_implementation

# Prerequisite.
We need below to compile and run the demo.

1. vscode or visual studio(vscode need other dependencies such as cmake and mingw64)
2. opencv

As a reference, https://blog.csdn.net/MOZHOUH/article/details/124979715 and https://blog.csdn.net/Avrilzyx/article/details/107036375 will help to build the environment.

# Compile
1. mkdir build & cd build
2. cmake ..
3. make
Then it will generate a .exe at build folder. 

# Run
Simply run .build/radon
It will run for several hours and generated the Radon transformed pictures. If you only test one picture, you can change the code in src/main.cpp
