find_path(CSC_INCLUDE_DIR NAMES CSC.h PATHS “./include”) 
find_library(CSC_LIBRARY NAMES CSC.so PATHS “./” ) 
set(CSC_FOUND TRUE) 
set(CSC_INCLUDE_DIRS ${CSC_INCLUDE_DIR}) 
set(CSC_LIBS ${CSC_LIBRARY})
mark_as_advanced(CSC_INCLUDE_DIRS CSC_LIBS )
