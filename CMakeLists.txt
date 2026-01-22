#  @file CMakeLists.txt
#  @author Gaspard Kirira
#
#  Copyright 2025, Gaspard Kirira.  All rights reserved.
#  https://github.com/vixcpp/vix
#  Use of this source code is governed by a MIT license
#  that can be found in the License file.
#
#  Vix.cpp | ORM module (sugar layer on top of vix::db)
cmake_minimum_required(VERSION 3.20)

project(vix_orm VERSION 1.3.0 LANGUAGES CXX)

include(GNUInstallDirs)

# ------------------------------------------------------------------------------
# Options
# ------------------------------------------------------------------------------
option(VIX_ORM_BUILD_TESTS       "Build unit tests for Vix ORM"               OFF)
option(VIX_ORM_BUILD_EXAMPLES    "Build examples for Vix ORM"                OFF)

# language / PIC
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# warnings / sanitizers
set(_WARNINGS_GNU
  -Wall -Wextra -Wshadow -Wconversion
  -Wnon-virtual-dtor -Wold-style-cast -Woverloaded-virtual -Wpedantic
)
set(_WARNINGS_MSVC /W4 /permissive-)

# optional deps
find_package(spdlog QUIET CONFIG)

# ------------------------------------------------------------------------------
# Sources / Headers (ORM sugar only)
# ------------------------------------------------------------------------------
set(VIX_ORM_PUBLIC_HEADERS
  include/vix/orm/Entity.hpp
  include/vix/orm/Mapper.hpp
  include/vix/orm/Repository.hpp
  include/vix/orm/QueryBuilder.hpp
  include/vix/orm/UnitOfWork.hpp
  include/vix/orm/orm.hpp
  include/vix/orm/db_compat.hpp
)

set(VIX_ORM_SOURCES
  src/QueryBuilder.cpp
)

# ------------------------------------------------------------------------------
# Bring required modules when building standalone
# ------------------------------------------------------------------------------
if (NOT TARGET vix::core)
  add_subdirectory(../core core_build)
endif()

if (NOT TARGET vix::db)
  add_subdirectory(../db db_build)
endif()

# ------------------------------------------------------------------------------
# Library target
# ------------------------------------------------------------------------------
add_library(vix_orm STATIC ${VIX_ORM_SOURCES} ${VIX_ORM_PUBLIC_HEADERS})
add_library(vix::orm ALIAS vix_orm)

set_target_properties(vix_orm PROPERTIES EXPORT_NAME orm)

target_include_directories(vix_orm PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(vix_orm PUBLIC vix::core vix::db)

if (VIX_ENABLE_SANITIZERS AND TARGET vix_sanitizers)
  target_link_libraries(vix_orm PUBLIC vix_sanitizers)
endif()

if (MSVC)
  target_compile_options(vix_orm PRIVATE ${_WARNINGS_MSVC})
else()
  target_compile_options(vix_orm PRIVATE ${_WARNINGS_GNU})
endif()

# deps / defines
if (spdlog_FOUND)
  if (TARGET spdlog::spdlog_header_only)
    target_link_libraries(vix_orm PUBLIC spdlog::spdlog_header_only)
  elseif (TARGET spdlog::spdlog)
    target_link_libraries(vix_orm PUBLIC spdlog::spdlog)
  endif()
  target_compile_definitions(vix_orm PUBLIC VIX_ORM_HAS_SPDLOG=1)
else()
  target_compile_definitions(vix_orm PUBLIC VIX_ORM_NO_LOGGER=1)
endif()

# ------------------------------------------------------------------------------
# Examples
# ------------------------------------------------------------------------------
function(vix_add_orm_example name file)
  add_executable(${name} ${file})
  target_link_libraries(${name} PRIVATE vix::orm)

  if (VIX_ENABLE_SANITIZERS AND TARGET vix_sanitizers)
    target_link_libraries(${name} PRIVATE vix_sanitizers)
  endif()

  if (MSVC)
    target_compile_options(${name} PRIVATE ${_WARNINGS_MSVC})
  else()
    target_compile_options(${name} PRIVATE ${_WARNINGS_GNU})
  endif()
endfunction()

if (VIX_ORM_BUILD_EXAMPLES)
  vix_add_orm_example(repository_crud_full
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/repository_crud_full.cpp)
  vix_add_orm_example(querybuilder_update
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/querybuilder_update.cpp)
  vix_add_orm_example(tx_unit_of_work
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/tx_unit_of_work.cpp)
  vix_add_orm_example(error_handling
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/error_handling.cpp)
endif()

# ------------------------------------------------------------------------------
# Install / export via umbrella export-set "VixTargets"
# ------------------------------------------------------------------------------
if (DEFINED VIX_UMBRELLA_BUILD)
  install(TARGETS vix_orm
    EXPORT  VixTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )
else()
  install(TARGETS vix_orm
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )
endif()

install(DIRECTORY include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h"
)

message(STATUS "------------------------------------------------------")
message(STATUS "[vix_orm] configured (${PROJECT_VERSION})")
message(STATUS "[vix_orm] depends on: vix::db (drivers) + vix::core (base)")
message(STATUS "------------------------------------------------------")
