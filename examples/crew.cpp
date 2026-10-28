/*
 *  Main authors:
 *     Guido Tack <tack@gecode.dev>
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Copyright:
 *     Guido Tack, 2004
 *     Christian Schulte, 2004
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.dev
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/set.hh>

using namespace Gecode;

namespace {
    /// The airline employees
    typedef enum {
        TOM,
        DAVID,
        JEREMY,
        RON,
        JOE,
        BILL,
        FRED,
        BOB,
        MARIO,
        ED,
        CAROL,
        JANET,
        TRACY,
        MARILYN,
        CAROLYN,
        CATHY,
        INEZ,
        JEAN,
        HEATHER,
        JULIET
    } Employee;
    const int no_of_employees = JULIET + 1;

    /// A flight to schedule
    struct Flight {
        int staff;     ///< Overall number of cabin crew needed
        int stewards;  ///< How many stewards are required
        int hostesses; ///< How many hostesses are required
        int french;    ///< How many French speaking employees are required
        int spanish;   ///< How many Spanish speaking employees are required
        int german;    ///< How many German speaking employees are required
    };

    const char* employee_to_name(Employee e);
    extern const int stewards[];
    extern const int no_of_stewards;
    extern const int hostesses[];
    extern const int no_of_hostesses;
    extern const int spanish_speaking[];
    extern const int no_of_spanish_speaking;
    extern const int french_speaking[];
    extern const int no_of_french_speaking;
    extern const int german_speaking[];
    extern const int no_of_german_speaking;
    extern const Flight required_crew[];
    extern const int no_of_flights;
} // namespace

/**
 * \brief %Example: Airline crew allocation
 *
 * Assign 20 flight attendants to 10 flights. Each flight needs a certain
 * number of cabin crew, and they have to speak certain languages.
 * Every cabin crew member has two flights off after an attended flight.
 *
 * \ingroup Example
 *
 */
class Crew : public Script {
public:
    /// The crew for each flight
    SetVarArray flight;

    /// The actual model
    Crew(const Options& opt) : Script(opt), flight(*this, no_of_flights, IntSet::empty, 0, no_of_employees - 1) {
        IntSet stewards_ds(stewards, no_of_stewards);
        IntSet hostesses_ds(hostesses, no_of_hostesses);
        IntSet spanish_ds(spanish_speaking, no_of_spanish_speaking);
        IntSet french_ds(french_speaking, no_of_french_speaking);
        IntSet german_ds(german_speaking, no_of_german_speaking);

        for (int i = 0; i < no_of_flights; i++) {
            // The flight has staff as required by the specification
            rel(*this, cardinality(flight[i]) == required_crew[i].staff);

            // Enough members of different categories are on board
            rel(*this, cardinality(flight[i] & stewards_ds) >= required_crew[i].stewards);
            rel(*this, cardinality(flight[i] & hostesses_ds) >= required_crew[i].hostesses);
            rel(*this, cardinality(flight[i] & spanish_ds) >= required_crew[i].spanish);
            rel(*this, cardinality(flight[i] & french_ds) >= required_crew[i].french);
            rel(*this, cardinality(flight[i] & german_ds) >= required_crew[i].german);
        }

        // No crew member of flight i works on flights i+1 and i+2
        for (int i = 0; i < no_of_flights - 2; i++) {
            rel(*this, flight[i] || flight[i + 1]);
            rel(*this, flight[i] || flight[i + 2]);
        }
        rel(*this, flight[no_of_flights - 2] || flight[no_of_flights - 1]);

        branch(*this, flight, SET_VAR_NONE(), SET_VAL_MIN_INC());
    }

