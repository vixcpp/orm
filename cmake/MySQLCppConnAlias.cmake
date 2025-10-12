# ------------------------------------------------------------------------------
# MySQLCppConnAlias.cmake — Imported target fallback for MySQL Connector/C++
#
# This script defines a fallback imported target (MySQLCppConn::MySQLCppConn)
# in case `find_package(MySQLCppConn)` fails or is unavailable.
#
# Supports both legacy (1.x) and modern (8.x) versions of the MySQL C++ connector.
# This is useful in CI, custom installations, or environments without proper config files.
# ------------------------------------------------------------------------------

# Abort if the imported target is already defined (e.g., by find_package)
if (TARGET MySQLCppConn::MySQLCppConn)
  return()
endif()

# ------------------------------------------------------------------------------
# User-configurable variables (can be overridden via -D flags)
# ------------------------------------------------------------------------------

set(MYSQLCPPCONN_ROOT "" CACHE PATH "Root path to MySQL Connector/C++ (must contain include/ and lib/)")
set(MYSQLCPPCONN_LIB "" CACHE FILEPATH "Full path to libmysqlcppconn{8}.so or .a")
set(MYSQLCPPCONN_INCLUDE_DIR "" CACHE PATH "Include directory containing either cppconn/ or jdbc/")

# ------------------------------------------------------------------------------
# Step 0: Quick path for common Debian/Ubuntu installations
# ------------------------------------------------------------------------------

if (NOT MYSQLCPPCONN_LIB AND EXISTS "/usr/lib/x86_64-linux-gnu/libmysqlcppconn.so")
  set(MYSQLCPPCONN_LIB "/usr/lib/x86_64-linux-gnu/libmysqlcppconn.so" CACHE FILEPATH "" FORCE)
endif()

if (NOT MYSQLCPPCONN_INCLUDE_DIR AND EXISTS "/usr/include/cppconn/connection.h")
  set(MYSQLCPPCONN_INCLUDE_DIR "/usr/include" CACHE PATH "" FORCE)
endif()

# ------------------------------------------------------------------------------
# Step 1–3: Attempt to locate the connector manually (library and headers)
# ------------------------------------------------------------------------------

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

# Step 1: Try to locate the library if not already set
if (NOT MYSQLCPPCONN_LIB)
  find_library(MYSQLCPPCONN_LIB
    NAMES mysqlcppconn mysqlcppconn8
    HINTS ${_mysql_lib_hints}
  )
endif()

# Step 2: Look for legacy (1.x) headers
if (NOT MYSQLCPPCONN_INCLUDE_DIR)
  find_path(MYSQLCPPCONN_INCLUDE_DIR
    NAMES cppconn/connection.h
    HINTS ${_mysql_inc_hints}
  )
endif()

# Step 3: Look for modern (8.x) headers if legacy not found
if (NOT MYSQLCPPCONN_INCLUDE_DIR)
  find_path(MYSQLCPPCONN_INCLUDE_DIR
    NAMES jdbc/mysql_connection.h
    HINTS ${_mysql_inc_hints}
  )
endif()

# ------------------------------------------------------------------------------
# Step 4: If not found, do not block the configuration process
# ------------------------------------------------------------------------------

if (NOT MYSQLCPPCONN_LIB OR NOT MYSQLCPPCONN_INCLUDE_DIR)
  message(WARNING "[vix_orm] MySQLCppConn fallback: could not locate library or headers.")
  message(WARNING "           You can set them manually using:")
  message(WARNING "           -DMYSQLCPPCONN_LIB=/path/to/libmysqlcppconn.so")
  message(WARNING "           -DMYSQLCPPCONN_INCLUDE_DIR=/path/to/include")
  return()
endif()

# ------------------------------------------------------------------------------
# Step 5: Define the imported target manually
# ------------------------------------------------------------------------------

if (NOT TARGET MySQLCppConn::MySQLCppConn)
  add_library(MySQLCppConn::MySQLCppConn UNKNOWN IMPORTED)
endif()

set_target_properties(MySQLCppConn::MySQLCppConn PROPERTIES
  IMPORTED_LOCATION "${MYSQLCPPCONN_LIB}"
  INTERFACE_INCLUDE_DIRECTORIES "${MYSQLCPPCONN_INCLUDE_DIR}"
  IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
)

# ------------------------------------------------------------------------------
# Step 6: If using a static library (.a), link common dependencies
# ------------------------------------------------------------------------------

get_filename_component(_ext "${MYSQLCPPCONN_LIB}" EXT)
if (_ext STREQUAL ".a")
  find_package(OpenSSL REQUIRED)
  find_package(ZLIB REQUIRED)
  set_property(TARGET MySQLCppConn::MySQLCppConn APPEND PROPERTY
    INTERFACE_LINK_LIBRARIES OpenSSL::SSL OpenSSL::Crypto ZLIB::ZLIB
  )
endif()

# ------------------------------------------------------------------------------
# Done: Output the configuration summary
# ------------------------------------------------------------------------------

message(STATUS "[orm] MySQLCppConn alias configured:")
message(STATUS "       LIB: ${MYSQLCPPCONN_LIB}")
message(STATUS "       INC: ${MYSQLCPPCONN_INCLUDE_DIR}")
