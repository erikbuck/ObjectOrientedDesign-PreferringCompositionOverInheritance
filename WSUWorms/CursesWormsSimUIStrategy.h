#ifndef CURSESDRAWWORMSGAMEDECORATOR_H // Guard
#define CURSESDRAWWORMSGAMEDECORATOR_H

#include "Worm.h"
#include "WormsSim.h"

//////////////////////////////////////////////////////////////////////
/// Instances of CursesWormsSimUIStrategy provide NCurses based
/// graphical display and user interaction of WormsSim instances.
///
/// Design Notes:
/// - CursesWormsSimUIStrategy implements the interface specified by the
/// abstract AbstractWormsSimUIStrategy class and participates in the
/// Strategy design pattern to decouple display and user input from
/// simulation encapsulation.
/// - CursesWormsSimUIStrategy adheres to the practical advice from
/// Scott Meyers, “More Effective C++: 35 New Ways to Improve Your
/// Programs and Designs 1st Edition” Item 33, “Make non-leaf classes
/// abstract” as elaborated to use the Template Method design pattern
/// in “Virtuality: C/C++ Users Journal, 19, September 2001”
/// http://www.gotw.ca/publications/mill18.htm
///
//////////////////////////////////////////////////////////////////////
class CursesWormsSimUIStrategy : public AbstractWormsSimUIStrategy
{
private:
    static const char esc = '\033'; //< the ESC char ASCII code
    static const int rowsInMessageArea = 5; //< Arbitrary number

    /* parameters for the 'graphical' (such as it is) display */
    static int slowness;             //< Total number of delayQuantum intervals before each call to processUserInput() returns
    static int delayQuantum;         //< an amount of time in milliseconds
    static int delayRemaining;       //< remaining number of delayQuantum intervals before each call to processUserInput() returns
    static bool isPaused;            //< == true iff isPaused

    // See documentation in implementation file
    static void endCurses();

    // See documentation in implementation file
    static void startCurses(int &asogRows, int &asogCols);

    // See documentation in implementation file
    static std::map<int, int> wormAttr;
    
    /// The simulation instance to be displayed in a user interface
    /// specific way.
    WormsSim &m_sim;

    // See documentation in implementation file
    int getOneChar();

    // See documentation in implementation file
    char handleUserKeyPress(char c);

    /// Draw specified string of characters on row y on the display.
    void showMessage(
        std::string text, //< String of characters to show
        int y //< Row number on which to show characters on display
    );
    
    // Draw user interface specific information about the status of
    // a simulation
    void showStatus();
    
public:
    CursesWormsSimUIStrategy(
       WormsSim &sim) //< The simulation to be used by the strategy
       : m_sim(sim)
    {}
    
    //////////////////////////////////////////////////////////////////
    /// Call this function once before calling any other
    /// CursesWormsSimUIStrategy functions. out_width and out_height
    /// respectively are set the the width and height in characters of
    /// the available display.
    static void initializeForDisplay(
       int &out_width,   //< The width of the available display in characters
       int &out_height); //< The height of the available display in characters
       
    //////////////////////////////////////////////////////////////////
    /// Call this function to make the available display if any being
    /// used by CursesWormsSimUIStrategy available for use by other
    /// objects.
    static void releaseDisplay();
    
    //////////////////////////////////////////////////////////////////
    /// Returns the total number of delayQuantum intervals before
    /// each call to processUserInput() returns
    static int getSlowness() { return slowness; }

    //////////////////////////////////////////////////////////////////
    /// Sets the total number of delayQuantum intervals before
    /// each call to processUserInput() returns. Change the slowness
    /// to speed up or slow down the rate at which the simulation
    /// that calls processUserInput() can execute simulation steps.
    static void setSlowness(int aSlowness) { slowness = slowness; }
    
    //////////////////////////////////////////////////////////////////
    /// Call this function to request user interface specific
    /// confirmation that the simulation should exit (quit).
    bool confirmExit();
    
    //////////////////////////////////////////////////////////////////
    /// Override of AbstractWormsSimUIStrategy Template Method:
    /// Returns the simulation with which the CursesWormsSimUIStrategy
    /// was created.
    WormsSim &getCurrentSim() { return m_sim; }
    
    //////////////////////////////////////////////////////////////////
    /// Override of AbstractWormsSimUIStrategy Template Method:
    /// Call his function to perform user interface specific logic
    /// including handling of any user input that may have been input
    /// since the last call to this function.  Regardless of user
    /// input, this function does not return for at least getSlowness()
    /// number of delayQuantum time intervals have elapsed since the
    /// call was made.
    bool processUserInput();

    //////////////////////////////////////////////////////////////////
    /// Override of AbstractWormsSimUIStrategy Template Method:
    /// Call his function to request a redraw of the user interface
    /// using a user interface specific mechanism. Calling this
    /// function may result in calls to the strategy's sim to obtain
    /// information about what to display.
    void redrawDisplay();
};

#endif // CURSESDRAWWORMSGAMEDECORATOR_H