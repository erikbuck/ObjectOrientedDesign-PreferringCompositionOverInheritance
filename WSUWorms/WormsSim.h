#ifndef WORMSGAME_H // Guard
#define WORMSGAME_H

#include <array>
#include <random>
#include <random>
#include <cassert>
#include "Worm.h"

class AbstractWormsSimUIStrategy;


//////////////////////////////////////////////////////////////////////
/// WormSim encapsulates the board and zero or more Worm instances.
/// WormSim provides a runSimulation() function that when called starts
/// or restarts a simulation with a pseudo random number of initial
/// worms and a board containing a carrot in every square. Note: more
/// worms than the initial number may come into existence while the
/// simulation is running. Note: although simulation state is
/// encapsulated by an instance of WormSim, the WormSim functions,
/// runSimulation(), and createWorm(), must be called periodically to
/// modify simulation state over time. Additionally, the WormSim
/// functions, tryToEatCarrotAt(), sliceVictimForWorm(), and
/// eatVictimForWorm(), are called by instances of Worm.
///
/// Design Notes:
/// - Following the principle of separation of concerns, the WormSim
/// class intentionally does not have any "drawing" or user input
/// dependencies. It is anticipated that many different strategies for
/// "drawing" a simulation board and the worms may eventually be
/// implemented. Similarly, many different mechanisms for user input
/// may eventually be implemented.  WormSim is responsible for
/// encapsulating board state irrespective and decoupled from drawing
/// and user input implemented elsewhere. See Section Use of the
/// Strategy Design Pattern for more information about the decoupling.
/// - Composition ("has-a") relationships are preferred over
/// inheritance ("is-a") relationships between classes. For example,
/// each WormsSim instance has an arbitrary number of Worm instances
/// and has a board. WormsSim has no “virtual” functions suitable for
/// inheritance based polymorphic dispatch.
///
//////////////////////////////////////////////////////////////////////
class WormsSim
{
private:
    static std::random_device  rdev; //< C++11 default pseudo random number device
    static std::default_random_engine random_engine; //< C++11 default pseudo random number generator
    
    static const int max_board_width = 100;  ///< Arbitrary value
    static const int max_board_height = 100; ///< Arbitrary value
    
    /// Stores an arbitrary number of "sayings" which are used when
    /// creating Worm instances.
    static const std::vector<std::string> sayings;
    
    /// Arbitrary "display" attributes for each board square.
    /// Attributes can be any information as long as the information is
    /// encoded as a single signed integer. e.g. an attribute might be
    /// a key used to look-up graphical properties in a separately
    /// maintained dictionary, or an attribute might encode RGB
    /// components of a color.
    /// default_square_attr stores the default value used to initialize
    /// each board square.
    static const int default_square_attr = 0;

    /// Arbitrary "display" attributes for dead worm segments.
    /// Attributes can be any information as long as the information is
    /// encoded as a single signed integer. e.g. an attribute might be
    /// a key used to look-up graphical properties in a separately
    /// maintained dictionary, or an attribute might encode RGB
    /// components of a color.
    static const int dead_attribute = 4;
    
    static const char carrot = '.'; ///< a carrot indicator char
    
    /// A "board" is a 2D array of square instances.
    struct square
    {
        char onec;         //< encodes contents of square
        int attr;          //< Arbitrary encoded attributes
        
        square() : onec(WormsSim::carrot), attr(default_square_attr) {}
        square(char c, int a) : onec(c), attr(a) {}
    };
    
    /// The type of a single column in the simulation's board
    typedef std::array<square, max_board_height> board_column;
    
    /// The type of the simulation's board
    typedef std::array<board_column, max_board_width> board;
    
    /// An arbitrary number of worms in the simulation
    std::vector<Worm> m_worms;
    
    /// The maximum number of worms that have ever been in the
    /// simulation simultaneously (per process invocation)
    std::vector<Worm>::size_type m_high_water_mark;
    
    /// A board used to store non-moving simulation elements i.e.
    /// carrots.
    board m_passive_board;
    
    /// A board used to store information to be directly presented
    /// to users
    board m_screen_board;
    
    /// The actual width of the simulation's boards
    int m_actual_board_width;

    /// The actual hight of the simulation's boards
    int m_actual_board_height;

