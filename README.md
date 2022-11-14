# radon_transform_opengl_implementation
This is simplest demo which could run Radon Transform on mali linux platform. 

## Prerequisite.
We need below to compile and run the demo. 
1. Juno platform with Mali GPU bitfile. 
2. Mali GPU DDK prebuilt on Ubuntu. 
3. Aarch64 linux toolchain installed and add to PATH. 

## Compile
1. Put the mali DDK libraries to <DDK_Path> (Default have Odin DDK at <Project_Root>/Libs/mali_ddk)
2. simply run make

## Run
1. Push mali_demo to Juno platform .
2. Push mali_ddk libs to Juno platform, and export the LD_LIBRARY_PATH. 
3. Push input_pic folder to Juno with same path as mali_demo.
4. Create output_pic folder at the path of where mali_demo presents. 
5. Simply run ./mali_demo

It will run for minutes and generated the Radon transformed pictures. 
