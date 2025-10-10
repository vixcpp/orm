cmake_minimum_required(VERSION 3.20)

# ======================================================================
#  Vix ORM — CMakeLists.txt (professionnel, documenté)
# ======================================================================
# Objet
#   - Construire la lib vix_orm (C++20, PIC), avec liaisons optionnelles
#     vers MySQL (Connector/C++) et SQLite3, et exemples.
#
# Points clés
#   - Le projet doit rester buildable SANS MySQL/SQLite (détection douce).
#   - MySQL : on essaie d’abord un package officiel CMake, puis un alias
#     fallback (MySQLCppConnAlias.cmake) qui cherche les chemins usuels.
#   - RPATH : on utilise $ORIGIN pour exécuter les binaires hors install.
#   - CI : on peut exiger MySQL (fail-fast) via VIX_ORM_REQUIRE_MYSQL=ON.
#
# Exemples d’invocation
#   - Build release + exemples (auto-détection MySQL/SQLite) :
#       cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DVIX_ORM_BUILD_EXAMPLES=ON
#       cmake --build build -j
#
#   - Forcer MySQL legacy 1.x si CMake ne le trouve pas seul :
#       cmake -S . -B build \
#         -DVIX_ORM_USE_MYSQL=ON \
#         -DMYSQLCPPCONN_LIB=/usr/lib/x86_64-linux-gnu/libmysqlcppconn.so \
#         -DMYSQLCPPCONN_INCLUDE_DIR=/usr/include
#
#   - CI stricte : échouer si MySQL est requis mais introuvable :
#       cmake -S . -B build -DVIX_ORM_USE_MYSQL=ON -DVIX_ORM_REQUIRE_MYSQL=ON
#
# Notes MySQL
#   - Legacy 1.x (Ubuntu/Debian) : libmysqlcppconn.so + headers cppconn/
#     Paquet : libmysqlcppconn-dev
#   - 8.x (Oracle) : libmysqlcppconn8.so + headers jdbc/
#     Paquet : libmysqlcppconn8-dev (ou tarball officiel)
#
# Maintenance
#   - Ajouter de nouvelles cibles/exemples sous VIX_ORM_BUILD_EXAMPLES.
#   - Les macros/logs doivent rester clairs : pas de générateurs $<BOOL:…>
#     dans les messages utilisateur.
# ======================================================================

project(vix_orm VERSION 0.1.0 LANGUAGES CXX)

# ---------------- Options (activables via -D…) ----------------
option(VIX_ORM_BUILD_TESTS       "Build unit tests for Vix ORM"              ON)
option(VIX_ORM_BUILD_EXAMPLES    "Build examples for Vix ORM"                ON)
option(VIX_ORM_USE_MYSQL         "Enable MySQL Connector/C++ driver"         ON)
option(VIX_ORM_USE_SQLITE        "Enable SQLite3 driver"                     OFF)
option(VIX_ORM_ENABLE_SANITIZERS "Enable sanitizers in Debug builds"         ON)

# ---------------- Langage / PIC (C++20, lib position indépendante) ----
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Type de build par défaut (modifiable : Debug/RelWithDebInfo/…)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()
message(STATUS "[vix_orm] Build type: ${CMAKE_BUILD_TYPE}")

# ---------------- RPATH (global) ---------------------------------------
# $ORIGIN permet d’exécuter les binaires depuis le dossier build/install
# sans devoir exporter LD_LIBRARY_PATH, tant que les libs sont à proximité.
set(CMAKE_BUILD_RPATH_USE_ORIGIN ON)
set(CMAKE_INSTALL_RPATH_USE_ORIGIN ON)

# ---------------- Warnings / Sanitizers --------------------------------
set(_WARNINGS_GNU "-Wall;-Wextra;-Wshadow;-Wconversion;-Wnon-virtual-dtor;-Wold-style-cast;-Woverloaded-virtual;-Wpedantic")
set(_WARNINGS_MSVC "/W4;/permissive-")
set(_SAN_DBG_GNU "-fsanitize=address,undefined")
set(_SAN_DBG_MSVC "") # MSVC : vide par défaut (ASan/UBSan gérés différemment)

# ---------------- Dépendances optionnelles ------------------------------
# spdlog est optionnel : si absent, on compile quand même (NO_LOGGER).
find_package(spdlog QUIET)

# --- MySQL (single-pass, guarded) --------------------------------------
# Stratégie :
#  1) find_package(MySQLCppConn) si un package officiel est dispo.
#  2) sinon, fallback via MySQLCppConnAlias.cmake (détection chemins communs).
if(VIX_ORM_USE_MYSQL)
  find_package(MySQLCppConn QUIET)
  if(NOT MySQLCppConn_FOUND AND NOT TARGET MySQLCppConn::MySQLCppConn)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
    include(MySQLCppConnAlias OPTIONAL RESULT_VARIABLE _MYSQL_ALIAS_INCLUDED)
  endif()
