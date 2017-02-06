#include "BoardTestBase.h"

namespace
{
const std::string layout(
    "01000000100000200001010012110001100000100100001010000000000100011010100010"
    "01000010000000000000100000101011001010" );

const std::vector< int32_t > fields_with_no_vertical_wall{
    0,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 13, 15, 16, 19, 20, 21, 22, 23, 24, 26, 28, 29, 30,
    31, 32, 33, 34, 37, 39, 41, 42, 43, 44, 46, 47, 48, 49, 50, 51, 53, 54, 55, 57, 58, 60, 62, 63};
}

using ::testing::WithParamInterface;
using ::testing::ValuesIn;

class BoardVerticalWallNegativeTest : public BoardTestBase, public WithParamInterface< int32_t >
{
public:
    BoardVerticalWallNegativeTest( )
        : BoardTestBase( layout )
    {
    }
};

TEST_P( BoardVerticalWallNegativeTest, field_has_no_wall )
{
    const int32_t field = GetParam( );
    ASSERT_TRUE( has_no_vertical_wall( field ) );
    ASSERT_FALSE( has_vertical_wall( field ) );
}

INSTANTIATE_TEST_CASE_P( Walls,
                         BoardVerticalWallNegativeTest,
                         ValuesIn( fields_with_no_vertical_wall ) );
