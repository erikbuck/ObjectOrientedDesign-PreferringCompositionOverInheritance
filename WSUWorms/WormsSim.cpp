#include "WormsSim.h"
#include <algorithm>
#include <memory>

//////////////////////////////////////////////////////////////////////
// One instance of std::default_random_engine for use anywhere
// a random number in needed.
// The initial seed for random number generation vary from run to run
// and is produced by calling rdev() which is the C++11 preferred
// mechanism vs. traditional approaches to this seeding subproblem
// that involve the use of the system clock (e.g., via time(0))
std::random_device  WormsSim::rdev{};
std::default_random_engine WormsSim::random_engine{rdev()};

//////////////////////////////////////////////////////////////////////
/// This variable exists to enable pre and post condition assertion
/// checking in unrelated modules. If not for the need to check
/// pre and post conditions, this variable would be an implementation
/// detail of the WormsSim::initSingletonSim() function.
static std::unique_ptr<WormsSim> instancePtr(nullptr);

//////////////////////////////////////////////////////////////////////
//                          PUBLIC METHODS
//////////////////////////////////////////////////////////////////////

// See documentation in header
WormsSim &WormsSim::initSingletonSim(
    int width, //< The width of the 2D array of board squares
    int height //< The height of the 2D array of board squares
)
{
    const WormsSim *instancePtr_pre = instancePtr.get(); // used only for post condition checking
    
    Worm::resetWormCounters();
    
    assert(0 == Worm::getNumCanibals() );
    assert(0 == Worm::getNumVegetarians() );
    assert(0 == Worm::getNumScissorheads() );

    if(nullptr == instancePtr ||
        instancePtr->getWidth() != width ||
        instancePtr->getHeight() != height)
    {
        instancePtr = std::unique_ptr<WormsSim>(
            new WormsSim(width, height));
    }

    assert(nullptr != instancePtr);
    assert(nullptr == instancePtr_pre ||
        (instancePtr_pre == instancePtr.get() &&
         instancePtr->getWidth() == width &&
         instancePtr->getHeight() == height) ||
         instancePtr_pre != instancePtr.get());
    
    return *instancePtr;
}

// See documentation in header
WormsSim &WormsSim::getSingletonSim()
{
    assert(nullptr != instancePtr);
    
    return *instancePtr;
}

// See documentation in header
WormsSim::WormsSim(int width, int height) :
    m_high_water_mark(0)
{
    m_actual_board_width = std::max(1, std::min(width, getMaxBoardWidth()));
    m_actual_board_height = std::max(1, std::min(height, getMaxBoardHeight()));
    
    assert(m_actual_board_width <= getMaxBoardWidth());
    assert(m_actual_board_height <= getMaxBoardHeight());
}

// See description in header
unsigned int WormsSim::getRandomModX(unsigned int x)
{
    assert(x > 1);
    
    // random_distribution's () operator will return any integer in the
    // inclusive range 0 to the larges possible unsigned integer.
    static std::uniform_int_distribution<int> random_distribution(
        0, std::numeric_limits<unsigned int>::max());

    // No output assertion is needed because of the definition of the
    // C++ modulus, '%', operator
    return random_distribution(random_engine) % x;
}

// See description in header
void WormsSim::runSimulation(
    AbstractWormsSimUIStrategy &uiStrategy)
{
    static const int minimumNumberOfWorms = 3;
    static const int variationInNumberOfWoms = 6;
    int numWorms = minimumNumberOfWorms + getRandomModX(
        variationInNumberOfWoms);
    
    sprinkleCarrots();
    m_worms.clear();
    
    for (int i = numWorms; i > 0; i--) { createWorm(); }
    
    do { } while(!runSimulationStep(uiStrategy));
}