endif()

# --- Fail-fast optionnel si MySQL requis (CI stricte) ------------------
# Activez -DVIX_ORM_REQUIRE_MYSQL=ON si vous voulez faire échouer CMake
# quand MySQL est activé mais introuvable.
option(VIX_ORM_REQUIRE_MYSQL "Fail if MySQL was requested but not found" OFF)
if (VIX_ORM_USE_MYSQL AND VIX_ORM_REQUIRE_MYSQL)
  if (NOT (MySQLCppConn_FOUND OR TARGET MySQLCppConn::MySQLCppConn))
    message(FATAL_ERROR "[vix_orm] MySQL is required but MySQLCppConn was not found.")
  endif()
endif()

# --- SQLite (optionnel) ------------------------------------------------
if(VIX_ORM_USE_SQLITE)
  find_package(SQLite3 QUIET)
  if(NOT SQLite3_FOUND)
    message(WARNING "[vix_orm] SQLite3 not found; disable VIX_ORM_USE_SQLITE or install SQLite3 dev files.")
  endif()
endif()

# ---------------- Sources publiques/privées -----------------------------
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

# ---------------- Bibliothèque -----------------------------------------
add_library(vix_orm ${VIX_ORM_SOURCES} ${VIX_ORM_PUBLIC_HEADERS})
add_library(Vix::orm ALIAS vix_orm)

# Chemins d’include exposés aux consommateurs (headers dans include/)
target_include_directories(vix_orm PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

# Warnings cohérents
if(MSVC)
  target_compile_options(vix_orm PRIVATE ${_WARNINGS_MSVC})
else()
  target_compile_options(vix_orm PRIVATE ${_WARNINGS_GNU})
endif()

# Sanitizers en Debug (Linux/Clang/GCC)
if(CMAKE_BUILD_TYPE MATCHES "Debug" AND VIX_ORM_ENABLE_SANITIZERS)
  if(MSVC)
    # pas d’ASan/UBSan ici par défaut
  else()
    target_compile_options(vix_orm PRIVATE ${_SAN_DBG_GNU})
    target_link_options(vix_orm PRIVATE ${_SAN_DBG_GNU})
  endif()
endif()

# --- Liaisons optionnelles ---------------------------------------------
# spdlog (header-only si trouvé, sinon NO_LOGGER)
if(spdlog_FOUND)
  target_link_libraries(vix_orm PUBLIC spdlog::spdlog_header_only)
  target_compile_definitions(vix_orm PUBLIC VIX_ORM_HAS_SPDLOG=1)
else()
  target_compile_definitions(vix_orm PUBLIC VIX_ORM_NO_LOGGER=1)
endif()

# MySQL : on ne linke que si la cible importée existe
if(VIX_ORM_USE_MYSQL)
  if(TARGET MySQLCppConn::MySQLCppConn)
    target_link_libraries(vix_orm PUBLIC MySQLCppConn::MySQLCppConn)
    target_compile_definitions(vix_orm PUBLIC VIX_ORM_HAS_MYSQL=1)
  else()
    message(WARNING "[vix_orm] MySQL enabled but MySQLCppConn target not found; building without MySQL linkage.")
  endif()
endif()

# SQLite
if(VIX_ORM_USE_SQLITE AND SQLite3_FOUND)
  target_link_libraries(vix_orm PUBLIC SQLite::SQLite3)
  target_compile_definitions(vix_orm PUBLIC VIX_ORM_HAS_SQLITE=1)
endif()

# ---------------- Exemples ---------------------------------------------
# Astuce : ajoutez d’autres exemples sous examples/*.cpp et linkez Vix::orm
if(VIX_ORM_BUILD_EXAMPLES)
  add_executable(vix_orm_users examples/users_crud.cpp)
  target_link_libraries(vix_orm_users PRIVATE Vix::orm)
  if(MSVC)
    target_compile_options(vix_orm_users PRIVATE ${_WARNINGS_MSVC})
  else()
    target_compile_options(vix_orm_users PRIVATE ${_WARNINGS_GNU})
  endif()
endif()

# ---------------- Install / Package Config -----------------------------
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

# ---------------- Résumé (lisible) -------------------------------------
if (MySQLCppConn_FOUND OR TARGET MySQLCppConn::MySQLCppConn)
  set(_mysql_found_txt "1")
else()
  set(_mysql_found_txt "0")
endif()
message(STATUS "[vix_orm] MySQL:  ${VIX_ORM_USE_MYSQL} (target: ${_mysql_found_txt})")
message(STATUS "[vix_orm] SQLite: ${VIX_ORM_USE_SQLITE} (found: ${SQLite3_FOUND})")
message(STATUS "[vix_orm] spdlog: ${spdlog_FOUND}")
