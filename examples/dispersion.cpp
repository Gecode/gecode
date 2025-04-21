#include <gecode/driver.hh>
#include <gecode/kernel.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh> // Explicitly include minimodel.hh

#include <vector>
#include <numeric>    // For std::iota
#include <cmath>      // For std::abs, std::sqrt in print
#include <algorithm>  // For std::shuffle
#include <random>     // For std::default_random_engine
#include <iostream>   // For output
#include <stdexcept>  // For exceptions
#include <string>     // For branching name lookup

// Helper structure for grid points
struct Point {
    int x; // x-coordinate
    int y; // y-coordinate
    int id;// Original index in the full n*n grid (0 to n*n-1)
};

// Calculate Manhattan distance between two points
int manhattan_distance(const Point& p1, const Point& p2) {
    return std::abs(p1.x - p2.x) + std::abs(p1.y - p2.y);
}

/**
 * \brief Propagator implementing greedy heuristic pruning for p-Dispersion.
 *
 * Inherits from NaryOnePropagator:
 * - Nary part: Views `x` (IntVar, trigger PC_INT_VAL)
 * - One part: View `y` (min_dist, trigger PC_INT_BND)
 *
 * Estimates an upper bound on the objective at the current node using a greedy
 * assignment of unassigned variables. If the estimate is not better than the current
 * best solution found (incumbent lower bound), it posts failure.
 */
class HeuristicPruner : public Gecode::MixNaryOnePropagator<
        Gecode::Int::IntView, Gecode::Int::PC_INT_VAL,
        Gecode::Int::IntView, Gecode::Int::PC_INT_BND
