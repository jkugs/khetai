# KhetAI

## Usage
```
require 'khetai'
move = KhetAI.move(board, whose_turn, search_depth)
```

## Example
```
require 'khetai'

        # A = anubis, P = pyramid, S = scarab, X = pharaoh, L = sphinx
        # capital letters = red, lowercase letters = silver
        # 0 = north, 1 = east, 2 = south, 3 = west
        # contains an extra row and column on all sides
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

# Red = 1, Silver = 0
whose_turn = 1

search_depth = 4

# returns [start_index, end_index, rotation]
# rotation can be 1 (clockwise), -1 (counter clockwise), or 0 (no rotation)
move = KhetAI.move(board, whose_turn, search_depth)
```

## Build and Deploy Commands
```
bundle exec rake compile
bundle ecec rake build
bundle exec rake release

gem push pkg/<gem>
```