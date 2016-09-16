find_package(Qt5Test REQUIRED)
# add_executable(integer_test integer-test.cpp)
# target_link_libraries(integer_test parser Qt5::Test)
# add_test(integer_test integer_test)

add_executable(repetition_test repetition-test.cpp)
target_link_libraries(repetition_test parser Qt5::Test)
add_test(repetition_test repetition_test)

add_executable(parse_test parsetest.cpp)
target_link_libraries(parse_test parser Qt5::Test)
add_test(parse_test parse_test)
