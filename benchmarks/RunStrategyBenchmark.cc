#include <benchmark/benchmark.h>

#include "../player/Common.h"

#include "../player/Board.h"
#include "../player/RunStrategy.h"

namespace
{
const std::string layout(
    "01000000100000200001010012110001100000100100001010000000000100011010100010"
    "01000010000000000000100000101011001010" );
}

class RunStrategyBenchmark : public benchmark::Fixture
{
    void
    SetUp( benchmark::State& state ) override
    {
        benchmark::Fixture::SetUp( state );
    }
};

BENCHMARK_F( RunStrategyBenchmark, get_best_moves )( benchmark::State& state )
{
    Board::init_data( layout );
    Board::pre_compute( );
    Board board;
    board.enable_solo_mode( Board::YELLOW );
    RunStrategy run_strategy{};
    while ( state.KeepRunning( ) )
    {
        const auto best_moves = run_strategy.get_best_action( board );
        (void)best_moves;
    }
}
