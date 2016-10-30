# MaxIt
Classical game of MaxIt

This repository includes the source code in C++11, a compiled version (for Windows 64-bits), and a well documented configuration launch file (maxit.bat), which allows the change of board size, the direction of player's move (horizontally or vertically), AI's parameterization...

Compilation instructions are included in the maxit.cpp stand-alone source code - it is the only source file needed. I've used MinGW and compiled using:

    g++ -Wall -O3 -o "maxit" "maxit.cpp" -s -std=c++11 -static-libgcc -static-libstdc++ -static -lwinpthread
If you have MinGW installed, you can generate a smaller .exe file using:

    g++ -Wall -O3 -o "maxit" "maxit.cpp" -s -std=c++11
MaxIt's Artificial Intelligence is a Monte-Carlo, and the software uses parallel threading for maximum efficiency. Monte-Carlo is a fairly efficient strategy for a simple game like MaxIt and the computer will play almost instantaneously, and quite accurately...

Initially, MaxIt is a DOS game. One of the only documentation available is located [here](http://www.craigcolvin.com/Moosesoftware/maxit_rules.html). (From the game, press F1 to display the help screen at any time.)

This version of MaxIt implements **MaxIt** (the winner has the highest score), **MinIt** (the winner has the lowest score), and **ZeroIt** (the winner has the smallest score in absolute value). Although a player's greedy strategy is more or less optimal for MaxIt and MinIt, it is far from the case for ZeroIt so this game is actually more challenging.
