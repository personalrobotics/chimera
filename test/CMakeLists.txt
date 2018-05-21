include(CTest)

set(chimera_EXECUTABLE $<TARGET_FILE:chimera>)
include(chimeraFunctions)

# Set python version to be used to build bindings for.
if(NOT CHIMERA_TEST_PYTHON_VERSION)
  set(CHIMERA_TEST_PYTHON_VERSION 3.4 CACHE STRING
    "Choose the target Python version (e.g., 3.4, 2.7)" FORCE
  )
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Find PythonInter
find_package(PythonInterp ${CHIMERA_TEST_PYTHON_VERSION} REQUIRED)
execute_process(COMMAND ${PYTHON_EXECUTABLE} -c
  "from distutils.sysconfig import get_python_lib;\
  print(get_python_lib(plat_specific=True, prefix=''))"
  OUTPUT_VARIABLE PYTHON_SITE_PACKAGES
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Find boost with python components. The name of python component varies
# depending on the platform, boost version, and python version.
# TODO(JS): Check if thread component is really neccessary
if(APPLE)
  find_package(Boost REQUIRED
    COMPONENTS
      python${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR} thread
  )
  set(CHIMERA_TEST_Boost_PYTHON_LIBRARIES
    ${Boost_PYTHON${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR}_LIBRARIES}
  )
else() # LINUX assumed
  if(${PYTHON_VERSION_MAJOR} EQUAL 3)
    find_package(Boost
      COMPONENTS
        python-py${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR} thread
    )
    set(CHIMERA_TEST_Boost_PYTHON_LIBRARIES
      ${Boost_PYTHON-PY${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR}_LIBRARIES}
    )
    if(NOT Boost_FOUND)
      find_package(Boost REQUIRED COMPONENTS python3 thread)
      set(CHIMERA_TEST_Boost_PYTHON_LIBRARIES ${Boost_PYTHON3_LIBRARIES})
    endif()
  else() # Python 2 assumed
    find_package(Boost REQUIRED COMPONENTS python thread)
    set(CHIMERA_TEST_Boost_PYTHON_LIBRARIES ${Boost_PYTHON_LIBRARIES})
  endif()
endif()

# Find PythonLibs version that is the same with the version of PythonInterp.
find_package(PythonLibs ${CHIMERA_TEST_PYTHON_VERSION} REQUIRED)

add_subdirectory(dart_example)
