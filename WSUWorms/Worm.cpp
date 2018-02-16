#include "Worm.h"
#include "WormsSim.h"
#include <algorithm>
#include <cassert>

//////////////////////////////////////////////////////////////////////
//                          PUBLIC METHODS
//////////////////////////////////////////////////////////////////////

// See documentation in header
Worm::Worm(
    UniqueWormType typeInfo,
    std::string saying,
    int posX,
    int posY,
    WormsSim &sim)
{
    assert(nullptr != typeInfo);
    assert(posX >= 0 && posX < sim.getWidth());
    assert(posY >= 0 && posY < sim.getHeight());
    
    m_typeInfo = typeInfo;
    int count_pre = m_typeInfo->count; // Needed only for post condition
    m_typeInfo->count += 1;
    m_direction = NORTH;
    
    // Store the saying reversed so first caharcter in saying will be
    // the one carried by the worm's head which is the last segment.
    std::reverse(saying.begin(), saying.end());
    
    // cause creation of "eraser" segment at index 0 by
    // prepending a ' ' character to saying
    saying = " " + saying;
    
    for(auto c : saying)
    {
        m_body.push_back(segment(posX, posY, c));
    }
    
    m_stomach = (int)m_body.size() * m_typeInfo->capacity;
    m_status = Worm::ALIVE;

    assert(1 < m_body.size());
    assert(nullptr != m_typeInfo);
    assert(Worm::ALIVE == m_status);
    assert(count_pre == (m_typeInfo->count - 1));
    assert(m_body[0].c == ' ');
    assert(areAllSegmentsContiguous(WormsSim::getSingletonSim()));
}

// See documentation in header
Worm::Worm(const Worm &original, int truncationIndex) :
        m_body(original.m_body.begin(),
        original.m_body.begin()+truncationIndex)
{
    assert(nullptr != original.m_typeInfo);
    assert(truncationIndex <= (int)original.m_body.size());
    assert(1 < truncationIndex);
    
    m_typeInfo = original.m_typeInfo;
    int count_pre = m_typeInfo->count; // Needed only for post condition
    m_typeInfo->count += 1;
    m_direction = original.m_direction;
    m_stomach = original.m_stomach * (int)m_body.size() /
        (int)original.m_body.size();
    m_status = original.m_status;

    assert(original.m_stomach >= m_stomach);
    assert(original.m_status == m_status);
    assert(truncationIndex == m_body.size());
    assert(nullptr != m_typeInfo);
    assert(count_pre == (m_typeInfo->count - 1));
    assert(m_body[0].c == ' ');
    assert(areAllSegmentsContiguous(WormsSim::getSingletonSim()));
}


// See documentation in header
void Worm::live(WormsSim &sim)
{
    assert(!isAlive() || nullptr != m_typeInfo);
    assert(!isAlive() || 1 < m_body.size());
    assert(areAllSegmentsContiguous(WormsSim::getSingletonSim()));
    
    /// The number of directions from one board position to any
    /// adjacent board position
    static const int numberDirections = 8;
    
    /// dx changes, indexed by DIRECTION; (+0 just to line up)
    static const int dxa[numberDirections] = {
        +0, +1, +1, +1, +0, -1, -1, -1 };
    
    /// dy changes, indexed by DIRECTION; (+0 just to line up)
    static const int dya[numberDirections] = {
        -1, -1, +0, +1, +1, +1, +0, -1 };

    // The number of possible direction indexes to choose
    static const int distributionOfSelectableDirections = 16;
    
    /// Stores DIRECTIONs, a.k.a. indexes into dxa and dya and
    /// indirectly controls frequency and direction of turns made by
    /// the head because because head movement directions are
    /// selected pseudo randomly from this array.
    static const int nextTurn[distributionOfSelectableDirections] = {
        0, 0, 0, 0,  0, 0, 0, 0,
        1, 1, 1, 7,  7, 7, 2, 6
    };

    if (!isAlive())
    {   // !!!! EARLY EXIT !!!!
        assert(m_body[0].c == ' ');
        assert(areAllSegmentsContiguous(WormsSim::getSingletonSim()));
        return;
    }

    // Make each body segment move to the position of the next segment
    auto headIt = getBody().end() - 1;
    for(auto i = m_body.begin(); i < headIt; ++i)
    {
        i->x = (i + 1)->x;
        i->y = (i + 1)->y;
    }

    // Pick a movement direction relative to the current direction
    // from the selectable directions
    const int dir = (m_direction + nextTurn[WormsSim::getRandomModX(
        distributionOfSelectableDirections)]) % numberDirections;
    
    m_direction = static_cast<Worm::direction>(dir);
    
    // Move head in chosen direction
    getHead().y += dya[dir];
    getHead().x += dxa[dir];

    // Wrap around edges of board
    if (getHead().y < 0) getHead().y = sim.getHeight() - 1;
    else if (getHead().y >= sim.getHeight()) getHead().y = 0;
    
    if (getHead().x < 0) getHead().x = sim.getWidth() - 1;
    else if (getHead().x >= sim.getWidth()) getHead().x = 0;
    
    // Limit amount of food in stomach to capacity of body segments
    m_stomach = std::min((int)getBody().size() * m_typeInfo->capacity,
        m_stomach);
    
    // Consume some food
    m_stomach -= 1;
    if(isHungry())
    {
        m_typeInfo->eatFunction(*this, sim);
    }

    updateStatusBasedOnStomach();
    
    assert(m_body[0].c == ' ');
    assert(areAllSegmentsContiguous(WormsSim::getSingletonSim()));
}

