# LEDs
```
[] - []
[] - []
[] - []
[] - []
```

# Buttons
```
[left] [right] - [select] [cancel]
```

# Usage
1. Start in STATE_INACTIVE. All LEDs remain unlit.
2. Press [select] to go to STATE_CHOOSE_OPPONENT. Middle 2 LEDs of left column will blink.
3. Press [left] and [right] to choose human or ai opponent respectively. Left column is human. Right column is ai. Press [cancel] to return to STATE_INACTIVE.
5. Press [select] to select opponent and begin game in STATE_PLAYING. Game state LEDs will light up. You start first.
6. Press [left] X number of times to select X items from the left column to remove. Will cycle from 1 through all remaining and back to 1. Selected items will blink. Pressing [right] will not do anything if [left] has been pressed. Pressing [cancel] will cancel the selection and thus allow [right] to be pressed with the same logic as above.
7. Press [select] to confirm removal. Game state updates internally and LEDs will reflect the updated state.
8. When game ends, go to STATE_SHOW_WINNER. If player wins, blink left column. If opponent wins, blink right column.
9. Press [select] to restart game and enter STATE_CHOOSE_OPPONENT again, or press [cancel] to enter STATE_INACTIVE.
