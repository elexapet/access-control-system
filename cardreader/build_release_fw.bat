@echo off
cmake -G Ninja -DARM_COMPILER_PATH="C:\nxp\MCUXpressoIDE_10.3.1_2233\ide\tools\bin\arm-none-eabi-gcc.exe" -DCMAKE_TOOLCHAIN_FILE=cmake/arm-gcc-toolchain.cmake -DCMAKE_BUILD_TYPE=Release -B Release
cmake --build Release --clean-first --target all
pause