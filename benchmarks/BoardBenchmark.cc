#include <benchmark/benchmark.h>

#include "../player/Common.h"

#include "../player/Board.h"

namespace
{
const std::string layout(
    "01000000100000200001010012110001100000100100001010000000000100011010100010"
    "01000010000000000000100000101011001010" );
}

class BoardBenchmark : public benchmark::Fixture
{
    void
    SetUp( benchmark::State& state ) override
    {
        benchmark::Fixture::SetUp( state );
    }
};

BENCHMARK_F( BoardBenchmark, init_data )( benchmark::State& state )
{
    while ( state.KeepRunning( ) )
    {
        Board::init_data( layout );
    }
}

BENCHMARK_F( BoardBenchmark, evaluate )( benchmark::State& state )
{
    Board::init_data( layout );
    Board::pre_compute( );

    Board board;
    while ( state.KeepRunning( ) )
    {
        board.evaluate( Board::YELLOW );
    }
}

BENCHMARK_F( BoardBenchmark, get_random_move )( benchmark::State& state )
{
    Board::init_data( layout );
    Board::pre_compute( );

    Board board;
    while ( state.KeepRunning( ) )
    {
        board.get_random_move( );
    }
}
