#include "BoardTestBase.h"

namespace
{
const std::string layout(
    "01000000100000200001010012110001100000100100001010000000000100011010100010"
    "01000010000000000000100000101011001010" );

const std::vector< int32_t > fields_with_vertical_wall{1,  12, 14, 17, 18, 25, 27, 35,
                                                       36, 38, 40, 45, 52, 56, 59, 61};
}

using ::testing::WithParamInterface;
using ::testing::ValuesIn;

class BoardVerticalWallPositiveTest : public BoardTestBase, public WithParamInterface< int32_t >
{
public:
    BoardVerticalWallPositiveTest( )
        : BoardTestBase( layout )
    {
    }
};

TEST_P( BoardVerticalWallPositiveTest, field_has_wall )
{
    const int32_t field = GetParam( );
    ASSERT_TRUE( has_vertical_wall( field ) );
    ASSERT_FALSE( has_no_vertical_wall( field ) );
}

INSTANTIATE_TEST_CASE_P( Walls,
                         BoardVerticalWallPositiveTest,
                         ValuesIn( fields_with_vertical_wall ) );
