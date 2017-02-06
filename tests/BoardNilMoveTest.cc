#include "BoardTestBase.h"

namespace
{
const std::string layout(
    "01000000100000200001010012110001100000100100001010000000000100011010100010"
    "01000010000000000000100000101011001010" );
}

class BoardNilMoveTest : public BoardTestBase
{
public:
    BoardNilMoveTest( )
        : BoardTestBase( layout )
    {
    }
};

TEST_F( BoardNilMoveTest, can_do_nil_move_when_team_is_done )
{
    const uint64_t bitmasks[ 4 ]
        = {49344ull, 13889101250810609664ull, 68719477016ull, 217017207043915776ull};
    Board board{Board::RED, bitmasks};

    int32_t count = 0;
    Move available_move = INVALID_MOVE;
    for ( const auto move : board )
    {
        ++count;
        if ( count > 1 )
        {
            break;
        }

        available_move = move;
    }

    ASSERT_EQ( 1, count );
    ASSERT_EQ( NIL_MOVE, available_move );
}
