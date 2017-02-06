#include "Common.h"

#include "Board.h"
#include "RunStrategy.h"
#include "Timer.h"

RunStrategy::RunStrategy( )
{
}

RunStrategy::~RunStrategy( )
{
}

Action
RunStrategy::get_best_action( const Board& board )
{
    Action best_action;

    Timer timer;
    board.display( );

    auto next_board = board;
    auto player = board.get_player( );
    auto cost = 0;
    while ( not next_board.is_done( player ) )
    {
        auto best_move = next_board.get_best_running_move( );
        best_action.push_back( best_move );
        cost += next_board.do_move( best_move );
    }

    timer.stop( );

    std::cerr << "Using RunStrategy\n\tcost=" << cost << " dt=" << timer.get_delta_time( )
              << " tt=" << Timer::get_total_time( ) << "\n";

    return best_action;
}
