# MaxIt
Classical game of MaxIt

This repository includes the source code in C++11, a compiled version (Windows 64-bits), and a configuration launch maxit.bat file, which allows to change the board size, the direction of player's play (horizontally or vertically), AI's parameterization...

Compilation instructions are included in the maxit.cpp source code, which is the only source file needed. I've used MinGW and the instructions are for a Geany build.

MaxIt's Artificial Intelligence is a Monte-Carlo, and the software uses parallel threading for maximum efficiency. Monte-Carlo is a fairly efficient strategy for a simple game like MaxIt and the computer will play almost instantaneously...

Initially, MaxIt is a DOS game. One of the only documentation available is located at http://www.craigcolvin.com/Moosesoftware/maxit_rules.html

This version of MaxIt implements MaxIt (the winner has the highest score), MinIt (the winner has the lowest score), and ZeroIt (the winner has the smallest score in absolute value). Although a player's greedy strategy is more or less optimal for MaxIt and MinIt, it is far from the case for ZeroIt so this game is actually more challenging.
