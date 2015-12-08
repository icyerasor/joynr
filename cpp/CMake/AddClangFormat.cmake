function(add_clang_format TARGET)
    if(${ENABLE_CLANG_FORMATTER})
        set(COMMENT "running C++ code formatter: clang-format")
        set(COMMENT "${COMMENT}\nworking directory: ${CMAKE_CURRENT_SOURCE_DIR}")
	set(COMMENT "${COMMENT}\nprocessing files:\n${ARGN}")

	set(FORMAT_TARGET "format-${TARGET}")
	add_custom_target(
            ${FORMAT_TARGET}
	    COMMAND "${CLANG_FORMAT_PATH}" -i ${ARGN}
	    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	    COMMENT "${COMMENT}"
	    VERBATIM
        )
        add_dependencies(${TARGET} ${FORMAT_TARGET})
    endif(${ENABLE_CLANG_FORMATTER})
endfunction(add_clang_format)
