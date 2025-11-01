cmake_minimum_required(VERSION 3.20)

# ======================================================================
# Vix ORM — Core Library (Installable, Exported via Umbrella)
# ======================================================================
# Purpose:
#   Build the vix_orm library (C++20, PIC), with optional bindings for
#   MySQL (Connector/C++) and SQLite3. Examples are optional. The module
#   integrates with the umbrella's export-set (VixTargets) so that a
#   system-wide installation of Vix exposes vix::orm via find_package(Vix).
#
# Key points:
#   - The project must remain buildable WITHOUT MySQL/SQLite (soft detection).
#   - MySQL: try an official CMake package first (MySQLCppConn); otherwise
#     include a local alias finder (cmake/MySQLCppConnAlias.cmake) if present.
#   - RPATH: handled globally by the superproject if needed.
#   - CI strict mode: -DVIX_ORM_REQUIRE_MYSQL=ON will fail if MySQL is missing.
#
# Usage examples:
#   - Release + examples (auto-detect MySQL/SQLite):
#       cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DVIX_ORM_BUILD_EXAMPLES=ON
#       cmake --build build -j
#
#   - Force legacy MySQL 1.x if auto-detection fails:
#       cmake -S . -B build \
#         -DVIX_ORM_USE_MYSQL=ON \
#         -DMYSQLCPPCONN_LIB=/usr/lib/x86_64-linux-gnu/libmysqlcppconn.so \
#         -DMYSQLCPPCONN_INCLUDE_DIR=/usr/include
#
#   - CI strict: fail if MySQL is requested but not found:
#       cmake -S . -B build -DVIX_ORM_USE_MYSQL=ON -DVIX_ORM_REQUIRE_MYSQL=ON
#
# Installation/Export:
#   - Installs the library and public headers.
#   - Contributes to the umbrella export-set `VixTargets`:
#       install(TARGETS vix_orm EXPORT VixTargets ...)
#   - Consumers use:
#       find_package(Vix CONFIG REQUIRED)
#       target_link_libraries(app PRIVATE vix::orm)
# ======================================================================

project(vix_orm VERSION 1.1.1 LANGUAGES CXX)

include(GNUInstallDirs)

# ---------------- Options ----------------
option(VIX_ORM_BUILD_TESTS       "Build unit tests for Vix ORM"              OFF)
option(VIX_ORM_BUILD_EXAMPLES    "Build examples for Vix ORM"                ON)
option(VIX_ORM_USE_MYSQL         "Enable MySQL Connector/C++ driver"         ON)
option(VIX_ORM_USE_SQLITE        "Enable SQLite3 driver"                     OFF)
option(VIX_ORM_ENABLE_SANITIZERS "Enable sanitizers in Debug builds"         ON)
option(VIX_ORM_REQUIRE_MYSQL     "Fail if MySQL was requested but not found" OFF)

# ---------------- Language / PIC ----------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ---------------- Warnings / Sanitizers --------
set(_WARNINGS_GNU  -Wall -Wextra -Wshadow -Wconversion -Wnon-virtual-dtor -Wold-style-cast -Woverloaded-virtual -Wpedantic)
set(_WARNINGS_MSVC /W4 /permissive-)

# Sanitizers (Debug only, GCC/Clang)
set(_SAN_DBG_GNU -fsanitize=address,undefined)

# ---------------- Optional deps ----------------
# spdlog is optional; build without logging if not found.
find_package(spdlog QUIET)

# --- MySQL (soft detection) ---
if(VIX_ORM_USE_MYSQL)
  find_package(MySQLCppConn QUIET)
  if(NOT MySQLCppConn_FOUND AND NOT TARGET MySQLCppConn::MySQLCppConn)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
    include(MySQLCppConnAlias OPTIONAL RESULT_VARIABLE _MYSQL_ALIAS_INCLUDED)
  endif()
endif()

# Fail-fast if requested
if (VIX_ORM_USE_MYSQL AND VIX_ORM_REQUIRE_MYSQL)
  if (NOT (MySQLCppConn_FOUND OR TARGET MySQLCppConn::MySQLCppConn))
    message(FATAL_ERROR "[vix_orm] MySQL is required but MySQLCppConn was not found.")
  endif()
endif()

