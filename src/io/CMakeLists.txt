FILE(
  GLOB
  io_files
  *.cpp
  *.h
)
# Add "io" library
ADD_LIBRARY(io ${io_files})

# Linking
TARGET_LINK_LIBRARIES(io util)
