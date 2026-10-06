# QUASAR

## ROADMAP

### Design and write the board representation
- [x] Represent the board
- [x] Represent the pieces
- [ ] Read a position from an FEN-string
- [ ] Keep the current state of the game
- [ ] Keep a history of played moves
- [x] Implement Zobrist hashing
- [ ] Create functions to control the board
- [ ] Create functions to get information from the board

### Design and write the move generator
- [ ] “Teach” the move generator how the pieces move
- [ ] Create a move format
- [ ] Generate moves for all the pieces when the MG is given a position
- [ ] Take special moves into account (such as castling)
- [ ] Adds generated moves to a move list and returns this when done
- [ ] Can generate captures and silent moves separately
- [ ] Can determine if a square is attacked

### Add perft (performance testing)
- [ ] Add a perft function that runs on a given position.
- [ ] Run through a perft suite containing “tricky” positions.

** First milestone reached: move generator is bug-free **
