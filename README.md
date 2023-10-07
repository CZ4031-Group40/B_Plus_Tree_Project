# B+ Tree Project

## Setup to run

### Windows:

1. make sure cmake is installed (>=3.26). Check by running
   ```
   cmake --version
   ```
   Otherwise can install from https://cmake.org/download/
2. Go to project directory and create a new directory build
   ```
   mkdir build
   cd build
   ```
3. Run these two command to compile and build the exe file
   ```
   cmake ..
   cmake --build .
   ```
4. An exe file should be created inside the Debug folder run this exe in console by running
   ```
   .\Debug\B_Plus_Tree_Project.exe
   ```

### Mac

1. Install CMake through
   ```bash
   sudo apt-get update
   sudo apt-get install cmake
   ```
   Alternatively you can use brew,
   ```bash
   brew install cmake
   ```
2. In the project directory, create dir named 'build' and navigate into it. This is to keep the source directory clean.
   ```bash
   mkdir build && cd build
   ```
3. Run this command to generate build files and build the project
   ```
   cmake .. && make
   ```
4. Run the executable
   ```bash
   ./B_Plus_Tree_Project
   ```

## Experiments

The script related to experiment is located in `/src/experiment.cpp`.

- This script creates a record for each row from `/data/games.txt` and drops the rows with missing value
- It prints out the relevant statistics regarding data insertion.
- It prints out the relevant statistics and parameters of the B+ tree.
- 
### Windows
To run this script follow step 1 to 3 on top replace step 4 with
```
.\Debug\B_Plus_Tree_Project.exe
```
   
### Mac
To run this script, run `./experiment` in the `/build` folder. Ensure you have done the setup above before running the script.


If the script is not running whitin expectation please email
twijaya001@e.ntu.edu.sg
