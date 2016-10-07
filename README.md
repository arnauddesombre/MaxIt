# MaxIt
Classical game of MaxIt

This repository includes the source code in C++, a compiled version (Windows 64-bits), and a configuration launch maxit.bat file, which allows to change the board size, the direction of player's play (horizontally or vertically), AI's parameterization...

Compilation instructions are included in the maxit.cpp source code, which is the only source file needed.

MaxIt's Artificial Intelligence is a Monte-Carlo, and the software uses parallel threading for maximum efficiency

Initially, MaxIt is a DOS game. One of the only documentation available is located at http://www.craigcolvin.com/Moosesoftware/maxit_rules.html

This version of MaxIt implements MaxIt (the winner has the highest score), MinIt (the winner has the lowest score), and ZeroIt (the winner has the smallest score in absolute value). Although a player's greedy strategy is more or less optimal, it is far from the case for MaxIt so this game is actually challenging.