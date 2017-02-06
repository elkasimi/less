#include "BoardTestBase.h"

namespace
{
const std::string layout(
    "01000000100000200001010012110001100000100100001010000000000100011010100010"
    "01000010000000000000100000101011001010" );

const std::vector< int32_t > fields_with_no_vertical_wall{
    0,  2,  3,  4,  5,  6,  8,  9,  14, 15, 16, 18, 19, 21, 22, 23, 24,
    25, 26, 27, 28, 29, 30, 32, 34, 35, 36, 38, 39, 40, 41, 42, 43, 44,
    45, 46, 47, 48, 49, 50, 52, 54, 56, 57, 58, 59, 60, 61, 62, 63};
}

using ::testing::WithParamInterface;
using ::testing::ValuesIn;

class BoardHorizontalWallNegativeTest : public BoardTestBase, public WithParamInterface< int32_t >
{
public:
    BoardHorizontalWallNegativeTest( )
        : BoardTestBase( layout )
    {
    }
};

TEST_P( BoardHorizontalWallNegativeTest, field_has_no_wall )
{
    const int32_t field = GetParam( );
    ASSERT_TRUE( has_no_horizontal_wall( field ) );
    ASSERT_FALSE( has_horizontal_wall( field ) );
}

INSTANTIATE_TEST_CASE_P( Walls,
                         BoardHorizontalWallNegativeTest,
                         ValuesIn( fields_with_no_vertical_wall ) );
