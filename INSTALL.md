# INSTALL
 
## CONFIGURE OPTIONS

 * To create an executable using Intel MKL through Eigen add the option:
   `-DEIGEN_USE_MKL_ALL=ON`
 
 * By default a release version is created. To create a debug version add the option:
   `-DCMAKE_BUILD_TYPE=Debug`

   To go back to a release version:
   `-DCMAKE_BUILD_TYPE=Release`
   

 **Example**: To create a debug version with MKL
 ```
 ./configure -DEIGEN_USE_MKL_ALL=ON -DCMAKE_BUILD_TYPE=Debug
 ``` 

## WINDOWS

If using Cygwin:
```
 ./configure
 make
 make install
```

If using Visual Studio:

Follow the directions at the link for running CMake on Windows:
http://www.opentissue.org/wikitissue/index.php/Using_CMake
    
**NOTE**: Select the `build` folder as the location to build the binaries.
    
    
## MAC OS X
```
 ./configure
 make
 sudo make install
```

## LINUX
```
 ./configure
 make
 sudo make install
```
 
## DOCUMENTATION
 
 If you want to make the manual, issue the following command:
 (you need Doxygen installed on your system)

 ```
 make doc
 ```

 You can browse the documentation in the directory `doc/html`.