// See documentation in header
void Worm::onWasSlicedAtSegmentIndex( int victimSegmentNumber)
{
    assert(1 < m_body.size());
    assert(victimSegmentNumber < m_body.size());
    assert(m_body[0].c == ' ');
    assert(areAllSegmentsContiguous(WormsSim::getSingletonSim()));
    
    auto tsegs = m_body.size(); // size before slice

    m_body = std::vector<segment>(
        m_body.begin() + victimSegmentNumber,
        m_body.end());
    m_body[0].c = ' '; // Set new tail sentinel
    
    m_stomach = m_stomach * (int)m_body.size() / (int)tsegs;
    updateStatusBasedOnStomach();
    
    assert(m_body.size() == (tsegs - victimSegmentNumber));
    assert(m_body[0].c == ' ');
    assert(areAllSegmentsContiguous(WormsSim::getSingletonSim()));
}

// See documentation in header
void Worm::onWasEaten()
{
    assert(1 < m_body.size());
    assert(nullptr != m_typeInfo);
    assert(m_status == ALIVE); // only alive worms are eaten
    assert(0 < m_typeInfo->count);

    m_status = EATEN;
    m_typeInfo->count -= 1;

    assert(m_body[0].c == ' ');
    assert(0 <= m_typeInfo->count);
    assert(areAllSegmentsContiguous(WormsSim::getSingletonSim()));
}

// See documentation in header
int Worm::getNumVegetarians()
{
    return Worm::vegetarianInfo.count;
}

// See documentation in header
int Worm::getNumCanibals()
{
    return Worm::cannibalInfo.count;
}

// See documentation in header
int Worm::getNumScissorheads()
{
    return Worm::scissorInfo.count;
}

// See documentation in header
void Worm::resetWormCounters()
{
    Worm::vegetarianInfo.count = 0;
    Worm::scissorInfo.count = 0;
    Worm::cannibalInfo.count = 0;
    
    assert(0 == Worm::getNumVegetarians());
    assert(0 == Worm::getNumCanibals());
    assert(0 == Worm::getNumScissorheads());
}

//////////////////////////////////////////////////////////////////////
//                          PRIVATE METHODS
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
/// A Worm is hungry if its stomach foof content value at least 25%
/// less that the worm's stomach capacity.
/// Returns true if the worm is alive and hungry and false otherwise
bool Worm::isHungry() const
/*   */
{
    assert(1 < m_body.size());
    assert(nullptr != m_typeInfo);
    
    int m = m_stomach;
    int n = (int)getBody().size() * m_typeInfo->capacity;

    bool result = (m_status == ALIVE && 4 * m < 3 * n);
    
    assert((result != 0) ?
        (isAlive() && 4 * m < 3 * n) :
        (!isAlive() || 4 * m >= 3 * n));

    return result;
}

//////////////////////////////////////////////////////////////////////
/// Returns the neutritional food value of the worm. This is the amount
/// of food another worm may obtain by eating this worm.
/// Returns a value proportional to the body size of the worm.
int Worm::getFoodValue() const {
    assert(1 < m_body.size());
    assert(m_body[0].c == ' ');

    return (int)m_body.size() * m_typeInfo->foodValue;
}

