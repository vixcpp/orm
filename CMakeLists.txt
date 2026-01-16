#  @file CMakeLists.cpp
#  @author Gaspard Kirira
#
#  Copyright 2025, Gaspard Kirira.  All rights reserved.
#  https://github.com/vixcpp/vix
#  Use of this source code is governed by a MIT license
#  that can be found in the License file.
#
#  Vix.cpp | ORM module
cmake_minimum_required(VERSION 3.20)

project(vix_orm VERSION 1.2.1 LANGUAGES CXX)

include(GNUInstallDirs)

# options
option(VIX_ORM_BUILD_TESTS       "Build unit tests for Vix ORM"               OFF)
option(VIX_ORM_BUILD_EXAMPLES    "Build examples for Vix ORM"                ON)

option(VIX_ORM_USE_MYSQL         "Enable MySQL Connector/C++ driver"         ON)
option(VIX_ORM_REQUIRE_MYSQL     "Fail if MySQL requested but not found"     OFF)

option(VIX_ORM_USE_SQLITE        "Enable SQLite3 driver"                     OFF)
option(VIX_ORM_BUILD_TOOLS "Build ORM CLI tools (migrator)" ON)

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

# MySQL detection
set(VIX_ORM_HAS_MYSQL OFF)
set(MYSQLCPPCONN_LIB "")
set(MYSQLCPPCONN_INCLUDE_DIR "")

if (VIX_ORM_USE_MYSQL)
  if (EXISTS "/usr/lib/x86_64-linux-gnu/libmysqlcppconn.so")
    set(MYSQLCPPCONN_LIB "/usr/lib/x86_64-linux-gnu/libmysqlcppconn.so")
  elseif (EXISTS "/usr/lib/x86_64-linux-gnu/libmysqlcppconn.so.7")
    set(MYSQLCPPCONN_LIB "/usr/lib/x86_64-linux-gnu/libmysqlcppconn.so.7")
  endif()

  if (EXISTS "/usr/include/cppconn/connection.h")
    set(MYSQLCPPCONN_INCLUDE_DIR "/usr/include")
  endif()

  if (NOT MYSQLCPPCONN_LIB)
    find_library(MYSQLCPPCONN_LIB
      NAMES mysqlcppconn mysqlcppconn8
      PATHS /usr/lib/x86_64-linux-gnu /usr/local/lib /usr/lib
    )
  endif()

  if (NOT MYSQLCPPCONN_INCLUDE_DIR)
    find_path(MYSQLCPPCONN_INCLUDE_DIR
      NAMES cppconn/connection.h
      PATHS /usr/include /usr/local/include
    )
  endif()

  if (MYSQLCPPCONN_LIB AND MYSQLCPPCONN_INCLUDE_DIR)
    set(VIX_ORM_HAS_MYSQL ON)
    message(STATUS "[vix_orm] mysql: enabled")
    message(STATUS "         lib: ${MYSQLCPPCONN_LIB}")
    message(STATUS "         inc: ${MYSQLCPPCONN_INCLUDE_DIR}")
  else()
    if (VIX_ORM_REQUIRE_MYSQL)
      message(FATAL_ERROR "[vix_orm] mysql requested but Connector/C++ not found (VIX_ORM_REQUIRE_MYSQL=ON).")
    endif()
    message(STATUS "[vix_orm] mysql: disabled (connector not found)")
    set(VIX_ORM_USE_MYSQL OFF)
  endif()
endif()

# SQLite detection (soft)
set(VIX_ORM_HAS_SQLITE OFF)
set(_VIX_SQLITE_TARGET "")

if (VIX_ORM_USE_SQLITE)
  find_package(SQLite3 QUIET)

  if (TARGET SQLite::SQLite3)
    set(_VIX_SQLITE_TARGET SQLite::SQLite3)
  elseif (TARGET SQLite3::SQLite3)
    add_library(SQLite::SQLite3 ALIAS SQLite3::SQLite3)
    set(_VIX_SQLITE_TARGET SQLite::SQLite3)
  endif()

  if (SQLite3_FOUND OR _VIX_SQLITE_TARGET)
    set(VIX_ORM_HAS_SQLITE ON)
    message(STATUS "[vix_orm] sqlite: enabled")
  else()
    message(WARNING "[vix_orm] sqlite requested but not found; disabled.")
    set(VIX_ORM_USE_SQLITE OFF)
  endif()
endif()

# sources / headers
set(VIX_ORM_PUBLIC_HEADERS
  include/vix/orm/Errors.hpp
  include/vix/orm/Drivers.hpp
  include/vix/orm/ConnectionPool.hpp
  include/vix/orm/Transaction.hpp
  include/vix/orm/QueryBuilder.hpp
  include/vix/orm/Entity.hpp
  include/vix/orm/Mapper.hpp
  include/vix/orm/Repository.hpp
  include/vix/orm/UnitOfWork.hpp
  include/vix/orm/Migration.hpp
  include/vix/orm/MigrationsRunner.hpp
  include/vix/orm/Database.hpp
  include/vix/orm/orm.hpp
  include/vix/orm/FileMigrationsRunner.hpp
  include/vix/orm/Sha256.hpp
)

