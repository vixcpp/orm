cmake_minimum_required(VERSION 3.20)
project(vix_orm VERSION 0.1.0 LANGUAGES CXX)

# ======================================================
# Options
# ======================================================
option(VIX_ORM_BUILD_TESTS      "Build unit tests for Vix ORM"                ON)
option(VIX_ORM_BUILD_EXAMPLES   "Build examples for Vix ORM"                  ON)
option(VIX_ORM_USE_MYSQL        "Enable MySQL Connector/C++ driver"           ON)
option(VIX_ORM_USE_SQLITE       "Enable SQLite3 driver"                       OFF)
option(VIX_ORM_ENABLE_SANITIZERS "Enable sanitizers in Debug builds"          ON)

# ======================================================
# C++ standard & PIC
# ======================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Default build type
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()
message(STATUS "[vix_orm] Build type: ${CMAKE_BUILD_TYPE}")

# ======================================================
# Compiler warnings (target-scoped later)
# ======================================================
set(_WARNINGS_GNU "-Wall;-Wextra;-Wshadow;-Wconversion;-Wnon-virtual-dtor;-Wold-style-cast;-Woverloaded-virtual;-Wpedantic")
set(_WARNINGS_MSVC "/W4;/permissive-")
set(_SAN_DBG_GNU "-fsanitize=address,undefined")
set(_SAN_DBG_MSVC "") # ASan/UBSan on MSVC is different; keep empty by default

# ======================================================
# Dependencies (optional & graceful)
# ======================================================
# Logging (optional). If not found, we still build.
find_package(spdlog QUIET)

# MySQL (optional)
if(VIX_ORM_USE_MYSQL)
  # Aide find_package avec *_ROOT (env & cache)
  if (DEFINED ENV{MYSQLCPPCONN_ROOT} AND NOT DEFINED MYSQLCPPCONN_ROOT)
    set(MYSQLCPPCONN_ROOT "$ENV{MYSQLCPPCONN_ROOT}" CACHE PATH "MySQL Connector/C++ root" FORCE)
  endif()

  # Essayes d'abord via CMake config dans le prefix
  if (MYSQLCPPCONN_ROOT)
    list(APPEND CMAKE_PREFIX_PATH "${MYSQLCPPCONN_ROOT}")
  endif()

  find_package(MySQLCppConn QUIET)

  if(NOT MySQLCppConn_FOUND)
    # Fallback: module d'alias fourni dans modules/orm/cmake (garde ta logique)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
    include(MySQLCppConnAlias OPTIONAL RESULT_VARIABLE _MYSQL_ALIAS_INCLUDED)
    if(NOT _MYSQL_ALIAS_INCLUDED)
      message(WARNING "[vix_orm] MySQLCppConn not found; set MYSQLCPPCONN_ROOT or install the connector. MySQL driver will likely fail to link.")
    endif()
  endif()
endif()


# SQLite (optional)
if(VIX_ORM_USE_SQLITE)
  find_package(SQLite3 QUIET)
  if(NOT SQLite3_FOUND)
    message(WARNING "[vix_orm] SQLite3 not found; disable VIX_ORM_USE_SQLITE or install SQLite3 dev files.")
  endif()
endif()

# ======================================================
# Sources
# ======================================================
set(VIX_ORM_PUBLIC_HEADERS
  include/vix/orm/Errors.hpp
  include/vix/orm/Drivers.hpp
  include/vix/orm/MySQLDriver.hpp
  include/vix/orm/ConnectionPool.hpp
  include/vix/orm/Transaction.hpp
  include/vix/orm/QueryBuilder.hpp
  include/vix/orm/Entity.hpp
  include/vix/orm/Mapper.hpp
  include/vix/orm/Repository.hpp
  include/vix/orm/UnitOfWork.hpp
  include/vix/orm/Migration.hpp
  include/vix/orm/MigrationsRunner.hpp
  include/vix/orm/orm.hpp
)

set(VIX_ORM_SOURCES
  src/ConnectionPool.cpp
  src/Mapper.cpp
  src/MigrationsRunner.cpp
  src/MySQLDriver.cpp
  src/QueryBuilder.cpp
  src/Repository.cpp
  src/Transaction.cpp
)

# Optionally add SQLite driver files if you create them later
if(VIX_ORM_USE_SQLITE AND SQLite3_FOUND)
  list(APPEND VIX_ORM_PUBLIC_HEADERS
    include/vix/orm/SQLiteDriver.hpp
  )
  list(APPEND VIX_ORM_SOURCES
    src/SQLiteDriver.cpp
  )