> {
protected:

    // Instance data needed for calculations
    const int p;                         // Number of facilities to select
    const int num_potential;             // Number of potential locations (|P|)
    const Gecode::IntArgs dist_storage;  // Copy of the flat distance matrix

public:
    /// Constructor for creation
    HeuristicPruner(Gecode::Home home,
                    Gecode::ViewArray<Gecode::Int::IntView>& points,
                    Gecode::Int::IntView min_dist,
                    int p_val,
                    int num_potential_val,
                    const Gecode::IntArgs& dist_storage_val)
            : MixNaryOnePropagator<Gecode::Int::IntView, Gecode::Int::PC_INT_VAL,
                                   Gecode::Int::IntView, Gecode::Int::PC_INT_BND>(home, points, min_dist),
              p(p_val),
              num_potential(num_potential_val),
              dist_storage(dist_storage_val)
    {}

    /// Constructor for cloning
    HeuristicPruner(Gecode::Space& home, HeuristicPruner& p)
            : MixNaryOnePropagator<Gecode::Int::IntView, Gecode::Int::PC_INT_VAL,
                                   Gecode::Int::IntView, Gecode::Int::PC_INT_BND>(home, p),
              p(p.p),
              num_potential(p.num_potential),
              dist_storage(p.dist_storage)
    {}

    /// Propagator execution
    virtual Gecode::ExecStatus propagate(Gecode::Space& home, const Gecode::ModEventDelta& med) {
        // Get current incumbent cost (lower bound of objective variable y)
        int incumbent_cost = y.min();

        // Don't run if no incumbent exists yet
        if (incumbent_cost <= 0) { // Adjust if 0 is a possible valid distance
            return Gecode::ES_FIX;
        }

        // --- Identify assigned and unassigned variables ---
        std::vector<int> assigned_indices;
        std::vector<int> unassigned_indices;
        assigned_indices.reserve(p);
        unassigned_indices.reserve(p);

        for (int i = 0; i < p; ++i) {
            if (x[i].assigned()) {
                assigned_indices.push_back(i);
            } else {
                unassigned_indices.push_back(i);
            }
        }
        int assigned_count = assigned_indices.size();

        // If all are assigned, propagator is done.
        if (assigned_count == p) {
            return home.ES_SUBSUMED(*this);
        }

        // --- Perform Greedy Assignment for unassigned variables ---
        // Stores the complete assignment (original + greedy fill)
        std::vector<int> greedy_assignment(p);
        // Keep track of which indices in greedy_assignment are filled greedily
        std::vector<int> greedily_assigned_indices;
        greedily_assigned_indices.reserve(p - assigned_count);

        bool possible = true; // Flag if greedy assignment is possible

        // 1. Copy known assigned values
        for (int idx : assigned_indices) {
            greedy_assignment[idx] = x[idx].val();
        }

        // 2. Greedily assign variables based on unassigned_indices
        for (int current_unassigned_idx : unassigned_indices) {
            int best_val = -1;
            int max_min_dist = -1;

            // Iterate through domain of the current unassigned variable x[current_unassigned_idx]
            for (Gecode::Int::ViewValues<Gecode::Int::IntView> val_iter(x[current_unassigned_idx]); val_iter(); ++val_iter) {
                int current_val = val_iter.val();
                int current_min_dist_for_val = std::numeric_limits<int>::max();

                // Calculate distance to already assigned variables
                for (int assigned_idx : assigned_indices) {
                    int dist = get_dist(dist_storage, num_potential, current_val, greedy_assignment[assigned_idx]);
                    if (dist < current_min_dist_for_val) {
                        current_min_dist_for_val = dist;
                    }
                }

                // Calculate distance to previously greedily assigned variables
                for (int prev_greedy_idx : greedily_assigned_indices) {
                    int dist = get_dist(dist_storage, num_potential, current_val, greedy_assignment[prev_greedy_idx]);
                    if (dist < current_min_dist_for_val) {
                        current_min_dist_for_val = dist;
                    }
                }

                // Check if this value gives a better *minimum* distance
                if (current_min_dist_for_val > max_min_dist) {
                    max_min_dist = current_min_dist_for_val;
                    best_val = current_val;
                }
            } // End domain iteration for x[current_unassigned_idx]

            // If no suitable value could be found
            if (best_val == -1) {
                possible = false;
                break; // Exit the greedy assignment loop
            }
            // Store the chosen greedy value and mark index as greedily assigned
            greedy_assignment[current_unassigned_idx] = best_val;
            greedily_assigned_indices.push_back(current_unassigned_idx);

        } // End loop through unassigned variables

        // If greedy assignment failed, let other propagators handle it
        if (!possible) {
            return Gecode::ES_FIX;
        }

        // --- Compute Cost of the completed greedy assignment ---
        // Need to consider all pairs within the completed assignment
        int greedy_cost = std::numeric_limits<int>::max();
        std::vector<int> final_indices = assigned_indices;
        final_indices.insert(final_indices.end(), greedily_assigned_indices.begin(), greedily_assigned_indices.end());

        for (size_t i = 0; i < final_indices.size(); ++i) {
            for (size_t j = i + 1; j < final_indices.size(); ++j) {
                int idx1 = final_indices[i];
                int idx2 = final_indices[j];
                int dist = get_dist(dist_storage, num_potential, greedy_assignment[idx1], greedy_assignment[idx2]);
                if (dist < greedy_cost) {
                    greedy_cost = dist;
                }
            }
        }

        // --- Compare estimated cost (greedy_cost) with incumbent ---
        if (greedy_cost <= incumbent_cost) {
            // std::cout << "Pruning! Greedy Est: " << greedy_cost << " <= Incumbent: " << incumbent_cost << std::endl; // Debug
            return Gecode::ES_FAILED; // Post failure
        } else {
            // std::cout << "NOT Pruning. Greedy Est: " << greedy_cost << " > Incumbent: " << incumbent_cost << std::endl; // Debug
            return Gecode::ES_FIX; // Heuristic doesn't prune, keep searching
        }
    }

    /// Create copy during cloning
    virtual Propagator* copy(Gecode::Space& home) {
        return new (home) HeuristicPruner(home, *this);
    }

    /// Cost function
    virtual Gecode::PropCost cost(const Gecode::Space&, const Gecode::ModEventDelta&) const {
        // Quadratic cost seems reasonable given the nested loops
        return Gecode::PropCost::crazy(Gecode::PropCost::HI, x.size());
    }

    /// Post function to create and schedule the propagator
    static Gecode::ExecStatus post(Gecode::Home home,
                                   Gecode::ViewArray<Gecode::Int::IntView>& points,
                                   Gecode::Int::IntView min_dist,
                           int p_val,
                           int num_potential_val,
                           const Gecode::IntArgs& dist_storage_val) {
        if (p_val >= 2) {
            (void) new (home) HeuristicPruner(home, points, min_dist, p_val, num_potential_val, dist_storage_val);
        }
        return Gecode::ES_OK;
    }

};


// Enum defining the different branching strategies available via command-line
enum BranchingType {
    BRANCH_AFC_MIN,    // Default: INT_VAR_AFC_MIN / INT_VAL_MIN
    // Add other branching variants here if desired later
};


