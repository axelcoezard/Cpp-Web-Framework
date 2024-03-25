cmake_minimum_required(VERSION 3.12)
project(CppWebFramework)

set(CMAKE_CXX_STANDARD 20)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

# Recherche de la bibliothèque libCurl
find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIR})
link_directories(${CURL_LIBRARY})

# Recherche de la bibliothèque Boost
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})

include_directories(Sources/)

add_subdirectory("ThirdParty/json")

# Ajoutez vos fichiers source ici
add_executable(${PROJECT_NAME}
	Sources/main.cpp
	Sources/Http.hpp
	Sources/OAuth/OAuth.hpp)

# Lier les bibliothèques
target_link_libraries(${PROJECT_NAME} ${CURL_LIBRARIES} ${Boost_LIBRARIES} nlohmann_json)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/json/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Sources)