// See description in header
void WormsSim::createWorm()
{
    int typeIndex = WormsSim::getRandomModX(
        (int)Worm::UniqueWormTypes.size());
    
    std::string aSaying(sayings[WormsSim::getRandomModX(
                    (int)sayings.size())]);
    int yy = WormsSim::getRandomModX(getHeight());
    int xx = WormsSim::getRandomModX(getWidth());
    
    Worm::UniqueWormType type(Worm::UniqueWormTypes[typeIndex]);
    Worm newWorm(type, aSaying, xx, yy, *this);
    auto index = findSlot();
    if(index < m_worms.size())
    {
        m_worms[index] = newWorm;
    }
    else
    {
        m_worms.push_back(newWorm);
    }

    m_high_water_mark = std::max(m_worms.size(), m_high_water_mark);
}

// See description in header
void WormsSim::sliceVictimForWorm(Worm &worm)
{
    int victimSegNum(0);
    Worm &victim (getVictimWorm(worm, victimSegNum));
    
    if (0 < victimSegNum)
    {
        sliceVictim(victim, victimSegNum);
    }
}

// See description in header
int WormsSim::eatVictimForWorm(Worm &worm)
{
    int result = 0;
    int victimSegNum(0);
    Worm &victim(getVictimWorm(worm, victimSegNum));
    
    assert(0 == victimSegNum || victim.isAlive());
    
    if (0 < victimSegNum && victim.isAlive())
    {
        result = victim.getFoodValue();
        victim.onWasEaten();
    }
    
    return result;
}


//////////////////////////////////////////////////////////////////////
//                          PRIVATE METHODS
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
/// Each string in sayings is potentially used as a source of
/// characters stored in the segments of a worm.
const std::vector<std::string> WormsSim::sayings = {
"*do-not-optimize-too-soon#",
"*If it does not have to be correct, making the program efficient is easy.#",
"*90% of code executes only 10% of the time!#",
"*Is there a program, longer than sayings 1000 lines, that is correct?#",
"*OS is not bug-free, the compiler is not bug-free, so is my program!#",
"*ABCDEFGHIJKLMNOPQRSTUVWXYZ#",
"*98765432109876543210#",
"*Edsger-Dijkstra#",     // legendary programmers ...
"*C-A-R-Hoare#",
"*Donald-Knuth#",
"*Richard-Stallman#",
"*Linux-Torvalds#",
};

// See description in header
std::vector<Worm>::size_type WormsSim::findSlot() const
{
    std::vector<Worm>::size_type i = 0;
    for(; i < m_worms.size(); ++i)
    {
        if(!m_worms[i].isAlive())
        {   // !!!! EARLY EXIT !!!!
            return i;
        }
    }
    
    return i;
}

//////////////////////////////////////////////////////////////////////
/// Set the "passive" value of the square at {x,y} in the "board".
/// Passive squares are squares that store information other than
/// living worms e.g. "carrots". A distinction is made between
/// passive squares and others so that worms segments and carrots
/// may occupy the same positions.
void WormsSim::setPassiveSquareAt(square s, int x, int y)
{
    m_passive_board[x][y] = s;
}

// See description in header
bool WormsSim::tryToEatCarrotAt(int x, int y)
{
    assert(x >= 0 && x < getWidth());
    assert(y >= 0 && y < getHeight());
    
    auto square = getPassiveSquareAt(x, y);
    if(carrot == square.onec)
    {
        square.onec = ' ';
        setPassiveSquareAt(square, x, y);
        return true;
    }
    
    assert(getPassiveSquareAt(x, y).onec != carrot);
    
    return false;
}

//////////////////////////////////////////////////////////////////////
/// Places a carrot in every passive board square
void WormsSim::sprinkleCarrots()
{
    m_passive_board = board(); // initializes to carrots at construction
}

//////////////////////////////////////////////////////////////////////
/// Set the attrs and characters in the simulation based on a_worm's
/// current position and status
void WormsSim::updateBoardWithWorm(const Worm &a_worm)
{
    if(a_worm.isAlive())
    {
        for(const Worm::segment &s : a_worm.getBody())
        {
            m_screen_board[s.getX()][s.getY()] = square(
                s.getC(), a_worm.getAttr());
        }
    }
    else
    {
        for(const Worm::segment &s : a_worm.getBody())
        {
            m_passive_board[s.getX()][s.getY()] = square(
                s.getC(), dead_attribute);
        }
    }
}

