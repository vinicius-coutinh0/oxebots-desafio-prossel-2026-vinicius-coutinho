cmake_minimum_required(VERSION 3.14)
project(SimuladorFutebol)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# Habilita o log de download
set(FETCHCONTENT_QUIET FALSE)

# Fetch SFML via URL 
FetchContent_Declare(
    SFML
    URL https://github.com/SFML/SFML/archive/refs/tags/2.6.1.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

set(SFML_BUILD_AUDIO FALSE)
set(SFML_BUILD_NETWORK FALSE)
set(SFML_BUILD_DOC FALSE)
set(SFML_BUILD_EXAMPLES FALSE)

FetchContent_MakeAvailable(SFML)

# Busca arquivos do simulador e solução
file(GLOB_RECURSE SIMULADOR_SOURCES "simulador/src/*.cpp")
file(GLOB_RECURSE SOLUCAO_SOURCES "solucao/src/*.cpp")

# Add Executable
add_executable(SimuladorFutebol
    ${SIMULADOR_SOURCES}
    ${SOLUCAO_SOURCES}
)

# Inclui os diretórios de headers
target_include_directories(SimuladorFutebol PRIVATE 
    simulador/include 
    solucao/include
)

target_link_libraries(SimuladorFutebol PRIVATE 
    sfml-graphics 
    sfml-window 
    sfml-system
)

# --- Automação de Pós-Compilação (Windows) ---
if(WIN32)
    # Copia a fonte
    add_custom_command(TARGET SimuladorFutebol POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different 
            "${CMAKE_CURRENT_SOURCE_DIR}/assets/arial.ttf" 
            "$<TARGET_FILE_DIR:SimuladorFutebol>/arial.ttf"
        COMMENT "Copiando fonte..."
    )

    # Copia as DLLs do SFML (Graphics, Window, System)
    add_custom_command(TARGET SimuladorFutebol POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:sfml-graphics> "$<TARGET_FILE_DIR:SimuladorFutebol>/"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:sfml-window>   "$<TARGET_FILE_DIR:SimuladorFutebol>/"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:sfml-system>   "$<TARGET_FILE_DIR:SimuladorFutebol>/"
        COMMENT "Copiando DLLs do SFML..."
    )

    # Copia a OpenAL se ela existir (dependendo da arquitetura x64/x86)
    add_custom_command(TARGET SimuladorFutebol POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different 
            "${sfml_SOURCE_DIR}/extlibs/bin/x64/openal32.dll" 
            "$<TARGET_FILE_DIR:SimuladorFutebol>/"
        COMMENT "Copiando OpenAL..."
    )
endif()
