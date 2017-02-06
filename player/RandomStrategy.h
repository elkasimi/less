#pragma once

#include "Strategy.h"

class RandomStrategy : public Strategy
{
public:
    RandomStrategy( );
    ~RandomStrategy( );

    Action get_best_action( const Board& board ) override;
};
