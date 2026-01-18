# ------------------------------------------------------------------------------
# MySQLCppConnAlias.cmake — Imported target fallback for MySQL Connector/C++
#
# This script defines a fallback imported target (MySQLCppConn::MySQLCppConn)
# in case `find_package(MySQLCppConn)` fails or is unavailable.
#
# Supports both legacy (1.x) and modern (8.x) versions of the MySQL C++ connector.
# This is useful in CI, custom installations, or environments without proper config files.
# ------------------------------------------------------------------------------

if (TARGET MySQLCppConn::MySQLCppConn)
  return()
endif()

set(MYSQLCPPCONN_ROOT "" CACHE PATH "Root path to MySQL Connector/C++")
set(MYSQLCPPCONN_LIB "" CACHE FILEPATH "Full path to libmysqlcppconn{8}.so/.a")
set(MYSQLCPPCONN_INCLUDE_DIR "" CACHE PATH "Include directory containing cppconn/ or jdbc/")

# Debian/Ubuntu: quick-path
if (NOT MYSQLCPPCONN_LIB)
  if (EXISTS "/usr/lib/x86_64-linux-gnu/libmysqlcppconn.so")
    set(MYSQLCPPCONN_LIB "/usr/lib/x86_64-linux-gnu/libmysqlcppconn.so" CACHE FILEPATH "" FORCE)
  elseif (EXISTS "/usr/lib/x86_64-linux-gnu/libmysqlcppconn.so.7")
    set(MYSQLCPPCONN_LIB "/usr/lib/x86_64-linux-gnu/libmysqlcppconn.so.7" CACHE FILEPATH "" FORCE)
  elseif (EXISTS "/usr/lib/x86_64-linux-gnu/libmysqlcppconn.so.8")
    set(MYSQLCPPCONN_LIB "/usr/lib/x86_64-linux-gnu/libmysqlcppconn.so.8" CACHE FILEPATH "" FORCE)
  endif()
endif()

if (NOT MYSQLCPPCONN_INCLUDE_DIR)
  if (EXISTS "/usr/include/cppconn/connection.h")
    set(MYSQLCPPCONN_INCLUDE_DIR "/usr/include" CACHE PATH "" FORCE)
  elseif (EXISTS "/usr/include/mysql-cppconn-8/jdbc/mysql_connection.h")
    set(MYSQLCPPCONN_INCLUDE_DIR "/usr/include/mysql-cppconn-8" CACHE PATH "" FORCE)
  endif()
endif()

# Generic search (fallback)
set(_mysql_lib_hints
  ${MYSQLCPPCONN_ROOT}/lib
  ${MYSQLCPPCONN_ROOT}/lib64
  /usr/lib /usr/lib64 /usr/local/lib /usr/local/lib64
  /usr/lib/x86_64-linux-gnu /usr/local/lib/x86_64-linux-gnu
)

set(_mysql_inc_hints
  ${MYSQLCPPCONN_ROOT}/include
  /usr/include /usr/local/include
  /usr/include/mysql-cppconn-8
)

if (NOT MYSQLCPPCONN_LIB)
  find_library(MYSQLCPPCONN_LIB
    NAMES mysqlcppconn mysqlcppconn8 mysqlcppconn-static
    HINTS ${_mysql_lib_hints}
  )
endif()

if (NOT MYSQLCPPCONN_INCLUDE_DIR)
  find_path(MYSQLCPPCONN_INCLUDE_DIR
    NAMES cppconn/connection.h
    HINTS ${_mysql_inc_hints}
  )
endif()

if (NOT MYSQLCPPCONN_INCLUDE_DIR)
  find_path(MYSQLCPPCONN_INCLUDE_DIR
    NAMES jdbc/mysql_connection.h
    HINTS ${_mysql_inc_hints}
  )
endif()

# Not found -> do not hard fail
if (NOT MYSQLCPPCONN_LIB OR NOT MYSQLCPPCONN_INCLUDE_DIR)
  message(WARNING "[vix_orm] MySQLCppConn fallback: could not locate library or headers.")
  message(WARNING "           -DMYSQLCPPCONN_LIB=/path/to/libmysqlcppconn.so")
  message(WARNING "           -DMYSQLCPPCONN_INCLUDE_DIR=/path/to/include")
  return()
endif()

# Define imported target
add_library(MySQLCppConn::MySQLCppConn UNKNOWN IMPORTED)

set_target_properties(MySQLCppConn::MySQLCppConn PROPERTIES
  IMPORTED_LOCATION "${MYSQLCPPCONN_LIB}"
  INTERFACE_INCLUDE_DIRECTORIES "${MYSQLCPPCONN_INCLUDE_DIR}"
)

set_property(TARGET MySQLCppConn::MySQLCppConn PROPERTY
  INTERFACE_LINK_LIBRARIES "${MYSQLCPPCONN_LIB}"
)

# Static library extra deps
get_filename_component(_ext "${MYSQLCPPCONN_LIB}" EXT)
if (_ext STREQUAL ".a")
  find_package(OpenSSL REQUIRED)
  find_package(ZLIB REQUIRED)
  set_property(TARGET MySQLCppConn::MySQLCppConn APPEND PROPERTY
    INTERFACE_LINK_LIBRARIES OpenSSL::SSL OpenSSL::Crypto ZLIB::ZLIB
  )
endif()

message(STATUS "[orm] MySQLCppConn alias configured:")
message(STATUS "       LIB: ${MYSQLCPPCONN_LIB}")
message(STATUS "       INC: ${MYSQLCPPCONN_INCLUDE_DIR}")
