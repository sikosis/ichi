# ICHI

A terminal version of UNO, called ICHI (Japanese for "one").

Designed by Sikosis.

## Requirements

- C++17 compiler (g++)

## Build

```
make
```

## Run

```
./ichi
```

Optional flags:

- `--us` — use American spelling (color) instead of Australian (colour)
- `--au` — use Australian spelling (colour), the default
- `--help` — show help and exit

Set `ICHI_UI=plain` to force plain prompts instead of gum/hum.

## Playing

- Enter your name (blank = Walter).
- Pick how many computer opponents (1-3), or type `q` to quit.
- On your turn, type the number of a card to play, `d` to draw, or `q` to quit.
- When you have one card left, call "ichi" — if you forget, you draw two.
- Match the active colour or value; skip, reverse, and draw cards spice things up.

## Stats

Each completed game updates `ichi.stats` in the current directory
(games played, wins/losses, win:loss ratio, streaks, and more).

## Clean

```
make clean
```