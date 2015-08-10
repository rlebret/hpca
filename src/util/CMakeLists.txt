FILE(
  GLOB
  util_files
  *.cpp
  *.h
)
# Add "util" library
ADD_LIBRARY(util ${util_files})