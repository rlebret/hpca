FILE(
  GLOB
  redsvd_files
  *.cpp
  *.h
)
# Add "redsvd" library
ADD_LIBRARY(redsvd ${redsvd_files})
TARGET_LINK_LIBRARIES(redsvd ${BLAS_LIBRARIES} ${LAPACK_LIBRARIES})