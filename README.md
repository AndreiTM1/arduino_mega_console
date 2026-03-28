README

This is one of my personal projects where I wanted to code some simple games in order to play them on my Arduino Mega because it has plenty of RAM and flash to fit a kind of this project. I intended it for a 3.5 inch TFT display shield so if you want to use it make sure you use the TFT_HX8357.h library found on GitHub. Other components I used are 4 push buttons, male to male wires for a common ground and male to female wires with 90 degrees heather pins to connect the buttons to the digital pins (due to the LCD covering the pin holes).

I have implemented 2 games so far: Snake and Tetris. They both have a score system and the highscore is stored in the EEPROM.

SNAKE

This is a classic Snake game where your snake, score and speed increase after eating food. It has the usual rules, however the snake dies when you hit a wall because I haven't implemented the feature of it coming out from the other side of the screen yet.

TETRIS

This game follows the classic Tetris rules. It can be played both in portrait and in landscape mode. When a line is complete it is removed and the above pieces are moved down. The score as it is now is you get 10 pts. for a line removed, and when *at least* one line gets removed the speed increases. 
What I intend to change is to show what the next piece you will get, to change the speed based on the score rather than lines removed, to give the player a score bonus multiplier for more lines removed at once and to implement another button used to hold a piece for later.
