#pragma once

#include "Board.h"
#include "Strategy.h"

class MCTSStrategy : public Strategy
{
public:
    explicit MCTSStrategy( const Board::Player player );
    ~MCTSStrategy( );

    Action get_best_action( const Board& board ) override;

private:
    struct PlayerMove
    {
        PlayerMove( Board::Player player, Move move )
            : player( player )
            , move( move )
        {
        }

        Board::Player player;
        Move move;
    };

    double run_simulation( Board& board,
                           const int32_t max_turns,
                           std::vector< PlayerMove >& moves );

    Board::Player m_player;
};
