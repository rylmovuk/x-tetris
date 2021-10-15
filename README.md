# X-Tetris

An attempt at making a terminal based game, inspired by Tetris, that relies only on the C standard library (and that tries to make as few assumptions about the OS / terminal environment as possible).

## Build

- Ascii compatible version:
    ```sh
    make tetris-compat
    ./tetris-compat
    ```
- Unicode graphics version for UTF-8 compatible terminals:
    ```sh
    make tetris
    ./tetris
    ```

You can use `make all` to build all the executables.