endif()

# ======================================================
# Library
# ======================================================
add_library(vix_orm ${VIX_ORM_SOURCES} ${VIX_ORM_PUBLIC_HEADERS})
add_library(Vix::orm ALIAS vix_orm)

target_include_directories(vix_orm
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Warnings per compiler
if(MSVC)
  target_compile_options(vix_orm PRIVATE ${_WARNINGS_MSVC})
else()
  target_compile_options(vix_orm PRIVATE ${_WARNINGS_GNU})
endif()

# Sanitizers (Debug only)
if(CMAKE_BUILD_TYPE MATCHES "Debug" AND VIX_ORM_ENABLE_SANITIZERS)
  if(MSVC)
    # keep empty or add MSVC specific sanitizer flags if configured
  else()
    target_compile_options(vix_orm PRIVATE ${_SAN_DBG_GNU})
    target_link_options(vix_orm PRIVATE ${_SAN_DBG_GNU})
  endif()
endif()

# Logging
if(spdlog_FOUND)
  target_link_libraries(vix_orm PUBLIC spdlog::spdlog_header_only)
  target_compile_definitions(vix_orm PUBLIC VIX_ORM_HAS_SPDLOG=1)
else()
  target_compile_definitions(vix_orm PUBLIC VIX_ORM_NO_LOGGER=1)
endif()

# Link MySQL if enabled
if(VIX_ORM_USE_MYSQL)
  if(TARGET MySQLCppConn::MySQLCppConn)
    target_link_libraries(vix_orm PUBLIC MySQLCppConn::MySQLCppConn)
    target_compile_definitions(vix_orm PUBLIC VIX_ORM_HAS_MYSQL=1)
  else()
    message(WARNING "[vix_orm] MySQL driver requested but MySQLCppConn::MySQLCppConn target not found.")
  endif()
endif()

# Link SQLite if enabled
if(VIX_ORM_USE_SQLITE AND SQLite3_FOUND)
  target_link_libraries(vix_orm PUBLIC SQLite::SQLite3)
  target_compile_definitions(vix_orm PUBLIC VIX_ORM_HAS_SQLITE=1)
endif()

# ======================================================
# Examples (optional)
# ======================================================
if (VIX_ORM_BUILD_EXAMPLES)
  add_executable(vix_orm_users            examples/users_crud.cpp)
  target_link_libraries(vix_orm_users     PRIVATE vix_orm)

  add_executable(vix_orm_migrate_init     examples/migrate_init.cpp)
  target_link_libraries(vix_orm_migrate_init PRIVATE vix_orm)

  add_executable(vix_orm_tx_uow           examples/tx_unit_of_work.cpp)
  target_link_libraries(vix_orm_tx_uow    PRIVATE vix_orm)

  add_executable(vix_orm_repo_crud_full   examples/repository_crud_full.cpp)
  target_link_libraries(vix_orm_repo_crud_full PRIVATE vix_orm)

  add_executable(vix_orm_qb_update        examples/querybuilder_update.cpp)
  target_link_libraries(vix_orm_qb_update PRIVATE vix_orm)

  add_executable(vix_orm_batch_insert_tx  examples/batch_insert_tx.cpp)
  target_link_libraries(vix_orm_batch_insert_tx PRIVATE vix_orm)

  add_executable(vix_orm_error_handling   examples/error_handling.cpp)
  target_link_libraries(vix_orm_error_handling PRIVATE vix_orm)
endif()

# ======================================================
# Install & Package Config
# ======================================================
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS vix_orm
  EXPORT  VixOrmTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT VixOrmTargets
  NAMESPACE Vix::
  FILE     VixOrmTargets.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/VixOrm
)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/VixOrmConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/VixOrmConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/VixOrmConfig.cmake
  @ONLY
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/VixOrmConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/VixOrmConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/VixOrm
)

# Summary
message(STATUS "[vix_orm] MySQL requested: ${VIX_ORM_USE_MYSQL}; found: ${MySQLCppConn_FOUND}")
message(STATUS "[vix_orm] SQLite requested: ${VIX_ORM_USE_SQLITE}; found: ${SQLite3_FOUND}")
message(STATUS "[vix_orm] spdlog found: ${spdlog_FOUND}")

