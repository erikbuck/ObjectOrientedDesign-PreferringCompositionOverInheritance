#ifndef WORM_H // Guard
#define WORM_H

#include <string>
#include <vector>
#include <map>
#include <cassert>


class WormsSim;


//////////////////////////////////////////////////////////////////////
/// This class encapsulates a "worm" which lives, eats,
/// moves, and eventually dies.
///
/// This class collaborates with the WormsSim class to interact with a
/// simulated area called a "board" that potentially contains other
/// worms and "carrots" that worms may eat. Worm instances may
/// interact with each other in several ways, but in every case, the
/// interaction is mediated by an instance of WormsSim i.e. The only
/// way a Worm instance can influence another worm instance or the
/// board is via the functions of WormsSim.
///
/// Design Notes:
/// - Following the principle of separation of concerns, this class
/// intentionally does not have any "drawing" dependencies. It is
/// anticipated that many different strategies for "drawing" a sim
/// worm may eventually be implemented. This class is responsible for
/// encapsulating worm state irrespective of drawing implemented
/// elsewhere.
/// - Composition ("has-a") relationships are preferred over
/// inheritance ("is-a") relationships between classes. For example,
/// each Worm has a reference to a UniqueWormType and an arbitrary
/// number of segments. An alternative design might provide an
/// abstract "Worm" class and several distinct subclasses to
/// encapsulate each different “kinds" of worm. However, when
/// replicating the functionality of pmateti@wright.edu's 2013
/// worms.cpp, the only aspects of a Worm that vary by “kind” may be
/// represented using composition (has-a relationships).  Note: A
/// private “inner” class accessible only within the Worm class
/// encapsulates all differences between worm types. The kind of a
/// worm is hidden from and irrelevant to code outside the
/// implementation of the Worm class. It could be argued that if
/// inheritance based polymorphism was used to enable worm kind
/// specialization, hypothetical future Worm subclasses might be
/// implemented as subclasses of Worm, but that is hardly the only or
/// best approach: Future "kinds" could be accommodated through
/// template specialization, the Visitor design pattern, the Command
/// design pattern, or the Adaptor design pattern all of which exist
/// to minimize coupling between code specifically by avoiding
/// unnecessary inheritance.
///
//////////////////////////////////////////////////////////////////////
class Worm
{
private:
    //////////////////////////////////////////////////////////////////
    /// Instances of this class encapsulate everything that is
    /// different form one _kind_ of Worm to another.
    class Info {
    private:
        friend class Worm;
        
        /// Type of function pointer used to implement worm eating
        typedef void (*EatFunction)(Worm &worm, WormsSim &sim);
        
        const EatFunction eatFunction; //< Implements worm type specific eating logic
        const int attr;       //< Arbitrary int (may be used key in a map)
        const int capacity;   //< Amount of food storable per worm segment
        const int foodValue;  //< nutritional value (food amount) of an eaten segment
        
        /// The number of references to this Info (Used for simulation wide statistics)
        int count;

        // Available predefined EatFunctions
        static void VegetarianEat(Worm &worm, WormsSim &sim);
        static void ScissorEat(Worm &worm, WormsSim &sim);
        static void CannibalEat(Worm &worm, WormsSim &sim);

        Info(EatFunction func, int someAttr, int aCapacity, int aFoodValue) :
            eatFunction(func), attr(someAttr), capacity(aCapacity),
            foodValue(aFoodValue), count(0)
        {}
    };

    /// Nutritional value of each carrot eaten by a worm
    static const int foodValueOfCarrot = 2;
    
    /// The allowed directions of travel from any board position to
    /// an adjacent board positions
    typedef enum
    {
        NORTH=0, NORTHEAST, EAST, SE, S, SW, W, NW
    } direction;            // as numbers: 0 .. 7
    
    /// These are the possible statuses for a Worm instance
    typedef enum { EATEN, DEAD, ALIVE } status;
    
