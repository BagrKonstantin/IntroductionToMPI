cmake_minimum_required(VERSION 4.2)

project(MyMpiProject C)

set(CMAKE_CXX_STANDARD 14)

find_package(MPI REQUIRED)


function(add_mpi_folder folder_path)
    file(GLOB LESSON_SOURCES "${folder_path}/*.c")

    foreach(source_file ${LESSON_SOURCES})
        get_filename_component(TARGET_NAME ${source_file} NAME_WLE)

        add_executable(${TARGET_NAME} ${source_file})
        target_link_libraries(${TARGET_NAME} PRIVATE MPI::MPI_C)
        target_compile_options(${TARGET_NAME} PRIVATE -Wextra --pedantic -std=c23 -march=native -DNDEBUG -O3)
    endforeach()
endfunction()

#add_mpi_folder("Examples/Lesson1")
#add_mpi_folder("Examples/Lesson2")
#add_mpi_folder("Examples/Lesson3")
#add_mpi_folder("Examples/Lesson4")
#add_mpi_folder("Examples/Lesson5")
#add_mpi_folder("Examples/Lesson6")

add_mpi_folder("Examples/Lesson8")
add_mpi_folder("Examples/Lesson9")


add_mpi_folder("Examples/Solutions")
add_mpi_folder("Examples/MySolutions")