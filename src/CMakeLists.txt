add_executable(basic_example_folly basic_example_folly.cpp)
add_dependencies(basic_example_folly folly)
target_link_libraries(basic_example_folly ${folly_LIBRARIES})

add_executable(holiday_booking_folly holiday_booking_folly.cpp)
add_dependencies(holiday_booking_folly folly)
target_link_libraries(holiday_booking_folly ${folly_LIBRARIES})

add_executable(try_complete_with_folly try_complete_with_folly.cpp)
add_dependencies(try_complete_with_folly folly)
target_link_libraries(try_complete_with_folly ${folly_LIBRARIES})