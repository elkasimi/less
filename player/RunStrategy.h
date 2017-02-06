#pragma once

#include "Board.h"
#include "Strategy.h"

class RunStrategy : public Strategy
{
public:
    RunStrategy( );
    ~RunStrategy( );

    Action get_best_action( const Board& board ) override;
};