    static Info vegetarianInfo; //< Encapsulates Vegetarian specific data
    static Info scissorInfo;    //< Encapsulates ScissorHead specific data
    static Info cannibalInfo;   //< Encapsulates Cannibal specific data
    
public:
    /// Type used to uniquely identify each type of worm
    /// The fact that this type is a pointer cannot be exploited
    /// because the Info type is private to this class. This C++ idiom
    /// is sometimes called a "handle" i.e. an opaque pointer.
    typedef Info *UniqueWormType;
    
    /// This vector contains all possible unique worm types.
    static const std::vector<UniqueWormType> UniqueWormTypes;
    
    //////////////////////////////////////////////////////////////////
    /// Instances of this structure encapsulate information about each
    /// segment in a worm.
    class segment
    {
    private:
        friend class Worm;
        int x, y;         //< coordinates of the segment
        char c;           //< the letter carried
        
    public:
        segment(int ax, int ay, char ac) : x(ax), y(ay), c(ac) {}
        char getC() const { return c; }
        int getX() const { return x; }
        int getY() const { return y; }
    };

private:
    UniqueWormType        m_typeInfo;   //< once set, type does not change
    direction             m_direction;  //< its (head's) direction
    int                   m_stomach;    //< food value
    status                m_status;     //< EATEN, DEAD, or ALIVE
    std::vector<segment>  m_body;       //< body parts
 
    segment &getHead() { return m_body.back(); }
    bool isHungry() const;
    status getStatus() const { return m_status; }
    void updateStatusBasedOnStomach();
    
    /// This function should only be used for pre and post condition
    /// assertion checking
    bool areAllSegmentsContiguous(const WormsSim &sim) const;
    
public:

    //////////////////////////////////////////////////////////////////
    /// Constructs a worm instance with enough segments to store one
    /// each character in saying in a different worm segment. This
    /// function stores the saying character starting at the worm's
    /// head (the first character in saying is stored by the head).
    /// As a side effect, this function increases the count of the
    /// number of worms with the specified type.
    Worm(UniqueWormType typeInfo, //< Information about the type of the worm
        std::string saying, //< The saying that the worm carries one character per segment
        int posX,   //< The initial x position of the worm's segments
        int posY,   //< The initial y position of the worm's segments
        WormsSim &sim); //< The simulation in which the worm will reside
    
    //////////////////////////////////////////////////////////////////
    /// Constructs a worm instance that contains copies of the range of
    /// segments from the tail up to but not including the segment at
    /// truncation index in original.
    /// As a side effect, this function increases the count of the
    /// number of worms with the specified type.
    Worm(const Worm &original, //< The worm from whom segments are copied into the constructed worm
        int truncationIndex);  //< Must be greater than 0 and less than the number of segments in original
    
    /// @name Non-mutating Accessors
    /// @{
    int getFoodValue() const;
    int getAttr() const { return m_typeInfo->attr; }
    const segment &getHead() const { return m_body.back(); }
    const std::vector<segment> &getBody() const { return m_body; }
    int segmentIndexAt(int x, int y) const;
    bool isAlive() const { return getStatus() == ALIVE; }
    /// @}
    
    
    /// @name Functions That Mutate a Worm Instance
    /// @{
    
    //////////////////////////////////////////////////////////////////
    /// Call to execute one simulation step of the worm's life
    void live(WormsSim &sim);   //< The simulation in which the worm resides
    
    //////////////////////////////////////////////////////////////////
    /// Template Method: called when a worm has been sliced
    void onWasSlicedAtSegmentIndex(
        int victimSegmentNumber);   //< Segment index at which worm was sliced (must be > 0 and < the number of segments in the worm)
    
    //////////////////////////////////////////////////////////////////
    /// Template Method: called when worm has been eaten
    /// As a side effect, this function decrements the count of the
    /// number of living instances that have newly eaten worm's type.
    void onWasEaten();
    /// @}
    
    /// @name Access Simulation Wide Worm Statistics of Worm Types
    /// @{
    static int getNumVegetarians();   //< Returns number of living instances of worms with type Vegetarian
    static int getNumCanibals();      //< Returns number of living instances of worms with type Cannibal
    static int getNumScissorheads();  //< Returns number of living instances of worms with type ScissorHead
    static void resetWormCounters();  //< Sets counters of number of living instances of all types to 0
    /// @}
};

#endif // WORM_H
