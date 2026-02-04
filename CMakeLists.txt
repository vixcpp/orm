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
include(FetchContent)

# ------------------------------------------------------------------------------
# Options
# ------------------------------------------------------------------------------
option(VIX_ORM_BUILD_TESTS "Build unit tests for Vix ORM" OFF)
option(VIX_ORM_BUILD_EXAMPLES "Build examples for Vix ORM" OFF)

# Standalone support:
# - Monorepo layout (vix/modules/orm): uses ../core and ../db
# - Standalone repo (orm only): fetches Vix (core+db) with FetchContent
option(VIX_ORM_FETCH_VIX_DEPS "Fetch vix core/db when building standalone" ON)

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
set(_VIX_ORM_HAS_MONOREPO_DEPS OFF)
if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/../core/CMakeLists.txt" AND
    EXISTS "${CMAKE_CURRENT_LIST_DIR}/../db/CMakeLists.txt")
  set(_VIX_ORM_HAS_MONOREPO_DEPS ON)
endif()

# If core/db targets are missing, try to provide them via:
# 1) monorepo add_subdirectory(../core, ../db)
# 2) FetchContent vix + add_subdirectory(modules/core, modules/db)
if (NOT TARGET vix::core OR NOT TARGET vix::db)
  if (_VIX_ORM_HAS_MONOREPO_DEPS)
    if (NOT TARGET vix::core)
      add_subdirectory(../core core_build)
    endif()

    if (NOT TARGET vix::db)
      add_subdirectory(../db db_build)
    endif()
  else()
    if (VIX_ORM_FETCH_VIX_DEPS)
      # NOTE: pin this to a tag or commit for reproducible builds if desired.
      FetchContent_Declare(vix
        GIT_REPOSITORY https://github.com/vixcpp/vix.git
        GIT_TAG main
      )
      FetchContent_MakeAvailable(vix)

      # Some upstream layouts may not add module targets automatically.
      if (NOT TARGET vix::core AND EXISTS "${vix_SOURCE_DIR}/modules/core/CMakeLists.txt")
        add_subdirectory("${vix_SOURCE_DIR}/modules/core" vix_core_build)
      endif()

      if (NOT TARGET vix::db AND EXISTS "${vix_SOURCE_DIR}/modules/db/CMakeLists.txt")
        add_subdirectory("${vix_SOURCE_DIR}/modules/db" vix_db_build)
      endif()
    endif()
  endif()
endif()

if (NOT TARGET vix::core OR NOT TARGET vix::db)
  message(FATAL_ERROR
    "vix_orm requires vix::core and vix::db.\n"
    "Build in the vix monorepo (vix/modules/orm) so ../core and ../db exist,\n"
    "or enable VIX_ORM_FETCH_VIX_DEPS=ON to fetch dependencies.")
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
if (_VIX_ORM_HAS_MONOREPO_DEPS)
  message(STATUS "[vix_orm] deps mode : monorepo (../core, ../db)")
else()
  if (VIX_ORM_FETCH_VIX_DEPS)
    message(STATUS "[vix_orm] deps mode : standalone (FetchContent vix)")
  else()
    message(STATUS "[vix_orm] deps mode : standalone (no fetch)")
  endif()
endif()
message(STATUS "------------------------------------------------------")