# --- SQLite (soft detection) ---
if(VIX_ORM_USE_SQLITE)
  find_package(SQLite3 QUIET)
  if(NOT SQLite3_FOUND)
    message(WARNING "[vix_orm] SQLite3 not found; disable VIX_ORM_USE_SQLITE or install SQLite3 dev files.")
  endif()
endif()

# ---------------- Sources / Headers ---------
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

if(VIX_ORM_USE_SQLITE AND SQLite3_FOUND)
  list(APPEND VIX_ORM_PUBLIC_HEADERS include/vix/orm/SQLiteDriver.hpp)
  list(APPEND VIX_ORM_SOURCES       src/SQLiteDriver.cpp)
endif()

# ---------------- Library target -----------
add_library(vix_orm ${VIX_ORM_SOURCES} ${VIX_ORM_PUBLIC_HEADERS})
add_library(vix::orm ALIAS vix_orm)

target_include_directories(vix_orm PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if(MSVC)
  target_compile_options(vix_orm PRIVATE ${_WARNINGS_MSVC})
else()
  target_compile_options(vix_orm PRIVATE ${_WARNINGS_GNU})
endif()

# Optional sanitizers in Debug (GCC/Clang)
if(CMAKE_BUILD_TYPE MATCHES "Debug" AND VIX_ORM_ENABLE_SANITIZERS)
  if(NOT MSVC)
    target_compile_options(vix_orm PRIVATE ${_SAN_DBG_GNU})
    target_link_options(vix_orm PRIVATE ${_SAN_DBG_GNU})
  endif()
endif()

# ---- Optional linkages ----
if(spdlog_FOUND)
  target_link_libraries(vix_orm PUBLIC spdlog::spdlog_header_only)
  target_compile_definitions(vix_orm PUBLIC VIX_ORM_HAS_SPDLOG=1)
else()
  target_compile_definitions(vix_orm PUBLIC VIX_ORM_NO_LOGGER=1)
endif()

# MySQL (only if target exists)
if(VIX_ORM_USE_MYSQL)
  if(TARGET MySQLCppConn::MySQLCppConn)
    target_link_libraries(vix_orm PUBLIC MySQLCppConn::MySQLCppConn)
    target_compile_definitions(vix_orm PUBLIC VIX_ORM_HAS_MYSQL=1)
  else()
    message(WARNING "[vix_orm] MySQL enabled but MySQLCppConn target not found; building without MySQL linkage.")
  endif()
endif()

# SQLite (if found)
if(VIX_ORM_USE_SQLITE AND SQLite3_FOUND)
  target_link_libraries(vix_orm PUBLIC SQLite::SQLite3)
  target_compile_definitions(vix_orm PUBLIC VIX_ORM_HAS_SQLITE=1)
endif()

# ---------------- Examples -----------------
if(VIX_ORM_BUILD_EXAMPLES)
  add_executable(vix_orm_users examples/users_crud.cpp)
  target_link_libraries(vix_orm_users PRIVATE vix::orm)
  if(MSVC)
    target_compile_options(vix_orm_users PRIVATE ${_WARNINGS_MSVC})
  else()
    target_compile_options(vix_orm_users PRIVATE ${_WARNINGS_GNU})
  endif()
endif()

# ---------------- Install / Export (umbrella) --------------
install(TARGETS vix_orm
  EXPORT  VixTargets           # <- join umbrella export-set
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h")

# ---------------- Summary (readable) ------------------------
if (MySQLCppConn_FOUND OR TARGET MySQLCppConn::MySQLCppConn)
  set(_mysql_found_txt "1")
else()
  set(_mysql_found_txt "0")
endif()

message(STATUS "------------------------------------------------------")
message(STATUS "[vix_orm] configured (${PROJECT_VERSION})")
message(STATUS "[vix_orm] MySQL:  enabled=${VIX_ORM_USE_MYSQL} (target=${_mysql_found_txt})")
message(STATUS "[vix_orm] SQLite: enabled=${VIX_ORM_USE_SQLITE} (found=${SQLite3_FOUND})")
message(STATUS "[vix_orm] spdlog: found=${spdlog_FOUND}")
message(STATUS "Install include dir: ${CMAKE_INSTALL_INCLUDEDIR}")
message(STATUS "Install lib dir    : ${CMAKE_INSTALL_LIBDIR}")
message(STATUS "------------------------------------------------------")
