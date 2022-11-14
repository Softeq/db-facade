# - Try to find SQLite3
# Once done, this will define
#
#  SQLite3_FOUND - system has DBus
#  SQLite3_INCLUDE_DIRS - the DBus include directories
#  SQLite3_LIBRARIES - link these to use DBus

find_package (PkgConfig)
pkg_check_modules (PC_SQLite3_CPP QUIET sqlite3)

if (PC_SQLite3_CPP_LIBRARIES)
    set (SQLite3_LIBRARIES ${PC_SQLite3_CPP_LIBRARIES})
else()
    find_library (SQLite3_LIBRARIES
        NAMES sqlite3
        HINTS ${PC_SQLite3_CPP_LIBDIR}
              ${PC_SQLite3_CPP_LIBRARY_DIRS}
    )
endif()

if (PC_SQLite3_CPP_INCLUDE_DIRS)
    set (SQLite3_INCLUDE_DIRS ${PC_SQLite3_CPP_INCLUDE_DIRS})
else()
    find_path (SQLite3_INCLUDE_DIRS
        NAMES sqlite3.h
        HINTS ${PC_SQLite3_CPP_INCLUDEDIR}
        ${PC_SQLite3_CPP_INCLUDE_DIRS}
    )
endif()

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (SQLite3 REQUIRED_VARS SQLite3_INCLUDE_DIRS SQLite3_LIBRARIES)

if (SQLite3_FOUND AND NOT TARGET SQLite3::SQLite3)
    add_library(SQLite3::SQLite3 INTERFACE IMPORTED)
    set_target_properties(SQLite3::SQLite3 PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${SQLite3_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${SQLite3_LIBRARIES}"
    )
    if (PC_SQLite3_CPP_CFLAGS_OTHER)
        set_property(TARGET SQLite3::SQLite3 PROPERTY
            INTERFACE_COMPILE_OPTIONS "${PC_SQLite3_CPP_CFLAGS_OTHER}"
        )
    endif()
    if (PC_SQLite3_CPP_LDFLAGS_OTHER)
        set_property(TARGET SQLite3::SQLite3 PROPERTY
            INTERFACE_LINK_OPTIONS "${PC_SQLite3_CPP_LDFLAGS_OTHER}"
        )
    endif()
endif()
