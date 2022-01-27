# X-Tetris

An attempt at making a terminal based game, inspired by Tetris, that relies only on the C standard library (and that tries to make as few assumptions about the OS / terminal environment as possible). 
(As such, cursor movement, color sequences, non-ascii characters etc. are avoided.)

## Build

Default: release build in root directory
```sh
# make clean
make
./x-tetris
```

For debug & release versions under build/:
```sh
make debug
make release
```
