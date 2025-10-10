# Try common library names and include locations
find_library(MYSQLCPPCONN_LIB
  NAMES mysqlcppconn8 mysqlcppconn
  HINTS ${MYSQLCPPCONN_ROOT}/lib ${MYSQLCPPCONN_ROOT}/lib64
        /usr/lib /usr/lib64 /usr/local/lib /usr/local/lib64
)

find_path(MYSQLCPPCONN_INCLUDE_DIR
  NAMES cppconn/connection.h
  HINTS ${MYSQLCPPCONN_ROOT}/include /usr/include /usr/local/include
)

if (NOT MYSQLCPPCONN_LIB OR NOT MYSQLCPPCONN_INCLUDE_DIR)
  message(FATAL_ERROR "[vix_orm] Could not locate MySQL Connector/C++ (headers or library). Set MYSQLCPPCONN_ROOT or install it.")
endif()

add_library(MySQLCppConn::MySQLCppConn UNKNOWN IMPORTED)
set_target_properties(MySQLCppConn::MySQLCppConn PROPERTIES
  IMPORTED_LOCATION "${MYSQLCPPCONN_LIB}"
  INTERFACE_INCLUDE_DIRECTORIES "${MYSQLCPPCONN_INCLUDE_DIR}"
)
