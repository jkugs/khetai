# KhetAI

## Usage
```
require 'khetai'
move = KhetAI.move(board, whose_turn, max_search_depth, max_time_seconds)
```

## Example
```
require 'khetai'

# initial board setup:
# A = anubis, P = pyramid, S = scarab, X = pharaoh, L = sphinx
# capital letters = red, lowercase letters = silver
# 0 = north, 1 = east, 2 = south, 3 = west
board = ["--", "--", "--", "--", "--", "--", "--", "--", "--", "--", "--", "--",
        "--", "L2", "--", "--", "--", "A2", "X2", "A2", "P1", "--", "--", "--",
        "--", "--", "--", "P2", "--", "--", "--", "--", "--", "--", "--", "--",
        "--", "--", "--", "--", "p3", "--", "--", "--", "--", "--", "--", "--",
        "--", "P0", "--", "p2", "--", "S2", "S3", "--", "P1", "--", "p3", "--",
        "--", "P1", "--", "p3", "--", "s1", "s0", "--", "P0", "--", "p2", "--",
        "--", "--", "--", "--", "--", "--", "--", "P1", "--", "--", "--", "--",
        "--", "--", "--", "--", "--", "--", "--", "--", "p0", "--", "--", "--",
        "--", "--", "--", "p3", "a0", "x0", "a0", "--", "--", "--", "l0", "--",
        "--", "--", "--", "--", "--", "--", "--", "--", "--", "--", "--", "--"]

whose_turn = 1          # silver = 0, red = 1
max_search_depth = 25   # must be between 2 and 25
max_search_time = 5     # max search time in seconds

# returns [start_index, end_index, rotation]
move = KhetAI.move(board, whose_turn, max_search_depth, max_search_time)

# move[0] = start index
# move[1] = end index
# move[2] = rotation (1, -1, 0) (clockwise, anticlockwise, none)
```

## Build and Deploy Commands
```
bundle exec rake compile
bundle ecec rake build
bundle exec rake release

gem push pkg/<gem>
```