//////////////////////////////////////////////////////////////////////
/// Changes the worm's status based on the size of the worm and the
/// amount of food in the worm's stomach. As a side effect, if the
/// function changes the worm status to DEAD, the function also
/// decrements the count of the number of living instances that have
/// newly deceased worm's type.
void Worm::updateStatusBasedOnStomach()
{
    assert(0 < m_body.size());
    assert(nullptr != m_typeInfo);
    
    if (isAlive() && (m_stomach <= 0 || 1 == m_body.size()))
    {
        assert(0 < m_typeInfo->count);
        m_status = DEAD;
        m_typeInfo->count -= 1;
        assert(0 <= m_typeInfo->count);
    }
    
    assert((1 < m_body.size()) || (m_status == DEAD));
    assert((0 < m_stomach) || (m_status == DEAD));
    assert(areAllSegmentsContiguous(WormsSim::getSingletonSim()));
}

//////////////////////////////////////////////////////////////////////
/// Returns the index of one the worm's segments of one or more of
/// the worm's segments other than segment index 0 occupies the board
/// position, {x, y}. Returns 0 otherwise.
int Worm::segmentIndexAt(int x, int y) const
{
    assert(1 < m_body.size());
    assert(m_body[0].c == ' ');
    
    for(auto i = 1; i < m_body.size(); ++i)
    {
       if(m_body[i].x == x && m_body[i].y == y)
       {   // NOTE: !!!! EARLY RETURN !!!!
           return (int)i;
       }
    }
    
    return 0;
}

//////////////////////////////////////////////////////////////////////
/// This function implements eating logic for Vegitarian type worms
void Worm::Info::VegetarianEat(Worm &worm, WormsSim &sim)
{
    if(sim.tryToEatCarrotAt(
        worm.getHead().x, worm.getHead().y))
    {
        worm.m_stomach += foodValueOfCarrot;
    }
}

//////////////////////////////////////////////////////////////////////
/// This function implements eating logic for ScissorHead type worms
void Worm::Info::ScissorEat(Worm &worm, WormsSim &sim)
{
    sim.sliceVictimForWorm(worm);
    Worm::Info::VegetarianEat(worm, sim);
}

//////////////////////////////////////////////////////////////////////
/// This function implements eating logic for Cannibal type worms
void Worm::Info::CannibalEat(Worm &worm, WormsSim &sim)
{
    int foodValue = sim.eatVictimForWorm(worm);
    worm.m_stomach += foodValue;
    Worm::Info::VegetarianEat(worm, sim);
}

//////////////////////////////////////////////////////////////////////
/// Defines the information that distinguishes Vegetarian worms from
/// other types of worms
Worm::Info Worm::vegetarianInfo = {
    Worm::Info::VegetarianEat,
    1, 3, 3
};

//////////////////////////////////////////////////////////////////////
/// Defines the information that distinguishes ScissorHead worms from
/// other types of worms
Worm::Info Worm::scissorInfo = {
    Worm::Info::ScissorEat,
    2, 4, 5
};

//////////////////////////////////////////////////////////////////////
/// Defines the information that distinguishes Cannibal worms from
/// other types of worms
Worm::Info Worm::cannibalInfo = {
    Worm::Info::CannibalEat,
    3, 5, 4
};

//////////////////////////////////////////////////////////////////////
/// An immutable collection of all available worm types
const std::vector<Worm::UniqueWormType> Worm::UniqueWormTypes = {
    &Worm::vegetarianInfo,
    &Worm::scissorInfo,
    &Worm::cannibalInfo,
};

//////////////////////////////////////////////////////////////////////
/// This function should only be used to test pre and post conditions
/// of other functions.
bool Worm::areAllSegmentsContiguous(const WormsSim &sim) const
{
    for (auto it = m_body.rbegin()+1; it != m_body.rend(); ++it)
    {
        int deltaX = std::abs(it->x - (it-1)->x);
        int deltaY = std::abs(it->y - (it-1)->y);
        
        if((1 < deltaX && deltaX < (sim.getWidth() - 1)) ||
            (1 < deltaY && deltaY < (sim.getHeight() - 1)))
        {
            assert(0);
            return false;
        }
    }
    
    return true;
}
