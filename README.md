## Final Project: Jello Cubes

To run the program, simply open up the program in Qt Creator and click run. Be sure to set your working directory to the `jellocubes/` folder!

There are no known bugs our implementation. Note that the physics parameters describing the jello cube and its collisions can lead to unexpected though intentional interactions with the environment. The cube cannot handle a high velocity impact (such as by repeatedly clicking "Scatter"), leading it to explode (i.e. its vertices fly out toward infinity) or get squashed. This can be finetuned by increasing the collision and cube's damping parameter respectively; however, these changes also make the cube less "jello"-like.