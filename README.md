# KhetAI

An AI engine for the board game Khet. This gem provides move calculation for a specified color with configurable search depth and time limits.

## Installation

    $ gem install khetai

## Usage
```ruby
require 'khetai'
move = KhetAI.move(board, whose_turn, max_search_depth, max_search_time)
```

## Example
```ruby
require 'khetai'

# initial board setup:
# A = anubis, P = pyramid, S = scarab, X = pharaoh, L = sphinx
# capital letters = red, lowercase letters = silver
# 0 = north, 1 = east, 2 = south, 3 = west
board = ["L2", "--", "--", "--", "A2", "X2", "A2", "P1", "--", "--",
         "--", "--", "P2", "--", "--", "--", "--", "--", "--", "--",
         "--", "--", "--", "p3", "--", "--", "--", "--", "--", "--",
         "P0", "--", "p2", "--", "S2", "S3", "--", "P1", "--", "p3",
         "P1", "--", "p3", "--", "s1", "s0", "--", "P0", "--", "p2",
         "--", "--", "--", "--", "--", "--", "P1", "--", "--", "--",
         "--", "--", "--", "--", "--", "--", "--", "p0", "--", "--",
         "--", "--", "p3", "a0", "x0", "a0", "--", "--", "--", "l0"]

whose_turn = 1          # silver = 0, red = 1
max_search_depth = 25   # must be between 2 and 25
max_search_time = 5     # max search time in seconds

# returns [start_index, end_index, rotation]
move = KhetAI.move(board, whose_turn, max_search_depth, max_search_time)

# move[0] = start index
# move[1] = end index
# move[2] = rotation (1, -1, 0) (clockwise, anticlockwise, none)
```

The representation of the board gets converted an array of 8-bit integers once it is passed to the gem.

## Development

The internals of the gem are written in C, located in the [ext/khetai](/ext/khetai) directory.

## Development System Requirements

To build the gem from source, you'll need:
- Ruby >= `2.3.0`
- GCC or compatible C compiler
- Development headers (usually provided by ruby-dev or ruby-devel package)

## Build and Deploy Commands

This project uses `asdf` as the version manager for Ruby. However, any Ruby version >= `2.3.0` should work to compile this gem.

    $ gem install bundler
    $ bundle install
    $ bundle exec rake compile
    $ bundle exec rake build
    $ gem install pkg/khetai-<version>.gem

Once tested and verified, bump the version number in `lib/khetai/version.rb` and commit changes.

To release and push to rubygems.org:

    $ bundle exec rake release

To push a pre-built gem manually:

    $ gem push pkg/<gem>

## Manual Testing

There is a GUI test harness written in C++ using FLTK available in the [ext/khetai/dev/fltk-ui](/ext/khetai/dev/fltk-ui) directory.

### Why does this exist?

As something to work on and learn from.