/**
 * \brief Command-line options for the p-Dispersion problem.
 *
 * Handles instance parameters (grid size, points to select, potential points),
 * random seed, and branching strategy selection. Also generates the instance data.
 */
class DispersionOptions : public Gecode::Options {
private:
    // Option objects handled by Gecode::Driver
    Gecode::Driver::IntOption _n;
    Gecode::Driver::IntOption _p;
    Gecode::Driver::IntOption _num_potential;
    Gecode::Driver::IntOption _t; // Informational parameter, not used in max-min model
    Gecode::Driver::IntOption _seed;

public:
    // Stores the branching strategy chosen via command-line options
    BranchingType branching_type_ = BranchingType::BRANCH_AFC_MIN; // Default value

    // Instance Data: Generated based on options, public for access by the model
    std::vector<Point> potential_points; // The |P| randomly selected potential points
    Gecode::IntArgs flat_dist_storage;   // Flat storage for the |P|x|P| distance matrix
    int max_dist;                        // Maximum Manhattan distance in the instance

    /// Constructor: Initializes options with descriptions and defaults
    DispersionOptions(const char* s, int n_def, int p_def, int P_def, int t_def)
            : Gecode::Options(s),
            // Initialize Gecode::Driver option objects
              _n("n", "side length of the grid (n x n)", n_def),
              _p("p", "number of points to select", p_def),
              _num_potential("P", "number of potential location points |P|", P_def),
              _t("t", "distance divisor t (informational only)", t_def),
              _seed("seed", "random seed", 0),
            // Initialize instance data members
              flat_dist_storage(), max_dist(0)
    {
        // Register the custom options with the base parser
        add(_n);
        add(_p);
        add(_num_potential);
        add(_t);
        add(_seed);

        // Setup branching option selection
        Gecode::Options::branching(branching_type_); // Initialize base class mechanism
        // Register command-line strings for branching choices
        Gecode::Options::branching(BranchingType::BRANCH_AFC_MIN, "afcmin", "min dom divided by afc (dom/wdeg, default)");
        // Add more branching options here if needed
    }

    /// Parse options from command line and validate
    void parse(int& argc, char**& argv) {
        Gecode::Options::parse(argc, argv); // Base class parses registered options
        // Retrieve the chosen branching enum value AFTER base class parsing
        branching_type_ = static_cast<BranchingType>(Gecode::Options::branching());

        // Validate parsed option values using the getters
        if (n() <= 0) throw Gecode::Exception("Options", "Grid size n must be positive.");
        if (p() < 2) throw Gecode::Exception("Options", "Number of points p must be at least 2.");
        if (num_potential() < p()) throw Gecode::Exception("Options", "|P| must be at least p.");
        if (num_potential() > n() * n()) throw Gecode::Exception("Options", "|P| cannot exceed n*n.");
        if (t() <= 0) throw Gecode::Exception("Options", "Distance divisor t must be positive.");
    }

    // Getters for accessing parsed option values
    int n() const { return _n.value(); }
    int p() const { return _p.value(); }
    int num_potential() const { return _num_potential.value(); }
    int t() const { return _t.value(); }
    int seed() const { return _seed.value(); }
    BranchingType branching() const { return branching_type_; }

    /// Print help message (includes registered options automatically)
    virtual void help(void) { Gecode::Options::help(); }

    /// Generate instance data (point selection and distance matrix)
    void generate_instance(Gecode::Rnd rnd); // Defined outside class

};


/**
 * \brief Gecode model for the p-Dispersion Problem (Maximize Minimum Distance).
 *
 * Selects p points from a pre-generated set of |P| potential locations
 * to maximize the minimum Manhattan distance between any pair of selected points.
 * Uses a ternary table (extensional) constraint.
 */
class Dispersion : public Gecode::IntMaximizeScript {
protected:
    // Problem parameters (copied from options for convenience/constness)
    const int p;
    const int num_potential; // Number of potential points = |P|
    // Instance data needed by methods other than constructor (e.g., print)
    const std::vector<Point> potential_points_;

