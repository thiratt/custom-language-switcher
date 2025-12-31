cmake_minimum_required(VERSION 3.10)
project(CustomLanguageSwitcher)

set(CMAKE_CXX_STANDARD 20)
set(SOURCES main.cpp)

if(WIN32)
    list(APPEND SOURCES AppWindows.cpp CustomLanguageSwitcher.rc)
        
    add_executable(CustomLanguageSwitcher WIN32 ${SOURCES})
    target_link_libraries(CustomLanguageSwitcher PRIVATE user32 shell32)
    set_target_properties(CustomLanguageSwitcher PROPERTIES LINK_FLAGS "/ENTRY:mainCRTStartup /MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\"")

elseif(UNIX)
    list(APPEND SOURCES AppLinux.cpp)
    
    add_executable(CustomLanguageSwitcher ${SOURCES})
endif()

target_include_directories(CustomLanguageSwitcher PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})