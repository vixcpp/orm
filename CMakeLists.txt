cmake_minimum_required(VERSION 3.20)

project(vix_orm VERSION 1.2.1 LANGUAGES CXX)

include(GNUInstallDirs)

# ---------------- options ----------------
option(VIX_ORM_BUILD_TESTS       "Build unit tests for Vix ORM"               OFF)
option(VIX_ORM_BUILD_EXAMPLES    "Build examples for Vix ORM"                ON)

option(VIX_ORM_USE_MYSQL         "Enable MySQL Connector/C++ driver"         ON)
option(VIX_ORM_REQUIRE_MYSQL     "Fail if MySQL requested but not found"     OFF)

option(VIX_ORM_USE_SQLITE        "Enable SQLite3 driver"                     OFF)
option(VIX_ORM_ENABLE_SANITIZERS "Enable sanitizers in Debug builds"         ON)

# ---------------- language / PIC ----------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ---------------- warnings / sanitizers ----------------
set(_WARNINGS_GNU
  -Wall -Wextra -Wshadow -Wconversion
  -Wnon-virtual-dtor -Wold-style-cast -Woverloaded-virtual -Wpedantic
)
set(_WARNINGS_MSVC /W4 /permissive-)
set(_SAN_DBG_GNU -fsanitize=address,undefined)

# ---------------- optional deps ----------------
find_package(spdlog QUIET CONFIG)

# ============================================================
# MySQL detection (robust) — enable only if lib+headers found
# ============================================================
set(VIX_ORM_HAS_MYSQL OFF)
set(MYSQLCPPCONN_LIB "")
set(MYSQLCPPCONN_INCLUDE_DIR "")

if (VIX_ORM_USE_MYSQL)
  # 1) Try common Ubuntu paths directly (since the system lib exists)
  if (EXISTS "/usr/lib/x86_64-linux-gnu/libmysqlcppconn.so")
    set(MYSQLCPPCONN_LIB "/usr/lib/x86_64-linux-gnu/libmysqlcppconn.so")
  elseif (EXISTS "/usr/lib/x86_64-linux-gnu/libmysqlcppconn.so.7")
    set(MYSQLCPPCONN_LIB "/usr/lib/x86_64-linux-gnu/libmysqlcppconn.so.7")
  endif()

  if (EXISTS "/usr/include/cppconn/connection.h")
    set(MYSQLCPPCONN_INCLUDE_DIR "/usr/include")
  endif()

  # 2) Fallback search if not found
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

  # 3) Final decision
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

# ============================================================
# SQLite detection (soft)
# ============================================================
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

# ============================================================
# sources / headers
# ============================================================
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
)

set(VIX_ORM_SOURCES
  src/ConnectionPool.cpp
  src/Mapper.cpp
  src/MigrationsRunner.cpp
  src/QueryBuilder.cpp
  src/Repository.cpp
  src/Transaction.cpp
  src/Database.cpp
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

# ============================================================
# library target
# ============================================================
add_library(vix_orm STATIC ${VIX_ORM_SOURCES} ${VIX_ORM_PUBLIC_HEADERS})
add_library(vix::orm ALIAS vix_orm)

# Keep feature macros consistent everywhere (build + consumers)
# (define them ONCE; do NOT redefine them again later)
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
# ✅ ORM depends on Vix Core (Config.hpp, http, json, etc.)
target_link_libraries(vix_orm PUBLIC vix::core)
if (MSVC)
  target_compile_options(vix_orm PRIVATE ${_WARNINGS_MSVC})
else()
  target_compile_options(vix_orm PRIVATE ${_WARNINGS_GNU})
endif()

# sanitizers (Debug only)
if (CMAKE_BUILD_TYPE MATCHES "Debug" AND VIX_ORM_ENABLE_SANITIZERS AND NOT MSVC)
  target_compile_options(vix_orm PRIVATE ${_SAN_DBG_GNU})
  target_link_options(vix_orm PRIVATE ${_SAN_DBG_GNU})
endif()

# ============================================================
# deps / defines
# ============================================================
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

# ============================================================
# examples
# ============================================================
if (VIX_ORM_BUILD_EXAMPLES)
  add_executable(vix_orm_users examples/users_crud.cpp)
  target_link_libraries(vix_orm_users PRIVATE vix::orm)

  if (MSVC)
    target_compile_options(vix_orm_users PRIVATE ${_WARNINGS_MSVC})
  else()
    target_compile_options(vix_orm_users PRIVATE ${_WARNINGS_GNU})
  endif()
endif()

# ============================================================
# install / export via umbrella export-set "VixTargets"
# ============================================================
install(TARGETS vix_orm
  EXPORT  VixTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h"
)

message(STATUS "------------------------------------------------------")
message(STATUS "[vix_orm] configured (${PROJECT_VERSION})")
message(STATUS "[vix_orm] mysql:  requested=${VIX_ORM_USE_MYSQL} available=${VIX_ORM_HAS_MYSQL}")
message(STATUS "[vix_orm] sqlite: requested=${VIX_ORM_USE_SQLITE} available=${VIX_ORM_HAS_SQLITE}")
message(STATUS "------------------------------------------------------")