//////////////////////////////////////////////////////////////////////
/// Set the attrs and characters in the simulation based on the
/// positions of worms and carrots in the simulation
void WormsSim::updateBoardWithWormsAndCarrots()
{
    m_screen_board = m_passive_board; // copy the carrots and possibly dead worms
    
    for(auto w : m_worms) { updateBoardWithWorm(w); }
}

//////////////////////////////////////////////////////////////////////
/// Finds a worm other than in_worm that has a segment with index > 0
/// at the same position as in_worm's head.
/// If a suitable worm is found, this function sets out_SegementNumber
/// to the segment number of the found worm's segment at the same
/// position as in_worm's head and returns the found worm.
///
/// Otherwise, this function sets out_SegementNumber to 0 and returns
/// in_worm.
Worm &WormsSim::getVictimWorm(
    Worm &in_worm,           //< The worm whose head position is used
    int &out_SegementNumber  //< Set to 0 or the segment number of the
                             // victim's segment at in_worm's head's position
)
{
    Worm &result(in_worm);
    const Worm &const_inWorm(in_worm);
    for(Worm &candidate : m_worms)
    {
        const Worm &const_candidate(candidate);
        if( &const_candidate != &const_inWorm &&
           candidate.isAlive())
        {
           out_SegementNumber = candidate.segmentIndexAt(
               const_inWorm.getHead().getX(),
               const_inWorm.getHead().getY());
            
           if(0 != out_SegementNumber)
           {
               // !!!! Note: EARLY EXIT !!!!
               assert((out_SegementNumber == 0) ? (&result == &in_worm) : (&candidate != &in_worm));
               assert(candidate.isAlive());
               return candidate;
           }
        }
    }
    
    out_SegementNumber = 0;
    assert((out_SegementNumber == 0) ? (&result == &in_worm) : (&result != &in_worm));
    return result;
}

//////////////////////////////////////////////////////////////////////
/// If an existing slot in is available and victimSegementNumber is
/// > 1 and < the number of segments in victim, this function creates
/// a new worm containing copies of the tail segments up to but not
/// including the segment at victimSegementNumber in victim. The new
/// worm is stored in the available slot, and victim's
/// onWasSlicedAtSegmentIndex() function is called and may mutate
/// victim. If no new worm is created, this function returns without
/// modifying victim.
///
/// Note: Not creating a new worm and not slicing victim just because
/// no existing slot is available seems suspect, but it is consistent
/// with the behavior of the reference  pmateti@wright.edu sample code.
void WormsSim::sliceVictim(
    Worm &victim, //< the worm being sliced
    int victimSegementNumber) //< the segment index where victim is sliced (must be < the number of segments in victim)
{
    assert(victimSegementNumber < victim.getBody().size());
    
    auto availableIndex = findSlot();
    
    if(availableIndex < m_worms.size() &&
        1 < victimSegementNumber &&
        victimSegementNumber < victim.getBody().size())
    {
        m_worms[availableIndex] = Worm(victim, victimSegementNumber);
        victim.onWasSlicedAtSegmentIndex(victimSegementNumber);
    }
}

// See description in header
void WormsSim::makeAllWormsLive()
{
   WormsSim &sim = *this;
   std::for_each(m_worms.begin(), m_worms.end(),
       [&sim](Worm &worm) mutable { worm.live(sim); });
}

//////////////////////////////////////////////////////////////////////
/// This function executes one simulation step
bool WormsSim::runSimulationStep(AbstractWormsSimUIStrategy &uiStrategy)
{
    makeAllWormsLive();
    updateBoardWithWormsAndCarrots();
    uiStrategy.redrawDisplay();
    return uiStrategy.processUserInput();
}
