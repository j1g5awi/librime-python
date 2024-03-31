set(PYTHONEXT_VERSION 1.0.0)
set(PYTHONEXT_SOVERSION 1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Python 3.6 COMPONENTS Interpreter Development REQUIRED)
include_directories(${Python_INCLUDE_DIRS})

find_package(pybind11)
if(pybind11_FOUND)
  include_directories(${pybind11_INCLUDE_DIRS})
else()
  message("Using in-tree pybind11 source")
  include_directories(pybind11/include)
endif()

aux_source_directory(src pythonext_src)

set(pythonext_library rime-pythonext)
set(pythonext_deps ${rime_library} ${Python_LIBRARIES})
set(pythonext_modules "pythonext")

add_library(rime-pythonext-objs OBJECT ${pythonext_src})
if(BUILD_SHARED_LIBS)
  set_target_properties(rime-pythonext-objs PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

set(plugin_name ${pythonext_library} PARENT_SCOPE)
set(plugin_objs $<TARGET_OBJECTS:rime-pythonext-objs> PARENT_SCOPE)
set(plugin_deps ${pythonext_deps} PARENT_SCOPE)
set(plugin_modules ${pythonext_modules} PARENT_SCOPE)
