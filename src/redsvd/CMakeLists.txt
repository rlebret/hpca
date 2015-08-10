FILE(
  GLOB
  redsvd_files
  *.cpp
  *.h
)
# Add "redsvd" library
ADD_LIBRARY(redsvd ${redsvd_files})
IF(USE_BLAS)
  TARGET_LINK_LIBRARIES(redsvd ${BLAS_LIBRARIES})
END(USE_BLAS)
IF(USE_LAPACK)
  TARGET_LINK_LIBRARIES(redsvd ${LAPACK_LIBRARIES})
END(USE_LAPACK)
