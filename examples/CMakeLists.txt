cmake_minimum_required(VERSION 3.16)

set(ORM_EXAMPLE_SOURCES
  01_basic_repository.cpp
  02_repository_crud.cpp
  03_find_all_and_count.cpp
  04_exists_and_delete.cpp
  05_unit_of_work.cpp
  06_batch_insert_tx.cpp
  07_query_builder_select.cpp
  08_query_builder_update.cpp
  09_error_handling.cpp
  10_custom_repository.cpp
  11_sqlite_repository.cpp
  12_mysql_repository.cpp
  13_migrations_code.cpp
  14_migrations_files.cpp
  15_entity_identity.cpp
  16_manual_sql_with_orm.cpp
)

function(vix_orm_add_example source_file)
  get_filename_component(example_name "${source_file}" NAME_WE)

  add_executable(${example_name} ${source_file})

  target_compile_features(${example_name} PRIVATE cxx_std_20)

  target_include_directories(${example_name}
    PRIVATE
      ${PROJECT_SOURCE_DIR}/include
  )

  if(TARGET vix::orm)
    target_link_libraries(${example_name} PRIVATE vix::orm)
  elseif(TARGET orm)
    target_link_libraries(${example_name} PRIVATE orm)
  endif()

  if(TARGET vix::db)
    target_link_libraries(${example_name} PRIVATE vix::db)
  elseif(TARGET db)
    target_link_libraries(${example_name} PRIVATE db)
  endif()

  if(TARGET SQLite::SQLite3)
    target_link_libraries(${example_name} PRIVATE SQLite::SQLite3)
  endif()

  if(TARGET mysql::mysql)
    target_link_libraries(${example_name} PRIVATE mysql::mysql)
  elseif(TARGET mysqlcppconn)
    target_link_libraries(${example_name} PRIVATE mysqlcppconn)
  endif()

  set_target_properties(${example_name}
    PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples
  )
endfunction()

foreach(example_src IN LISTS ORM_EXAMPLE_SOURCES)
  vix_orm_add_example(${example_src})
endforeach()

# Copy migration files next to executables for file-based migration examples
foreach(migration_example IN ITEMS 13_migrations_code 14_migrations_files)
  if(TARGET ${migration_example})
    add_custom_command(TARGET ${migration_example} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_directory
              ${CMAKE_CURRENT_SOURCE_DIR}/migrations
              $<TARGET_FILE_DIR:${migration_example}>/migrations
    )
  endif()
endforeach()
