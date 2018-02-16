/*-
 This program is a refactored Object Oriented C++14 implementation of
 worms.cpp -- A primitive sim example of wiggly worms moving about.
 The original worms.cpp (c) 2013 pmateti@wright.edu
 
 All modifications were made by erik.buck@wright.edu. Because this
 work bares little resemblence to the original, copyright for 
 this version is (c) erik.buck@wright.edu ALL RIGHTS RESERVED
 
*/

#include "Worm.h"
#include "WormsSim.h"
#include "CursesWormsSimUIStrategy.h"


int main(int argc, char * argv[])
{
    int slowness = std::max(0, 10*(argc > 1? argv[1][0] - '0' : 1));
    int displayWidth, displayHeight;
    
    CursesWormsSimUIStrategy::initializeForDisplay(
        displayWidth, displayHeight);
    
    WormsSim &sim(WormsSim::initSingletonSim(displayWidth, displayHeight));
    CursesWormsSimUIStrategy uiStrategy(sim);
    uiStrategy.setSlowness(slowness);
    
    for (bool shouldExit = false; !shouldExit; )
    {
        sim.runSimulation(uiStrategy);
        shouldExit = uiStrategy.confirmExit();
    }
    uiStrategy.releaseDisplay();
    
    return 0;
}

/* -eof- */
