# Evolve
Evolve is a c++ implementation of the genetic algorithm for images described in Karl Sims' 1991 Paper: Artificial Evolution for Computer Graphics (http://www.karlsims.com/papers/siggraph91.html). It defines a symbolic grammar and provides a method to render the grammar as an image, as well as a Genetic Algorithm for the symbolic expressions.

Running Evolve.cpp will allow access to three provided demos. Further instruction is provided inside the software.

The SDL Library is required for graphics.

Implementation for Perlin Noise was taken from https://github.com/sol-prog/Perlin_Noise/.

<br /><br /><br />

Sample Images:


![Alt text](Images/Greyscale.png?raw=true "An Example Image")

( fraction ( | ( + ( noise 0.414332 ( min ( cos ( - Y Y ) ) ( abs Y ) ) 7.076917 ( if 8.053144 ( % ( sin ( & 2.628216 ( floor -8.215797 ) ) ) ( ^ ( if ( | 0.802172 Y ) ( | ( min ( * Y ( % ( abs ( % 4.220468 ( cos ( ^ Y X ) ) ) ) ( ^ Y ( cos X ) ) ) ) ( sin ( / ( min 9.875528 Y ) 5.439929 ) ) ) ( abs Y ) ) ( / X Y ) ( | ( cos X ) -3.212698 ) ) ( floor 5.960819 ) ) ) X ( max 7.680117 X ) ) ) 4.685479 ) ( | 0.298756 ( abs ( max X ( ^ Y ( ^ -1.690544 ( ^ X ( - X X ) ) ) ) ) ) ) ) )
<br /><br /><br />

![Alt text](Images/Color.png?raw=true "An Example Color Image")

Red: ( + ( % X ( sin Y ) ) ( + -7.414538 ( & -9.376418 6.525578 ) ) )

Green: ( | X 7.009321 )

Blue: ( abs ( | ( + ( | X ( abs ( noise ( * ( fraction ( | -5.912337 0.427945 ) ) X ) ( noise ( max Y X ) ( ^ ( fraction Y ) Y ) Y ( fraction ( if 5.525604 -6.676330 8.535641 X ) ) ) Y 6.672734 ) ) ) Y ) ( floor Y ) ) )
<br /><br /><br />

![Alt text](Images/Evolution.png?raw=true "An Example Generation")

An example generation created through the Evolution Demo
