# cmake .. -DCMAKE_INSTALL_PREFIX=/usr/ -DCMAKE_TOOLCHAIN_FILE=../CMakeModules/Cross_x86_64.cmake

# the name of the target operating system
set(CMAKE_SYSTEM_NAME      Linux)    # If set, CMAKE_CROSSCOMLING is automatically set as TRUE
set(CMAKE_SYSTEM_VERSION   1)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# where is the target environment
set(CMAKE_FIND_ROOT_PATH /crosscompiler/x86_64)

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM BOTH)

# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

SET(CMAKE_C_FLAGS "-m64 ${CMAKE_C_FLAGS}")
SET(CMAKE_CXX_FLAGS "-m64 ${CMAKE_CXX_FLAGS}")

SET(CMAKE_EXE_LINKER_FLAGS        "-m64 ${CMAKE_EXE_LINKER_FLAGS}")
SET(CMAKE_SHARED_LINKER_FLAGS     "-m64 ${CMAKE_SHARED_LINKER_FLAGS}")
SET(CMAKE_MODULE_LINKER_FLAGS     "-m64 ${CMAKE_MODULE_LINKER_FLAGS}")