    /// Use this function to find an index where a non-living Worm can
    /// be replaced by a new living worm. This approach helps to avoid
    /// storing lots and lots of non-living Worms in m_worms. Returns
    /// the first index within m_worms that contains a non living
    /// worm. Returns m_worms.size() if no suitable index is found.
    std::vector<Worm>::size_type findSlot() const;

    /// This function should be called any time a Worm in m_worms changes
    /// state e.g the worm has moved, been eaten, or has died.
    void updateBoardWithWorm(
        const Worm &a_worm //< worm that has changed state
    );

    // See description in implementation file
    void updateBoardWithWormsAndCarrots();

    // See description in implementation file
    void makeAllWormsLive();

    // See description in implementation file
    bool runSimulationStep(AbstractWormsSimUIStrategy &uiStrategy);

    // See description in implementation file
    void sprinkleCarrots();

    // See description in implementation file
    void setPassiveSquareAt(square s, int x, int y);

    // See description in implementation file
    square getPassiveSquareAt(int x, int y) const { return m_passive_board[x][y]; }

    // See description in implementation file
    square getScreenSquareAt(int x, int y) const { return m_screen_board[x][y]; }

    // See description in implementation file
    Worm &getVictimWorm(
        Worm &in_worm,
        int &out_SegementNumber);

    // See description in implementation file
    void sliceVictim(Worm &victim, int victimSegementNumber);
    
    /// Constructs a WormsSim instance encapsulating a board with
    /// width columns and height rows of squares. If the width or
    /// height specified exceeds the values of getMaxBoardWidth() and
    /// getMaxBoardHeight() respectively, the maximum values are used.
    /// Newly created WormsSim instances have a pseudo random number
    /// of living Worm instances of pseudo random types and a board
    /// containing a carrot at every position.
    WormsSim(
        int width, //< The width of the 2D array of board squares
        int height //< The height of the 2D array of board squares
    );
    
public:
    //////////////////////////////////////////////////////////////////
    /// This function creates an instance of WormsSim with the
    /// specified board width and height.
    /// If the width or height specified exceeds
    /// the values of getMaxBoardWidth() and getMaxBoardHeight()
    /// respectively, the maximum values are used.
    /// There can only be one instance of WormsSim at a time. If this
    /// function is called when a WormsSim instance containing a board
    /// with the specified dimensions already exists, a reference to
    /// the pre-existing WormsSim instance is returned. Otherwise, a
    /// new WormsSim instance is created that replaces any
    /// pre-existing WormsSim instance, and a reference to the new
    /// WormsSim instance is returned.
    /// Newly created WormsSim instances have a pseudo random number
    /// of living Worm instances of pseudo random types.
    /// As a side effect, this function resents the counters of live
    /// worm instances of each type prior to creating new worm
    /// instances that increase the counts.
    static WormsSim &initSingletonSim(
        int width, //< The width of the 2D array of board squares
        int height //< The height of the 2D array of board squares
    );

    //////////////////////////////////////////////////////////////////
    /// This function should only be used for pre and post condition
    /// assertion checking
    static WormsSim &getSingletonSim();
    
    //////////////////////////////////////////////////////////////////
    /// Utility function to return a pseudo random natural number in
    /// the range 0..(x-1).
    static unsigned int getRandomModX(
        unsigned int x); //< must be > 1
    
    //////////////////////////////////////////////////////////////////
    /// Returns the maximum number of rows of squares in a "board"
    static int getMaxBoardHeight() {return max_board_height; }
    
    //////////////////////////////////////////////////////////////////
    /// Returns the maximum number of columns of squares in a "board"
    static int getMaxBoardWidth() {return max_board_width; }

    /// @name Non-mutating Accessors
    /// @{
    const std::vector<Worm> &getWorms() const { return m_worms; }
    int getWidth() const { return m_actual_board_width; }
    int getHeight() const { return m_actual_board_height; }
    int getHighWaterMark() const { return (int)m_high_water_mark; }
    const char getOnecAt(int x, int y) const {
        assert(x >= 0 && x < getWidth() && y >= 0 && y < getHeight());
        return m_screen_board[x][y].onec;
    }
    const char getAttrAt(int x, int y) const { return m_screen_board[x][y].attr; }
    /// @}
    
