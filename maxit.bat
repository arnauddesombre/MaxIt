@echo off
goto begin

***************
MaxIt by Arnaud
***************

Use keyboard keys:
Arrows, Ctrl-Arrows, Enter, Space Bar, F1 (help)

syntax:
maxit <game> <board_size> <player_horizontal> <max_number> <processors> <monte_carlo> <display_computer_score> <display_modifs>

<game>			game played						(1 = MaxIt (default), 2 = MinIt, 3 = ZeroIt)
<board_size>		the size of the board					(default = 7, cannot be lower than 3)
<player_horizontal>	player plays horizontally				(default = YES)
<max_number>		maximum number on the board (included)			(default = 50, cannot be lower than 1)
<processors>		number of core processors to use			(default = 2, cannot be lower than 1)
<monte_carlo>		cumulated number of Monte Carlo paths for computer's AI	(default = 25000, cannot be lower than 100)
<display_score>		display score assessed on last computer's move		(default = YES)
<display_modifs>	YES/NO; display parameters screen on startup		(default = YES)

Update parameters below to change the defaults

*************

:begin
set game=1
set board_size=8
set player_horizontal=YES
set max_number=50
set processors=6
set monte_carlo=25000
set display_score=YES
set display_modifs=NO

maxit %game% %board_size% %player_horizontal% %max_number% %processors% %monte_carlo% %display_score% %display_modifs%

set game=
set board_size=
set player_horizontal=
set max_number=
set processors=
set monte_carlo=
set display_score=
set display_modifs=