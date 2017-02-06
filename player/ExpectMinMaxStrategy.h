#pragma once

#include "Board.h"
#include "Strategy.h"
#include "TranspositionTable.h"

class ExpectMinMaxStrategy : public Strategy
{
public:
    explicit ExpectMinMaxStrategy( const Board::Player player );
    ~ExpectMinMaxStrategy( );

    Action get_best_action( const Board& board ) override;

private:
    void init_search( );
    double search( const Board& board,
                   const double min_value,
                   const double max_value,
                   const int32_t depth,
                   Move* best_move_ptr = nullptr );

    bool look_in_transposition_table( const Board& board,
                                      const int32_t depth,
                                      double& alpha,
                                      double& beta,
                                      double& value,
                                      Move* best_move_ptr,
                                      Move& PV );

    double get_best_value( const Board& board,
                           const double alpha,
                           const double beta,
                           const int32_t depth,
                           const Move PV,
                           Move* best_move_ptr );

    double get_average_value( const Board& board,
                              const double alpha,
                              const double beta,
                              const int32_t depth );

    double get_worst_value( const Board& board,
                            const double alpha,
                            const double beta,
                            const int32_t depth,
                            const Move PV );

    Board::Player m_player;
    Board::Player m_teammate;
    int32_t m_nodes;
    int32_t m_cutoff;
    bool m_can_abort;
    bool m_aborted;
    TranpositionTable m_transposition_table;
};