set(VIX_ORM_SOURCES
  src/ConnectionPool.cpp
  src/Mapper.cpp
  src/MigrationsRunner.cpp
  src/QueryBuilder.cpp
  src/Repository.cpp
  src/Transaction.cpp
  src/Database.cpp
  src/FileMigrationsRunner.cpp
  src/Sha256.cpp
)

if (VIX_ORM_HAS_MYSQL)
  list(APPEND VIX_ORM_PUBLIC_HEADERS include/vix/orm/MySQLDriver.hpp)
  list(APPEND VIX_ORM_SOURCES       src/MySQLDriver.cpp)
endif()

if (VIX_ORM_HAS_SQLITE)
  list(APPEND VIX_ORM_PUBLIC_HEADERS include/vix/orm/SqliteDriver.hpp)
  list(APPEND VIX_ORM_SOURCES       src/SqliteDriver.cpp)
endif()

# If building ORM standalone (not from umbrella), bring core as subdir
if (NOT TARGET vix::core)
  add_subdirectory(../core core_build)
endif()

# library target
add_library(vix_orm STATIC ${VIX_ORM_SOURCES} ${VIX_ORM_PUBLIC_HEADERS})
add_library(vix::orm ALIAS vix_orm)

# tools (migrator)  NOT an example
if (VIX_ORM_BUILD_TOOLS)
  add_executable(vix_orm_migrator)

  target_sources(vix_orm_migrator PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/migrator/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/migrator/MigratorCLI.cpp
  )

  target_link_libraries(vix_orm_migrator PRIVATE vix::orm)
  target_compile_features(vix_orm_migrator PRIVATE cxx_std_20)

  if (MSVC)
    target_compile_options(vix_orm_migrator PRIVATE ${_WARNINGS_MSVC})
  else()
    target_compile_options(vix_orm_migrator PRIVATE ${_WARNINGS_GNU})
  endif()

  install(TARGETS vix_orm_migrator
    RUNTIME DESTINATION ${CMAKE_INSTALL_LIBEXECDIR}/vix
  )
endif()


# Keep feature macros consistent everywhere (build + consumers)
target_compile_definitions(vix_orm PUBLIC
  VIX_ORM_HAS_MYSQL=$<BOOL:${VIX_ORM_HAS_MYSQL}>
  VIX_ORM_HAS_SQLITE=$<BOOL:${VIX_ORM_HAS_SQLITE}>
)

# export name => installed imported target is vix::orm
set_target_properties(vix_orm PROPERTIES EXPORT_NAME orm)

target_include_directories(vix_orm PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
# ORM depends on Vix Core (Config.hpp, http, json, etc.)
target_link_libraries(vix_orm PUBLIC vix::core)
# Inherit umbrella sanitizers if present
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
  # accept either target name
  if (TARGET spdlog::spdlog_header_only)
    target_link_libraries(vix_orm PUBLIC spdlog::spdlog_header_only)
  elseif (TARGET spdlog::spdlog)
    target_link_libraries(vix_orm PUBLIC spdlog::spdlog)
  endif()
  target_compile_definitions(vix_orm PUBLIC VIX_ORM_HAS_SPDLOG=1)
else()
  target_compile_definitions(vix_orm PUBLIC VIX_ORM_NO_LOGGER=1)
endif()

# MySQL linkage (THIS FIXES YOUR LINK ERROR)
if (VIX_ORM_HAS_MYSQL)
  target_include_directories(vix_orm PUBLIC "${MYSQLCPPCONN_INCLUDE_DIR}")
  target_link_libraries(vix_orm PUBLIC "${MYSQLCPPCONN_LIB}")
endif()

# SQLite linkage
if (VIX_ORM_HAS_SQLITE)
  if (_VIX_SQLITE_TARGET)
    target_link_libraries(vix_orm PUBLIC ${_VIX_SQLITE_TARGET})
  else()
    message(WARNING "[vix_orm] sqlite enabled but no target found; linkage skipped.")
  endif()
endif()

# examples
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

  vix_add_orm_example(batch_insert_tx
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/batch_insert_tx.cpp)
  vix_add_orm_example(vix_orm_users
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/users_crud.cpp)
  vix_add_orm_example(querybuilder_update
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/querybuilder_update.cpp)
  vix_add_orm_example(repository_crud_full
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/repository_crud_full.cpp)
  vix_add_orm_example(tx_unit_of_work
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/tx_unit_of_work.cpp)
  vix_add_orm_example(error_handling
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/error_handling.cpp)

endif()

# install / export via umbrella export-set "VixTargets"
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
message(STATUS "[vix_orm] mysql:  requested=${VIX_ORM_USE_MYSQL} available=${VIX_ORM_HAS_MYSQL}")
message(STATUS "[vix_orm] sqlite: requested=${VIX_ORM_USE_SQLITE} available=${VIX_ORM_HAS_SQLITE}")
message(STATUS "------------------------------------------------------")
