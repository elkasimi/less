find_package(GTest)
if (GTest_FOUND)
    include_directories(${GTEST_INCLUDE_DIRS})

    set(SOURCES
        ../player/Common.h
        ../player/Board.h
        ../player/Board.cc
        ../player/Timer.h
        ../player/Timer.cc
        ../player/RandomNumberGenerator.h
        ../player/RandomNumberGenerator.cc
        BoardHorizontalWallNegativeTest.cc
        BoardHorizontalWallPositiveTest.cc
        BoardNilMoveTest.cc
        BoardTestBase.h
        BoardTestBase.cc
        BoardVerticalWallNegativeTest.cc
        BoardVerticalWallPositiveTest.cc
        main.cc
    )

    add_executable(PlayerTests
        ${SOURCES}
    )

    target_link_libraries(PlayerTests
        ${GTEST_LIBRARIES}
        pthread
    )
endif()

