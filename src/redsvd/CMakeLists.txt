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
ENDIF(USE_BLAS)
IF(USE_LAPACK)
  TARGET_LINK_LIBRARIES(redsvd ${LAPACK_LIBRARIES})
ENDIF(USE_LAPACK)