    // Gecode variables
    Gecode::IntVarArray x;      // x[i] = index (0 to |P|-1) of the i-th selected point
    Gecode::IntVar min_dist;    // Objective: the minimum distance achieved

public:
    /// Constructor: posts constraints and branching
    Dispersion(const DispersionOptions& opt) :
            Gecode::IntMaximizeScript(opt),       // Initialize base Script/solver class
            p(opt.p()),                           // Initialize members from option getters
            num_potential(opt.num_potential()),
            potential_points_(opt.potential_points), // Copy necessary instance data
            x(*this, p, 0, num_potential - 1),       // Initialize decision variables
            min_dist(*this, 0, opt.max_dist)         // Initialize objective variable
    {
        // Create a LOCAL Matrix view for convenient access to distance data in this scope
        Gecode::Matrix<Gecode::IntArgs> dist_matrix_view(opt.flat_dist_storage,
                                                         num_potential,
                                                         num_potential);

        // 1. Build TupleSet for the extensional constraint
        //    Stores valid (point_idx1, point_idx2, distance) triples.
        Gecode::TupleSet dist_tuples(3);
        for (int i = 0; i < num_potential; ++i) {
            // Add entries for pairs (i, j) with i < j
            for (int j = i + 1; j < num_potential; ++j) {
                int d = dist_matrix_view(i, j); // Use local view
                Gecode::IntArgs tuple(3);
                tuple[0] = i; tuple[1] = j; tuple[2] = d;
                dist_tuples.add(tuple);
                // Add symmetric entry (j, i, d)
                tuple[0] = j; tuple[1] = i; tuple[2] = d;
                dist_tuples.add(tuple);
            }
            // Add entry for (i, i, 0) - needed for extensional robustness
            Gecode::IntArgs tuple(3);
            tuple[0] = i; tuple[1] = i; tuple[2] = 0;
            dist_tuples.add(tuple);
        }
        dist_tuples.finalize(); // Essential step


        // 2. Post Core Constraints
        // Ensure p distinct potential points are selected
        Gecode::distinct(*this, x, opt.ipl());

        // Symmetry breaking: Order the indices of selected points
        Gecode::rel(*this, x, Gecode::IRT_LE);

        // Create intermediate variables for distances between pairs of selected points
        int num_dist_vars = p * (p - 1) / 2;
        Gecode::IntVarArgs d_vars(*this, num_dist_vars, 0, opt.max_dist);

        // Link selected points x[i], x[j] to their distance d_vars[k] using the table
        int k = 0;
        for (int i = 0; i < p; ++i) {
            for (int j = i + 1; j < p; ++j) {
                Gecode::extensional(*this, {x[i], x[j], d_vars[k]}, dist_tuples);
                k++;
            }
        }

        // Define the objective: min_dist is the minimum of all pairwise distances
        Gecode::min(*this, d_vars, min_dist); // Use 'min', not 'minimum'

        // 3. Post Branching Strategy
        // Select branching based on command-line option choice
        switch (opt.branching()) { // Use getter for enum
            case BRANCH_AFC_MIN:
                // Use AFC heuristic on x variables, select smallest value first
                Gecode::branch(*this, x, Gecode::INT_VAR_AFC_MIN(opt.decay()), Gecode::INT_VAL_MIN());
                break;
                // Add cases for other branching strategies here if defined
            default: // Fallback to default
                Gecode::branch(*this, x, Gecode::INT_VAR_AFC_MIN(opt.decay()), Gecode::INT_VAL_MIN());
                break;
        }
    }

    /// Return the objective variable for maximization
    virtual Gecode::IntVar cost(void) const {
        return min_dist;
    }

    /// Print the solution
    virtual void
    print(std::ostream& os) const {
        os << "Solution Found:" << std::endl;
        os << "  Selected potential point indices (0 to |P|-1): " << x << std::endl;
        os << "  Selected points coordinates (original grid): ";
        // Use locally stored potential points vector to get coordinates
        for (int i=0; i < p; ++i) { // Use member p
            int potential_idx = x[i].val();
            if (potential_idx >= 0 && potential_idx < potential_points_.size()) {
                const Point& pt = potential_points_[potential_idx];
                os << "(" << pt.x << "," << pt.y << ") ";
            } else {
                // Should not happen if model is correct
                os << "(invalid index:" << potential_idx << ") ";
            }
        }
        os << std::endl;
        os << "  Objective: Minimum Manhattan distance = " << min_dist << std::endl;
        os << "--------------------" << std::endl;
    }

