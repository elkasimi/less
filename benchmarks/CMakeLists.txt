set(SOURCES
    ../player/Board.h
    ../player/Board.cc
    ../player/Common.h
    ../player/Strategy.h
    ../player/RunStrategy.cc
    ../player/Timer.h
    ../player/Timer.cc
    ../player/RandomNumberGenerator.h
    ../player/RandomNumberGenerator.cc
    BoardBenchmark.cc
    main.cc
    RunStrategyBenchmark.cc
)

add_executable(PlayerBenchmarks
${SOURCES}
)

target_link_libraries(PlayerBenchmarks
benchmark
)
