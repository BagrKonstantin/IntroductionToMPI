cmake_minimum_required(VERSION 4.3)

project(MyMpiProject C)

set(CMAKE_CXX_STANDARD 14)


add_executable(1.SERIAL_TrapezoidalRule_sin Examples/Exercises/1.SERIAL_TrapezoidalRule_sin.c)
find_package(MPI REQUIRED)
target_link_libraries(1.SERIAL_TrapezoidalRule_sin PRIVATE MPI::MPI_C)
target_compile_options(1.SERIAL_TrapezoidalRule_sin PRIVATE -Wextra --pedantic -std=c23 -march=native -DNDEBUG -O3)


add_executable(1.4.PARALLEL_TrapezoidalRule Examples/MySolutions/1.4.PARALLEL_TrapezoidalRule.c)
find_package(MPI REQUIRED)
target_link_libraries(1.4.PARALLEL_TrapezoidalRule PRIVATE MPI::MPI_C)
target_compile_options(1.4.PARALLEL_TrapezoidalRule PRIVATE -Wextra --pedantic -std=c23 -march=native -DNDEBUG -O3)


add_executable(2.2.MPI_send_recv_arrays_barrier Examples/Lesson2/2.2.MPI_send_recv_arrays_barrier.c)
find_package(MPI REQUIRED)
target_link_libraries(2.2.MPI_send_recv_arrays_barrier PRIVATE MPI::MPI_C)
target_compile_options(2.2.MPI_send_recv_arrays_barrier PRIVATE -Wextra --pedantic -std=c23 -march=native -DNDEBUG -O3)


add_executable(2.3.MPI_send_recv_arrays_dataloss Examples/Lesson2/2.3.MPI_send_recv_arrays_dataloss.c)
find_package(MPI REQUIRED)
target_link_libraries(2.3.MPI_send_recv_arrays_dataloss PRIVATE MPI::MPI_C)
target_compile_options(2.3.MPI_send_recv_arrays_dataloss PRIVATE)


add_executable(2.5.MPI_bsend_recv_arrays Examples/Lesson2/2.5.MPI_bsend_recv_arrays.c)
find_package(MPI REQUIRED)
target_link_libraries(2.5.MPI_bsend_recv_arrays PRIVATE MPI::MPI_C)
target_compile_options(2.5.MPI_bsend_recv_arrays PRIVATE -Wextra --pedantic -std=c23 -march=native -DNDEBUG -O3)

add_executable(2.7.MPI_rsend_recv_arrays Examples/Lesson2/2.7.MPI_rsend_recv_arrays.c)
find_package(MPI REQUIRED)
target_link_libraries(2.7.MPI_rsend_recv_arrays PRIVATE MPI::MPI_C)
target_compile_options(2.7.MPI_rsend_recv_arrays PRIVATE -Wextra --pedantic -std=c23 -march=native -DNDEBUG -O3)


add_executable(2.SERIAL_dot_product Examples/Exercises/2.SERIAL_dot_product.c)
find_package(MPI REQUIRED)
target_link_libraries(2.SERIAL_dot_product PRIVATE MPI::MPI_C)
target_compile_options(2.SERIAL_dot_product PRIVATE -Wextra --pedantic -std=c23 -march=native -DNDEBUG -O3)


add_executable(2.8.PARALLEL_dot_product Examples/MySolutions/2.8.PARALLEL_dot_product.c)
find_package(MPI REQUIRED)
target_link_libraries(2.8.PARALLEL_dot_product PRIVATE MPI::MPI_C)
target_compile_options(2.8.PARALLEL_dot_product PRIVATE -Wextra --pedantic -std=c23 -march=native -DNDEBUG -O3)


add_executable(3.1.MPI_hello_world_PP Examples/Lesson3/3.1.MPI_hello_world_PP.c)
find_package(MPI REQUIRED)
target_link_libraries(3.1.MPI_hello_world_PP PRIVATE MPI::MPI_C)
target_compile_options(3.1.MPI_hello_world_PP PRIVATE -Wextra --pedantic -std=c23 -march=native -DNDEBUG -O3)

add_executable(3.2.MPI_send_recv_arrays_PP Examples/Lesson3/3.2.MPI_send_recv_arrays_PP.c)
find_package(MPI REQUIRED)
target_link_libraries(3.2.MPI_send_recv_arrays_PP PRIVATE MPI::MPI_C)
target_compile_options(3.2.MPI_send_recv_arrays_PP PRIVATE -Wextra --pedantic -std=c23 -march=native -DNDEBUG -O3)

add_executable(3.3.MPI_Wtime_Wtick Examples/Lesson3/3.3.MPI_Wtime_Wtick.c)
find_package(MPI REQUIRED)
target_link_libraries(3.3.MPI_Wtime_Wtick PRIVATE MPI::MPI_C)
target_compile_options(3.3.MPI_Wtime_Wtick PRIVATE -Wextra --pedantic -std=c23 -march=native -DNDEBUG -O3)