    /// Constructor for cloning \a s
    Dispersion(Dispersion& s) :
            Gecode::IntMaximizeScript(s), // Call base copy constructor
            p(s.p),                       // Copy simple members
            num_potential(s.num_potential),
            potential_points_(s.potential_points_) // Copy stored points vector
    {
        // Update Gecode variables using base class functionality
        x.update(*this, s.x);
        min_dist.update(*this, s.min_dist);
    }

    /// Perform copying during cloning
    virtual Gecode::Space* copy(void) {
        return new Dispersion(*this);
    }

};


// Helper function to get branching strategy name from enum for printing
std::string get_branching_name(BranchingType type) {
    switch (type) {
        case BRANCH_AFC_MIN: return "afcmin";
            // Add cases for other types if defined
        default: return "unknown";
    }
}

// Definition of the instance generation method
void DispersionOptions::generate_instance(Gecode::Rnd rnd) {
    int current_n = n(); // Use getter
    int current_num_potential = num_potential(); // Use getter
    int total_grid_points = current_n * current_n;
    if (current_num_potential > total_grid_points) {
        throw Gecode::Exception("Instance Generation", "|P| cannot exceed n*n.");
    }
    // Generate all points on the n x n grid
    std::vector<Point> all_points(total_grid_points);
    for (int i = 0; i < total_grid_points; ++i) {
        all_points[i] = {i % current_n, i / current_n, i};
    }
    // Create sequential indices
    std::vector<int> indices(total_grid_points);
    std::iota(indices.begin(), indices.end(), 0);
    // Shuffle indices randomly using the provided RNG seed
    std::default_random_engine engine(rnd.seed());
    std::shuffle(indices.begin(), indices.end(), engine);
    // Select the first |P| points based on shuffled indices
    potential_points.clear();
    potential_points.resize(current_num_potential);
    for (int i = 0; i < current_num_potential; ++i) {
        potential_points[i] = all_points[indices[i]];
    }
    // Calculate the distance matrix (|P| x |P|) and store flattened
    int matrix_dim = current_num_potential;
    int matrix_size = matrix_dim * matrix_dim;
    flat_dist_storage = Gecode::IntArgs(matrix_size);
    max_dist = 0;
    for (int i = 0; i < matrix_dim; ++i) {
        flat_dist_storage[i * matrix_dim + i] = 0; // Distance(i,i) = 0
        for (int j = i + 1; j < matrix_dim; ++j) {
            int d = manhattan_distance(potential_points[i], potential_points[j]);
            // Store symmetrically
            flat_dist_storage[i * matrix_dim + j] = d;
            flat_dist_storage[j * matrix_dim + i] = d;
            if (d > max_dist) {
                max_dist = d; // Track maximum distance
            }
        }
    }
}


/** \brief Main function: Parses options, generates instance, runs solver */
int
main(int argc, char* argv[]) {
    // Set default option values: n=10, p=5, |P|=30, t=8
    DispersionOptions opt("p-Dispersion (Max-Min Manhattan)", 10, 5, 30, 8);
    // Set default propagation level (can be overridden by command line)
    opt.ipl(Gecode::IPL_DOM);

    try {
        // Parse command line arguments
        opt.parse(argc, argv);
        // Initialize random number generator with seed from options
        Gecode::Rnd rnd(opt.seed());
        // Generate the instance data (points, distances) based on options
        opt.generate_instance(rnd);

        // Print summary of the instance and options being used
        std::cout << "Running p-Dispersion:" << std::endl;
        std::cout << "  Grid: " << opt.n() << "x" << opt.n()
                  << ", Select p=" << opt.p()
                  << ", From |P|=" << opt.num_potential() << " potential"
                  << ", Seed=" << opt.seed()
                  << std::endl;
        std::cout << "  Objective: Maximize Minimum Manhattan Distance" << std::endl;
        std::cout << "  Max possible distance in instance: " << opt.max_dist << std::endl;
        std::cout << "  Branching: " << get_branching_name(opt.branching()) << std::endl;
        std::cout << "--------------------" << std::endl;

        // Run the model using Branch-and-Bound search engine
        Gecode::Script::run<Dispersion, Gecode::BAB, DispersionOptions>(opt);

    } catch (const Gecode::Exception& e) {
        // Handle Gecode-specific exceptions
        std::cerr << "Gecode Error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        // Handle standard C++ exceptions
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        // Handle any other unknown exceptions
        std::cerr << "Unknown Error" << std::endl;
        return 1;
    }

    // Return success
    return 0;
}