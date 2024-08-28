cmake_minimum_required(VERSION 3.20)

function(compile_rpsl_file RpslFile)
    cmake_path(GET CMAKE_CURRENT_SOURCE_DIR PARENT_PATH PARENT_DIR)

    get_filename_component(FileNameWithoutExtension ${RpslFile} NAME_WE)    

    string( APPEND RpsCompileOpts "$<IF:$<CONFIG:DEBUG>,-O0,-O3>")

    set(Input_File_Dir ${CMAKE_CURRENT_SOURCE_DIR}/Rendering/RPSL)
    set(Output_File_Dir ${CMAKE_CURRENT_BINARY_DIR}/rpsl)

    set(InputFile ${Input_File_Dir}/${RpslFile}.rpsl)
    set(OutputFile ${Output_File_Dir}/${FileNameWithoutExtension}.rpsl.g.c)
    
    if (WIN32)
        set(RpsCompilerBinaryDir "${CMAKE_CURRENT_SOURCE_DIR}/External/AMDRenderPipelineShaders/tools/rps_hlslc/win-x64/")
        set(ExecPostfix ".exe")
    elseif (UNIX)
        set(RpsCompilerBinaryDir "${CMAKE_CURRENT_SOURCE_DIR}/External/AMDRenderPipelineShaders/tools/rps_hlslc/linux-x64/bin/")
    else ()
        message(SEND_ERROR "Unsupported OS")
    endif()

    set(RpsHlslcExec "${RpsCompilerBinaryDir}/rps-hlslc${ExecPostfix}")    

    add_custom_command(
        OUTPUT ${OutputFile}
        COMMAND "${RpsHlslcExec}" "${InputFile}" -od "${Output_File_Dir}" -m ${FileNameWithoutExtension} ${RpsCompileOpts}
        COMMENT "Compiling RPSL file ${InputFile} to ${OutputFile}\n"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS ${InputFile}
    )

    list(APPEND RpsBinaryOutput ${OutputFile})
    set(RpsBinaryOutput ${RpsBinaryOutput} PARENT_SCOPE)
endfunction()

function(add_rpsl_dependencies PROJECT_TARGET)
    if (NOT RpsBinaryOutput)
        return()
    endif ()

    add_custom_target(Rpsl ALL DEPENDS ${RpsBinaryOutput})
    add_dependencies(${PROJECT_TARGET} Rpsl)

    set(Rpsl_Intermediate_Output_Dir ${CMAKE_CURRENT_BINARY_DIR}/rpsl)
    set(Rpsl_Generated_Output_Dir ${CMAKE_CURRENT_SOURCE_DIR}/Rendering/RPSL/Generated)

    add_custom_command(TARGET Rpsl
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${Rpsl_Generated_Output_Dir}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${Rpsl_Generated_Output_Dir}
    )

    foreach (RpsBinary ${RpsBinaryOutput})
        # get relative path of RpsBinaryOutput to Rpsl_Generated_Output_Dir
        file(RELATIVE_PATH RpsBinaryRelativePath ${Rpsl_Intermediate_Output_Dir} ${RpsBinary})
        add_custom_command(TARGET Rpsl
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${RpsBinary}" "${Rpsl_Generated_Output_Dir}/${RpsBinaryRelativePath}"
            COMMENT "Copying ${RpsBinary} to ${Rpsl_Generated_Output_Dir}/${RpsBinaryRelativePath}"
        )
    endforeach()
    
endfunction()