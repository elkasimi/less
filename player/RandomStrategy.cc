#include "Common.h"

#include "RandomStrategy.h"

#include "Board.h"

RandomStrategy::RandomStrategy( )
{
}

RandomStrategy::~RandomStrategy( )
{
}

Action
RandomStrategy::get_best_action( const Board& board )
{
    Action best_action;
    const auto player = board.get_player( );
    Board next_board = board;

    while ( next_board.get_player( ) == player )
    {
        const auto random_move = next_board.get_random_move( );

        if ( random_move != NIL_MOVE )
        {
            best_action.push_back( random_move );
        }
        else
        {
            break;
        }

        next_board.do_move( random_move );
    }

    return best_action;
}
