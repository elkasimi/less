#pragma once

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <queue>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#define N 8
#define NXN 64
#define NXNX8 512
#define NIL_MOVE ( (int32_t)-1 )
#define INVALID_MOVE ( (int32_t)-2 )
#define CREATE_MOVE( from, to ) ( ( from ) | ( ( to ) << 6 ) )
#define OO 1000000000
#define MAX_MOVES 17

using Move = int32_t;
using Action = std::vector< Move >;