    /// @name Functions that mutate the simulation
    /// @{
    
    //////////////////////////////////////////////////////////////////
    /// Each time this function is called, the simulation restarts
    /// from initial conditions with a pseudo random number of
    /// generated worms and a board containing a carrot at every
    /// position. This function indirectly calls
    /// uiStrategy.processUserInput(), and once this function is
    /// called, the simulation runs until uiStrategy.processUserInput()
    /// returns true. A strategy is able to implement a user interface
    /// that accepts user input. The strategy can signal to the
    /// WormsSim that the current simulation run should end by
    /// returning true from the strategy's implementation of
    /// uiStrategy.processUserInput().
    /// Note: more worms than the initial number may come into
    /// existence while the simulation is running.
    void runSimulation(
        /// The UI strategy that can be used by the running
        /// simulation to display simulation state to users and
        /// accept user input that controls the simulation.
        AbstractWormsSimUIStrategy &uiStrategy
    );

    // Adds a new worm head of a random type of worm at a random
    // position in the simulation's board. As a worm head moves, its
    // following body segments are added to the board automatically.
    void createWorm();

    // This function removes any carrot at x,y, from the simulation.
    // Returns true IFF a carrot was removed by this function.
    bool tryToEatCarrotAt(int x, int y);
    
    // If a victim worm segment is at the same location as worm's head,
    // the victim worm is split by removing the victim worm segment
    // at the same location as worm's head.
    //
    // After being split, the lower portion of the victim (segments
    // after the removed segment) may form a new worm by transforming
    // the segment after the removed segment into a new head.
    // Note: Worms do not split themselves.
    // Note: At most one worm is split per call to this function.
    // Note: Any new worm created by the split is the same type as the
    // victim worm.
    void sliceVictimForWorm(Worm &worm);
    
    // If a victim worm segment is at the same location as worm's head,
    // the victim worm is eaten, and this function returns the food
    // value of the number of segments in the eaten worm or 0
    // if no worm was eaten.
    // Note: Worms do not eat themselves.
    // Note: At most one worm is eaten per call to this function.
    int eatVictimForWorm(Worm &worm);

    /// @}
};


//////////////////////////////////////////////////////////////////////
/// The abstract AbstractWormsSimUIStrategy class (similar to a Java
/// or C# Interface or an Objective-C Protocol) declares the functions
/// needed by any class that displays WormsSim state to a user and/or
/// accepts user input. AbstractWormsSimUIStrategy collaborates as
/// part of an implementation of the Strategy Design Pattern:
/// https://en.wikipedia.org/wiki/Strategy_pattern
///The assumption is that many different concrete strategies for
/// displaying WormsSim state to a users and accepting user input may
/// exist. For example, there may be a terminal (Curses) based textual
/// display or a 2D vector graphics display or a 3D display. Similarly,
/// user input may come from a keyboard or over a network connection
/// or from a data file or from script.
class AbstractWormsSimUIStrategy
{
protected:
    virtual ~AbstractWormsSimUIStrategy() {} // C++ best practice
public:
    //////////////////////////////////////////////////////////////////
    /// Template Method design pattern: Subclasses must override this
    /// method to provide user a interface specific implementation.
    /// Returns the simulation being used with the UIStrategy to
    /// supply information for display in the user interface.
    virtual WormsSim &getCurrentSim() = 0; //< Template Method design pattern

    //////////////////////////////////////////////////////////////////
    /// Template Method design pattern: Subclasses must override this
    /// method to provide user a interface specific implementation.
    ///
    /// WormsSim calls this function to perform user interface specific
    /// logic including handling of any user input that may have been
    /// input since the last call to this function.
    virtual bool processUserInput() = 0;   //< Template Method design pattern

    //////////////////////////////////////////////////////////////////
    /// Template Method design pattern: Subclasses must override this
    /// method to provide user a interface specific implementation.
    ///
    /// WormsSim calls this function to request a redraw of the user
    /// interface using a user interface specific mechanism. Calling
    //  this function is expected to result in calls back to the
    /// strategy's sim to obtain information about what to display.
    virtual void redrawDisplay() = 0;      //< Template Method design pattern
};

#endif // WORMSGAME_H
