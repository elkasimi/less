#pragma once

class Board;

class Strategy
{
public:
    virtual Action get_best_action( const Board& board ) = 0;
};
