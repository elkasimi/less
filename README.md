![Less board](http://www.codecup.nl/images/leeg_bord.png)


This is my player which ranks second in CodeCup 2017 with the game "less".

For an overview of the rules of game less:

[LESS-RULES](http://www.codecup.nl/rules_less.php)

# *AI description*

I used a Monte Carlo Tree Search (MCTS) based strategy enforced with Rapid Action Value Estimation.

I tried to use ExpectMinMax Approach but it doesn't perform as well as MCTS due to the lack of a good evaluation function.

The two stratgies are implemented to try Exepect-Min-Max you need just to change AdoptedStratgey to ExpectMinMaxStrategy in:
player/main.cc

The ExpectMinMax was used to tune some parameters of the MCTS-RAVE.

For an introduction to MCTS please visit:
[MCTS-WIKI](https://en.wikipedia.org/wiki/Monte_Carlo_tree_search)

The paper "A Survey of Monte Carlo Tree Search Methods" by Cameron Browne et al. contains a very good description of MCTS and its enhancements.

# *Board representation and conventions*
I used a bitmask for every player to encode his stones. for every player his stones are encoded as a 64 bit unsigned integer.

### *Action*
An action is a list of moves no more than 3 and no less than one.

### *Move*
A move is an integer where it is encoded the pair (from, to).

NIL_MOVE is a convention for doing nothing and stop player moves.

INVALID_MOVE is a convention for an invalid move.

### *Running(My own definition)*
- A player is considered running when there is no other player between him and its target.
- A game is considered running if all players are running.

# *Enhancements*

## *Trade memory for speed*

All possibles movements for a bitmask are computed and stored in less than 0.2 second.

The distance of a player to his target is precomputed at the start. all possible bitmasks for all players are precomputed in less than 2 seconds.

I was using a hand written hashmap to store those data. the hashmap uses a simple allocation from a
continuous array to have a cache friendly processing. After I found that there is a nice formula
to know a combination from an index and this was a better approach. Please have a look at:
![Formula](https://en.wikipedia.org/wiki/Combinatorial_number_system#Finding_the_k-combination_for_a_given_number)
This optimized a lot the retrieval of data

As the log and sqrt are very used the values of sqrt(n) and sqrt(log(n)) are stored for every n less than or equal the max possible value for n.

# *MCTS-RAVE*
### *Selection*
In the MCTS-RAVE the selection step is done in 3 different ways:

1. For me choose the best one

2. For my teammate choose randomly based on the weight of the move

3. Else choose the best for opponent team(The worst for me)

I used the proposed formula by Gelly in his paper:

"Monte-Carlo Tree Search and Rapid Action Value Estimation in Computer Go" to bias move values with RAVE estimates.

The formula propose a parameter b to be tuned heuristically. b represent the bias between RAVE values and MC values.

I used the following approach:

For a given Tree node:

- If the number of simulations is less than a threshold(=100) consider b = 0.0

- else b = value(RAVE) - value(MC)

### *Expansion*
Expansion is done following "First Play Urgency" using a sorted moves list based on distance to target.

Expansion is stopped at a maximum level because I found that letting Tree expand indefinitely can lead to weaker play because of inaccurate results on big levels.

### *Simulation*
I used a simulation(playout) 16 turns forward instead of full playouts with simple random moves.

Random moves are chosen to be as near as possible from player target.
The check of end game is replaced with the check of the game is running.

### *Back propagation*
As transpositions was a lot in the tree I used UCT-2 version from the paper:
"Transpositions and Move Groups in Monte Carlo Tree Search" by Benjamin E. Childs, James H. Brodeur and Levente Kocsis.

This gives a lot of information to the tree search.

The transposed nodes share same Data so whenever a node has a value to back propagate it is shared among all those nodes.

### *Consisteny check*
Sometimes the most visited node is not the best one.
In this case a limited check iterations is done to find the best node and have a consistent state where the most visited is the best one(at least for level 0).

### *Prunning*
When a player is running only a running move is chosen(all others are equivalent or worse) so one move is expanded instead of many.

### *Bias*
Every node is biased towards its move weight.

# *ExpectMinMax*
It is like alpha-beta with Transposition Table and a very simple evaluation function based on current distance with players mobility.

# *Tests*
Some regression tests were added using Google test framework to ensure that the same bugs is not reproduced any more after its detection

# *Benchmarks*
Some benchmarks were added using Google benchmark framework for some critical functions such as Board::get_random_move which is heavily used in MCTS playout
