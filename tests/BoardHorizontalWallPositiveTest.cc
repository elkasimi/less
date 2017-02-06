#include "BoardTestBase.h"

namespace
{
const std::string layout(
    "01000000100000200001010012110001100000100100001010000000000100011010100010"
    "01000010000000000000100000101011001010" );

const std::vector< int32_t > fields_with_vertical_wall{1,  7,  10, 11, 12, 13, 17,
                                                       20, 31, 33, 37, 51, 53, 55};
}

using ::testing::WithParamInterface;
using ::testing::ValuesIn;

class BoardHorizontalWallPositiveTest : public BoardTestBase, public WithParamInterface< int32_t >
{
public:
    BoardHorizontalWallPositiveTest( )
        : BoardTestBase( layout )
    {
    }
};

TEST_P( BoardHorizontalWallPositiveTest, field_has_wall )
{
    const int32_t field = GetParam( );
    ASSERT_TRUE( has_horizontal_wall( field ) );
    ASSERT_FALSE( has_no_horizontal_wall( field ) );
}

INSTANTIATE_TEST_CASE_P( Walls,
                         BoardHorizontalWallPositiveTest,
                         ValuesIn( fields_with_vertical_wall ) );