    /// Print solution
    void
    print(std::ostream& os) const override {
        for (int i = 0; i < no_of_flights; i++) {
            os << "\tFlight " << i + 1 << ":" << '\n';
            os << "\t\tCrew\tStew.\tHost.\tFrench\tSpanish\tGerman" << '\n' << "\t";
            os << "\t" << required_crew[i].staff << "\t" << required_crew[i].stewards << "\t"
               << required_crew[i].hostesses << "\t" << required_crew[i].spanish << "\t" << required_crew[i].french
               << "\t" << required_crew[i].german << '\n';

            os << "\t\tSchedule:" << '\n' << "\t\t";
            if (flight[i].assigned()) {
                for (SetVarGlbValues d(flight[i]); d(); ++d) {
                    os << employee_to_name(static_cast<Employee>(d.val())) << " ";
                }
            } else {
                os << "\tRequired: ";
                for (SetVarGlbValues d(flight[i]); d(); ++d) {
                    os << employee_to_name(static_cast<Employee>(d.val())) << " ";
                }
                os << '\n' << "\t\t\tPossible: ";
                for (SetVarUnknownValues d(flight[i]); d(); ++d) {
                    os << employee_to_name(static_cast<Employee>(d.val())) << " ";
                }
            }
            os << '\n' << '\n';
        }
    }

    /// Constructor for cloning \a s
    Crew(Crew& s) : Script(s) {
        flight.update(*this, s.flight);
    }
    /// Copy during cloning
    Space*
    copy(void) override {
        return new Crew(*this);
    }
};

/** \brief Main-function
 *  \relates Crew
 */
int
main(int argc, char* argv[]) {
    Options o("Crew");
    o.iterations(100);
    o.parse(argc, argv);
    Script::run<Crew, DFS, Options>(o);
    return 0;
}

namespace {

    /// Return name of employee \a e as a string
    const char*
    employee_to_name(Employee e) {
        switch (e) {
        case TOM:
            return "Tom";
        case DAVID:
            return "David";
        case JEREMY:
            return "Jeremy";
        case RON:
            return "Ron";
        case JOE:
            return "Joe";
        case BILL:
            return "Bill";
        case FRED:
            return "Fred";
        case BOB:
            return "Bob";
        case MARIO:
            return "Mario";
        case ED:
            return "Ed";
        case CAROL:
            return "Carol";
        case JANET:
            return "Janet";
        case TRACY:
            return "Tracy";
        case MARILYN:
            return "Marilyn";
        case CAROLYN:
            return "Carolyn";
        case CATHY:
            return "Cathy";
        case INEZ:
            return "Inez";
        case JEAN:
            return "Jean";
        case HEATHER:
            return "Heather";
        case JULIET:
            return "Juliet";
        default:
            GECODE_NEVER;
            return "";
        }
    }

    // these have to be sorted!
    /// The stewards
    const int stewards[] = {TOM, DAVID, JEREMY, RON, JOE, BILL, FRED, BOB, MARIO, ED};
    /// The number of stewards
    const int no_of_stewards = sizeof(stewards) / sizeof(int);
    /// The hostesses
    const int hostesses[] = {CAROL, JANET, TRACY, MARILYN, CAROLYN, CATHY, INEZ, JEAN, HEATHER, JULIET};
    /// The number of hostesses
    const int no_of_hostesses = sizeof(hostesses) / sizeof(int);
    /// The French speaking employees
    const int french_speaking[] = {BILL, INEZ, JEAN, JULIET};
    /// The number of French speaking employees
    const int no_of_french_speaking = sizeof(french_speaking) / sizeof(int);
    /// The German speaking employees
    const int german_speaking[] = {TOM, JEREMY, MARIO, CATHY, JULIET};
    /// The number of German speaking employees
    const int no_of_german_speaking = sizeof(german_speaking) / sizeof(int);
    /// The Spanish speaking employees
    const int spanish_speaking[] = {JOE, BILL, FRED, MARIO, MARILYN, INEZ, HEATHER};
    /// The number of Spanish speaking employees
    const int no_of_spanish_speaking = sizeof(spanish_speaking) / sizeof(int);

    /// The flights to schedule
    const Flight required_crew[] = {{4, 1, 1, 1, 1, 1}, {5, 1, 1, 1, 1, 1}, {5, 1, 1, 1, 1, 1}, {6, 2, 2, 1, 1, 1},
                                    {7, 3, 3, 1, 1, 1}, {4, 1, 1, 1, 1, 1}, {5, 1, 1, 1, 1, 1}, {6, 1, 1, 1, 1, 1},
                                    {6, 2, 2, 1, 1, 1}, {7, 3, 3, 1, 1, 1}};

    /// The number of flights to schedule
    const int no_of_flights = sizeof(required_crew) / sizeof(Flight);
} // namespace

// STATISTICS: example-any
