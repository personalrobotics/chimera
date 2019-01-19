include(CTest)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(chimera_EXECUTABLE $<TARGET_FILE:chimera>)
include(chimeraFunctions)
include(chimeraTest)

# Set python version to be used to build bindings for.
if(NOT CHIMERA_TEST_PYTHON_VERSION)
  set(CHIMERA_TEST_PYTHON_VERSION 3.4 CACHE STRING
    "Choose the target Python version (e.g., 3.4, 2.7)" FORCE
  )
endif()

# Find PythonInterp
find_package(PythonInterp ${CHIMERA_TEST_PYTHON_VERSION} REQUIRED)
execute_process(COMMAND ${PYTHON_EXECUTABLE} -c
  "from distutils.sysconfig import get_python_lib;\
  print(get_python_lib(plat_specific=True, prefix=''))"
  OUTPUT_VARIABLE PYTHON_SITE_PACKAGES
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Find PythonLibs version that is the same with the version of PythonInterp.
find_package(PythonLibs ${CHIMERA_TEST_PYTHON_VERSION} REQUIRED)

# Find boost with python components. The name of python component varies
# depending on the platform, boost version, and python version.
# TODO(JS): Check if thread component is really neccessary
if(APPLE)
  find_package(Boost QUIET
    COMPONENTS
      python${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR} thread
  )
  set(CHIMERA_TEST_Boost_PYTHON_LIBRARIES
    ${Boost_PYTHON${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR}_LIBRARIES}
  )
else() # LINUX assumed
  if(${PYTHON_VERSION_MAJOR} EQUAL 3)
    find_package(Boost QUIET
      COMPONENTS
        python-py${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR} thread
    )
    set(CHIMERA_TEST_Boost_PYTHON_LIBRARIES
      ${Boost_PYTHON-PY${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR}_LIBRARIES}
    )
    if(NOT Boost_FOUND)
      find_package(Boost QUIET COMPONENTS python3 thread)
      set(CHIMERA_TEST_Boost_PYTHON_LIBRARIES ${Boost_PYTHON3_LIBRARIES})
    endif()
  else() # Python 2 assumed
    find_package(Boost QUIET COMPONENTS python thread)
    set(CHIMERA_TEST_Boost_PYTHON_LIBRARIES ${Boost_PYTHON_LIBRARIES})
  endif()
endif()
if(NOT CHIMERA_TEST_Boost_PYTHON_LIBRARIES)
  message(WARNING "Boost.Python is not found. Omitting from unit tests.")
endif()

# Find pybind11
# Needs to set PYBIND11_PYTHON_VERSION before finding pybind11
set(PYBIND11_PYTHON_VERSION ${PYTHON_VERSION_STRING})
find_package(pybind11 2.2.0 QUIET)
if(NOT pybind11_FOUND)
  message(WARNING "pybind11 (>=2.2.0) is not found. Omitting from unit tests.")
endif()

# Add unit tests from subdirectories (roughly in order of complexity).
add_subdirectory(01_function)
add_subdirectory(02_class)
add_subdirectory(03_smart_pointers)
add_subdirectory(04_enumeration)
add_subdirectory(05_variable)
add_subdirectory(99_dart_example)
add_subdirectory(regression)

# Add custom target `binding_tests` to build all the tests as a single target
get_property(chimera_pybind11_binding_tests GLOBAL
  PROPERTY CHIMERA_PYBIND11_BINDING_TESTS
)
get_property(chimera_boost_python_binding_tests GLOBAL
  PROPERTY CHIMERA_BOOST_PYTHON_BINDING_TESTS
)
add_custom_target(binding_tests
  DEPENDS
    ${chimera_pybind11_binding_tests}
    ${chimera_boost_python_binding_tests}
)
