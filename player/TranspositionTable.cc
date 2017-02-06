#include "Common.h"

#include "Board.h"
#include "TranspositionTable.h"

namespace
{
const int32_t buckets_count = 1000003;
}

TranpositionTable::TranpositionTable( )
    : m_buckets( buckets_count )
{
}

TranpositionTable::~TranpositionTable( )
{
}

const Entry*
TranpositionTable::find( const Board& board )
{
    const Entry* entry = nullptr;

    const auto board_hash = board.get_hash( );
    const auto board_lock = board.get_lock( );
    const auto bucket_index = board_hash % buckets_count;
    if ( m_buckets[ bucket_index ].lock == board_lock )
    {
        entry = &m_buckets[ bucket_index ];
    }

    return entry;
}

void
TranpositionTable::store( const Board& board,
                          const int32_t depth,
                          const Entry::Type type,
                          const double value,
                          const Move move )
{
    const auto board_hash = board.get_hash( );
    const auto board_lock = board.get_lock( );
    const auto bucket_index = board_hash % buckets_count;
    Entry* entry = &m_buckets[ bucket_index ];
    entry->lock = board_lock;
    entry->depth = depth;
    entry->type = type;
    entry->value = value;
    entry->move = move;
}
