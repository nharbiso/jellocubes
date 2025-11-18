# Jello Cubes

A realtime renderer and physics engine for jello-like cubes, completed as a final project for [CSCI 1230](https://cs1230.graphics/website-fall-24/) at Brown University.

The program requires Qt Creator to be run; to run it, open up the program in Qt Creator and click run. Be sure to set your working directory to the `jellocubes/` folder!

Note that the physics parameters describing the jello cube and its collisions can lead to unexpected though intentional interactions with the environment. The cube cannot handle a high velocity impact (such as by repeatedly clicking "Scatter"), leading it to explode (i.e. its vertices fly out toward infinity) or get squashed. This can be finetuned by increasing the collision and cube's damping parameter respectively; however, these changes also make the cube less "jello"-like.
