#include "CursesWormsSimUIStrategy.h"
#include <ncurses.h>
#include <unistd.h>   // For usleep()
#include <algorithm>

//////////////////////////////////////////////////////////////////////
//                          PUBLIC METHODS
//////////////////////////////////////////////////////////////////////

//!!!!! NOTE: Pre and post conditions are not provided here due to
// instructor guidance not to provide pre and post conditions related
// to Curses based display.

// See documentation in header
bool CursesWormsSimUIStrategy::confirmExit()
{
    static const size_t maxMessageLen = 1000; //< Arbitrary large
    
    char msg[maxMessageLen];
    snprintf(msg, maxMessageLen,
        "press WormsSim::esc to terminate, or any other key to "
        "re-run" "\n" "\n" "\n" "\n");
    showMessage(msg, m_sim.getHeight() + 1);
    
    return CursesWormsSimUIStrategy::esc == getOneChar();
}

// See documentation in header
bool CursesWormsSimUIStrategy::processUserInput()
{
    static const int numMicrosecondsInAMillisecond = 1000;
    
    showStatus();
    
    for (delayRemaining = slowness+1;
        delayRemaining > 0;
        delayRemaining -= delayQuantum)
    {
        char c = handleUserKeyPress(getch()); // no-delay
        if (c == esc)
        {    // !!!! NOTE EARLY RETURN !!!!
             return true;
        }
        if (isPaused)
        {
            delayRemaining += delayQuantum;
            showStatus();
        }
        if (delayQuantum > 0)
        {
            usleep(numMicrosecondsInAMillisecond * delayQuantum);
        }
    }
    
    return false;
}

// See documentation in header
void CursesWormsSimUIStrategy::showMessage(std::string text, int y)
{
    move(y, 0);
    addstr(text.c_str());
    refresh();
}

// See documentation in header
void CursesWormsSimUIStrategy::redrawDisplay()
{
    for(int y = 0; y < m_sim.getHeight(); ++y)
    {
        move(y, 0);
        for(int x = 0; x < m_sim.getWidth(); ++x)
        {
            char onec = m_sim.getOnecAt(x, y);
            int attr = m_sim.getAttrAt(x, y);
            addch(onec | wormAttr[attr]);
        }
    }
    refresh();
}

// See documentation in header
void CursesWormsSimUIStrategy::initializeForDisplay(
    int &out_width, int &out_height)
{
    startCurses(out_height, out_width);
    
    // Reserve some rows for a message display area
    out_height -= rowsInMessageArea;
}

// See documentation in header
void CursesWormsSimUIStrategy::releaseDisplay()
{
    endCurses();
}


//////////////////////////////////////////////////////////////////////
//                          PRIVATE METHODS
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
int CursesWormsSimUIStrategy::slowness = 10;     //< See documentation in header
int CursesWormsSimUIStrategy::delayQuantum = 10; //< See documentation in header
int CursesWormsSimUIStrategy::delayRemaining;    //< See documentation in header
bool CursesWormsSimUIStrategy::isPaused(false); //< See documentation in header

//////////////////////////////////////////////////////////////////////
/// Call to perform user interface specific shutdown / release of the
/// display being used by the strategy.
void CursesWormsSimUIStrategy::endCurses()
{
    if (!isendwin())
        endwin();
}

//////////////////////////////////////////////////////////////////////
void CursesWormsSimUIStrategy::startCurses(int &asogRows, int &asogCols)
{
    initscr();            // ncurses init
    cbreak();         // unbuffered getch
    noecho();         // no echoing of keys pressed
    nodelay(stdscr, TRUE);    // get a char *if* available
    atexit(endCurses);
    start_color();
    use_default_colors();
    init_pair(1, COLOR_GREEN, -1);
    init_pair(2, COLOR_RED, -1);
    init_pair(3, COLOR_BLACK, COLOR_WHITE);
    init_pair(4, COLOR_YELLOW, -1);
    getmaxyx(stdscr, asogRows, asogCols);
}

//////////////////////////////////////////////////////////////////////
/// Returns one available character of user input. This function
/// blocks until at least one character is available.
int CursesWormsSimUIStrategy::getOneChar()
{
    nodelay(stdscr, FALSE);   // wait until a char is typed
    int c =  getch();
    nodelay(stdscr, TRUE);
    return c;
}

// See documentation in header
void CursesWormsSimUIStrategy::showStatus()
{
    static const size_t maxMessageLen = 1000;  //< Arbitrary large
    
    char msg[maxMessageLen];
    snprintf
    (msg, maxMessageLen,
         "SPC %s, ESC terminates, k kills-, w creates-, s "
         "shows-a-worm\n%2d Vegetarians,%2d Cannibals,%2d "
         "Scissor-heads,%2d hi-water-mark\n"
         "%04d slowness, - increases, + reduces, f full-speed\n\n\n",
         (isPaused ? "resumes " : "pauses "),
         Worm::getNumVegetarians(),
         Worm::getNumCanibals(),
         Worm::getNumScissorheads(),
         m_sim.getHighWaterMark(),
         getSlowness());
    
     showMessage(msg, m_sim.getHeight() + 1);
}

//////////////////////////////////////////////////////////////////////
/// Perform user interface specific logic in response to user input
/// of the character, c.
char CursesWormsSimUIStrategy::handleUserKeyPress(char c)
{
    switch (c) {
        case '+':
        {
            slowness -= std::min(slowness, 100);   //< Arbitrary
            break;
        }
        case '-':
        {
            slowness += 100;  //< Arbitrary
            break;
        }
        case 'f':
        {
            slowness = 0; // Let simulation run at maximum speed
            break;
        }
        case 's':
        {
            /* TODO: highlight a worm */
            break;
        }
        case 'k':
        {
            /* TODO: kill the highlighted worm */
            break;
        }
        case 'w':
        {
            m_sim.createWorm();
            break;
        }
        case ' ':
        {
            isPaused ^= 1;
            break;
        }
        default:
        {  // Intentionally blank
            break;
        }
    }
    return c;
}

//////////////////////////////////////////////////////////////////////
/// A map from integer attribute IDs stored by the simulation to
/// corresponding display attributes in the user interface.
std::map<int, int> CursesWormsSimUIStrategy::wormAttr = {
    {0, COLOR_PAIR(4)},
    {1, COLOR_PAIR(1)|A_STANDOUT},
    {2, COLOR_PAIR(2)|A_STANDOUT},
    {3, COLOR_PAIR(3)|A_STANDOUT},
    {4, COLOR_PAIR(4)},
};
