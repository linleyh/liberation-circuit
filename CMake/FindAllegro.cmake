
set (ALLEGRO_LIBRARY_PATH "" CACHE PATH "Allegro library directory")

if (ALLEGRO_INCLUDE_DIR)
    set (ALLEGRO_FIND_QUIETLY true)
endif ()

find_path (ALLEGRO_INCLUDE_DIR allegro.h
    /usr/local/include
    /usr/include
    $ENV {MINGDIR}/include)

if (UNIX AND NOT CYGWIN)
    execute_process (allegro-config ARGS --libs OUTPUT_VARIABLE ALLEGRO_LIBRARY)

else (UNIX AND NOT CYGWIN)

    find_library (ALLEGRO_LIBRARY
        NAMES allegro
        PATHS /usr/lib /usr/local/lib $ENV {MINGDIR}/lib ${ALLEGRO_LIBRARY_PATH})

    find_library (ALLEGRO_PRIMITIVES_LIBRARY
        NAMES allegro_primitives
        PATHS /usr/lib /usr/local/lib $ENV {MINGDIR}/lib ${ALLEGRO_LIBRARY_PATH})

    find_library (ALLEGRO_AUDIO_LIBRARY
        NAMES allegro_audio
        PATHS /usr/lib /usr/local/lib $ENV {MINGDIR}/lib ${ALLEGRO_LIBRARY_PATH})

    find_library (ALLEGRO_DIALOG_LIBRARY
        NAMES allegro_dialog
        PATHS /usr/lib /usr/local/lib $ENV {MINGDIR}/lib ${ALLEGRO_LIBRARY_PATH})

    find_library (ALLEGRO_ACODEC_LIBRARY
        NAMES allegro_acodec
        PATHS /usr/lib /usr/local/lib $ENV {MINGDIR}/lib ${ALLEGRO_LIBRARY_PATH})

    find_library (ALLEGRO_FONT_LIBRARY
        NAMES allegro_font
        PATHS /usr/lib /usr/local/lib $ENV {MINGDIR}/lib ${ALLEGRO_LIBRARY_PATH})

    find_library (ALLEGRO_IMAGE_LIBRARY
        NAMES allegro_image
        PATHS /usr/lib /usr/local/lib $ENV {MINGDIR}/lib ${ALLEGRO_LIBRARY_PATH})
endif ()

if (ALLEGRO_INCLUDE_DIR AND ALLEGRO_LIBRARY)
    set (ALLEGRO_FOUND true)
    set (ALLEGRO_LIBRARIES ${ALLEGRO_LIBRARY})

else (ALLEGRO_INCLUDE_DIR AND ALLEGRO_LIBRARY)
    set (ALLEGRO_FOUND false)
    set (ALLEGRO_LIBRARIES)
endif ()

if (ALLEGRO_FOUND)

    if (NOT ALLEGRO_FIND_QUIETLY)
        message (STATUS "Found Allegro: ${ALLEGRO_LIBRARY}")
    endif ()

else (ALLEGRO_FOUND)

    if (ALLEGRO_FIND_REQUIRED)
        message (FATAL_ERROR "Could not find Allegro libraries")
    endif ()
endif ()
