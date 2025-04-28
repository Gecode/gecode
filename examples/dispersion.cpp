// =======================================
// p-Dispersion Solver with Gecode
// =======================================
// Author: Gemini & User Collaboration
// Date: April 23, 2025 (Simulated) - Revision 5 (Final)
// Description:
// Solves the p-Dispersion problem (maximize the minimum distance between
// selected facilities) using Gecode. Supports instance generation via
// an n*n grid (Manhattan distance) or reading from a BIN file format
// (lat/lon, Haversine distance). Includes an optional heuristic pruning
// propagator with optional local search enhancement.
// =======================================

// --- Gecode Includes ---
#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>
#include <gecode/kernel.hh>

// --- Standard C++ Includes ---
#include <vector>
#include <numeric>    // For std::iota
#include <cmath>      // For M_PI, sin, cos, asin, sqrt, abs, round
#include <algorithm>  // For std::shuffle, std::sort, std::min, std::max
#include <random>     // For std::default_random_engine
#include <iostream>   // For std::cout, std::cerr
#include <fstream>    // For std::ifstream
#include <sstream>    // For std::stringstream
#include <stdexcept>  // For std::runtime_error, std::exception
#include <string>     // For std::string, std::to_string
#include <limits>     // For std::numeric_limits
#include <memory>     // For std::unique_ptr
#include <cstring>    // For strcmp
#include <iomanip>    // For std::setw, std::left


// =======================================
// Enums (Defined before use)
// =======================================
/// Enum defining branching strategies selectable via command-line
enum BranchingType {
    BRANCH_AFC_MIN,
};

/// Enum defining instance generation types
enum GenType {
    GRID,
    BIN
};


// =======================================
// Forward Declarations
// =======================================
struct Point;

class DispersionInstance;

class StringArgOption; // Custom option class for filename
class DispersionOptions;

class HeuristicPruner;

class Dispersion;


// =======================================
// Global Constants & Helpers
// =======================================
const double EARTH_RADIUS_KM = 6371.0;

/// Represents a potential facility location.
struct Point {
    int x = 0;
    double lat = 0.0;
    int y = 0;
    double lon = 0.0;
    int id = -1;
    char category = ' ';
};

/// Calculate Manhattan distance
int manhattan_distance(const Point &p1, const Point &p2) {
    return std::abs(p1.x - p2.x) + std::abs(p1.y - p2.y);
}

/// Convert degrees to radians
double to_radians(double degrees) {
    return degrees * M_PI / 180.0;
}

/// Calculate Haversine distance in integer meters
int haversine_distance_meters(const Point &p1, const Point &p2) {
    double lat1_rad = to_radians(p1.lat);
    double lon1_rad = to_radians(p1.lon);
    double lat2_rad = to_radians(p2.lat);
    double lon2_rad = to_radians(p2.lon);
    double dlon = lon2_rad - lon1_rad;
    double dlat = lat2_rad - lat1_rad;
    double hav_dlat = sin(dlat / 2.0) * sin(dlat / 2.0);
    double hav_dlon = sin(dlon / 2.0) * sin(dlon / 2.0);
    double a = hav_dlat + cos(lat1_rad) * cos(lat2_rad) * hav_dlon;
    a = std::max(0.0, std::min(1.0, a));
    double c = 2.0 * asin(sqrt(a));
    double distance_km = EARTH_RADIUS_KM * c;
    return static_cast<int>(round(distance_km * 1000.0));
}

/// Get branching strategy name from enum for printing
std::string get_branching_name(BranchingType type) {
    switch (type) {
        case BRANCH_AFC_MIN:
            return "afcmin";
        default:
            return "unknown";
    }
}


// =======================================
// Custom String Argument Option Class
// =======================================
/**
 * \brief Custom option class for handling a single string argument (like a filename).
 */
class StringArgOption : public Gecode::Driver::BaseOption {
protected:
    std::string value_; ///< Stores the string value provided for the option
public:
    /// Constructor: Sets option name (e.g., "-binfile") and description
    StringArgOption(const char *option, const char *description)
            : Gecode::Driver::BaseOption(option, description), value_("") {}

    /// Parses command line arguments. Correctly overrides pure virtual function.
    virtual int parse(int argc, char *argv[]) override { // Correct signature
        if (argc == 0) { return 0; }
        // Use protected member 'iopt' (option with hyphen) for comparison
        if (std::strcmp(argv[0], this->iopt) == 0) {
            if (argc < 2) { // Value must follow the option name
                // Use 'opt' (option name without hyphen) for error message
                std::cerr << "Error: Option '" << this->iopt << "' requires a string argument." << std::endl;
                return -1; // Signal parsing error
            }
            value_ = argv[1]; // Store the provided string value
            // Return 2 to indicate consumption
            return 2;
        }
        return 0; // Option not found at current position
    }

    /// Prints help text for this option
    virtual void help(void) override {
        // Use protected members 'iopt' (option name) and 'exp' (explanation)
        std::cerr << "  " << std::setw(20) << std::left << this->iopt << this->exp << " (string)" << std::endl;
    }

    /// Returns the parsed string value
    const std::string &value(void) const { return value_; }
};


// =======================================
// DispersionInstance Class
// =======================================
/**
 * \brief Holds the data defining a specific p-Dispersion problem instance.
 */
class DispersionInstance {
public:
    // Members are non-const to allow default move operations
    int p = 0;
    int num_potential = 0;
    int max_dist = 0;
    std::vector<Point> potential_points;
    Gecode::IntArgs flat_dist_storage;

    /// Constructor: Initializes instance (takes ownership of moved data)
    DispersionInstance(int p_val, int num_potential_val,
                       std::vector<Point> &&points, Gecode::IntArgs &&dist_storage, int max_d)
            : p(p_val), num_potential(num_potential_val), max_dist(max_d),
              potential_points(std::move(points)), flat_dist_storage(std::move(dist_storage)) {
        if (p < 2 || num_potential < p || static_cast<int>(potential_points.size()) != num_potential ||
            flat_dist_storage.size() != num_potential * num_potential) {
            throw Gecode::Exception("DispersionInstance", "Inconsistent data in constructor.");
        }
    }

    /// Default constructor
    DispersionInstance() = default;

    // Allow moving
    DispersionInstance(DispersionInstance &&) = default;

    DispersionInstance &operator=(DispersionInstance &&) = default;

    // Prevent copying
    DispersionInstance(const DispersionInstance &) = delete;

    DispersionInstance &operator=(const DispersionInstance &) = delete;

    /// Helper to get distance from flat storage
    int get_dist(int i, int j) const {
        if (i < 0 || i >= num_potential || j < 0 || j >= num_potential) return std::numeric_limits<int>::max();
        return flat_dist_storage[i * num_potential + j];
    }
};


// =======================================
// DispersionOptions Class
// =======================================
/**
 * \brief Handles command-line options and owns the generated problem instance.
 */
class DispersionOptions : public Gecode::Options {
private:
    // Gecode option objects
    Gecode::Driver::IntOption _p;
    Gecode::Driver::IntOption _seed;
    Gecode::Driver::IntOption _n;
    Gecode::Driver::IntOption _grid_num_potential;
    StringArgOption _binfile; // Use custom option class
    Gecode::Driver::BoolOption _use_pruner;
    Gecode::Driver::IntOption _pruner_freq;
    Gecode::Driver::BoolOption _pruner_use_ls;
    Gecode::Driver::IntOption _t;
    std::unique_ptr<DispersionInstance> instance_data_; // Instance storage
    bool instance_generated_ = false;
    int gentype_choice_ = 0; // 0=GRID, 1=BIN

public:
    BranchingType branching_type_ = BRANCH_AFC_MIN;
    GenType gen_type_ = GRID;

    /// Constructor: Initializes and registers options
    DispersionOptions(const char *s, int n_def, int p_def, int P_def, int t_def)
            : Gecode::Options(s),
              _p("p", "number of points/facilities to select", p_def), // Option name without hyphen
              _seed("seed", "random seed for generation", 0),
              _n("n", "Grid: side length of the grid (n x n)", n_def),
              _grid_num_potential("P", "Grid: number of potential location points |P|", P_def),
              _binfile("binfile", "BIN: input file (lat,lon,category CSV)"), // Option name without hyphen
              _use_pruner("use-pruner", "enable heuristic pruning propagator", false),
              _pruner_freq("pruner-freq", "run pruner every k executions (1=always)", 1),
              _pruner_use_ls("pruner-ls", "enable local search within pruner", false),
              _t("t", "Informational distance divisor t", t_def) {
        // Register options with the Gecode parser
        add(_p);
        add(_seed);
        add(_n);
        add(_grid_num_potential);
        add(_binfile); // Add custom option
        add(_use_pruner);
        add(_pruner_freq);
        add(_pruner_use_ls);
        add(_t);

        // Setup choice options using Gecode::Options:: static-like calls
        // These configure the internal state of the Options object being constructed.
        Gecode::Options::branching(branching_type_);
        Gecode::Options::branching(BRANCH_AFC_MIN, "afcmin", "branch var AFC min, val min (default)");
/*
        Gecode::Options::string(gentype_choice_);
        Gecode::Options::string(0, "grid", "Generate instance on a grid (default)");
        Gecode::Options::string(1, "bin", "Generate instance from BIN file (-binfile needed)");
        */
    }

    /// Parse options, determine generation type, validate
    void parse(int &argc, char **&argv) {
        Gecode::Options::parse(argc, argv); // Base class calls parse for all registered options
        // Retrieve choice options using INSTANCE methods (getters)
        branching_type_ = static_cast<BranchingType>(branching());
        //gentype_choice_ = string(); // Note: string() is the getter for the string choice index
        gen_type_ = (gentype_choice_ == 0) ? GRID : BIN;

        // Validation
        if (p() < 2) throw Gecode::Exception("Options", "-p >= 2 required.");
        if (pruner_freq() <= 0) throw Gecode::Exception("Options", "-pruner-freq > 0 required.");
        if (gen_type_ == BIN && _binfile.value().empty()) {
            throw Gecode::Exception("Options", "-gentype bin requires -binfile argument.");
        }
    }

    // --- Getters ---
    int p() const { return _p.value(); }

    int seed() const { return _seed.value(); }

    BranchingType branching() const { return branching_type_; }

    GenType gentype() const { return gen_type_; }

    bool use_pruner() const { return _use_pruner.value(); }

    int pruner_freq() const { return _pruner_freq.value(); }

    bool pruner_use_ls() const { return _pruner_use_ls.value(); }

    int grid_n() const { return _n.value(); }

    int grid_num_potential() const { return _grid_num_potential.value(); }

    const std::string &bin_filename() const { return _binfile.value(); }

    /// Generate the instance data
    void create_instance(); // Defined later

    /// Get a const reference to the generated instance data
    const DispersionInstance &instance() const {
        if (!instance_generated_ || !instance_data_)
            throw Gecode::Exception("Options", "Instance accessed before creation.");
        return *instance_data_;
    }

    /// Print help message
    virtual void help(void) { Gecode::Options::help(); } // Base class help prints registered options
};


// =======================================
// HeuristicPruner Propagator
// =======================================
/**
 * \brief Propagator implementing greedy heuristic pruning for p-Dispersion.
 */
class HeuristicPruner : public Gecode::MixNaryOnePropagator<
        Gecode::Int::IntView, Gecode::Int::PC_INT_VAL,
        Gecode::Int::IntView, Gecode::Int::PC_INT_BND> {
protected:
    const DispersionInstance &instance;
    const int k_freq;
    const bool use_ls;
    int exec_count;
public:
    /// Constructor for creation
    HeuristicPruner(Gecode::Home home, Gecode::ViewArray<Gecode::Int::IntView> &x, Gecode::Int::IntView y,
                    const DispersionInstance &inst, int frequency, bool enable_ls)
            : Gecode::MixNaryOnePropagator<Gecode::Int::IntView, Gecode::Int::PC_INT_VAL,
            Gecode::Int::IntView, Gecode::Int::PC_INT_BND>(home, x, y),
              instance(inst), k_freq(frequency), use_ls(enable_ls), exec_count(0) {}

    /// Constructor for cloning
    HeuristicPruner(Gecode::Space &home, HeuristicPruner &p)
            : Gecode::MixNaryOnePropagator<Gecode::Int::IntView, Gecode::Int::PC_INT_VAL,
            Gecode::Int::IntView, Gecode::Int::PC_INT_BND>(home, p),
              instance(p.instance), k_freq(p.k_freq), use_ls(p.use_ls), exec_count(p.exec_count) {}

    /// Propagator execution logic
    virtual Gecode::ExecStatus propagate(Gecode::Space &home, const Gecode::ModEventDelta &med); // Defined later

    /// Create copy during cloning
    virtual Gecode::Propagator *copy(Gecode::Space &home) { return new(home) HeuristicPruner(home, *this); }

    /// Cost function: Estimate propagator cost
    virtual Gecode::PropCost cost(const Gecode::Space &, const Gecode::ModEventDelta &) const {
        return use_ls ? Gecode::PropCost::crazy(Gecode::PropCost::HI, x.size())
                      : Gecode::PropCost::quadratic(Gecode::PropCost::LO, x.size());
    }

    /// Static posting function used by helper
    static Gecode::ExecStatus post(Gecode::Home home,
                                   Gecode::ViewArray<Gecode::Int::IntView> &x_views,
                                   Gecode::Int::IntView y_min_dist,
                                   const DispersionInstance &inst, int frequency, bool enable_ls) {
        if (inst.p >= 2) {
            (void) new(home) HeuristicPruner(home, x_views, y_min_dist, inst, frequency, enable_ls);
        }
        return Gecode::ES_OK;
    }
};


// =======================================
// Helper Function for Posting Propagator
// =======================================
/// Posts the heuristic pruner, creating views internally.
void post_heuristic_pruner(Gecode::Home home,
                           Gecode::IntVarArgs x_vars,
                           Gecode::IntVar min_dist_var,
                           const DispersionInstance &inst,
                           int frequency, bool enable_ls) {
    if (inst.p >= 2) {
        // Create views needed by the propagator's post function
        Gecode::ViewArray<Gecode::Int::IntView> x_views(home, x_vars); // Correct constructor
        Gecode::Int::IntView min_dist_view(min_dist_var);
        // Call the propagator's static post function
        (void) HeuristicPruner::post(home, x_views, min_dist_view, inst, frequency, enable_ls);
    }
}


// =======================================
// Dispersion Model Class
// =======================================
/**
 * \brief Gecode model for the p-Dispersion Problem (Maximize Minimum Distance).
 */
class Dispersion : public Gecode::IntMaximizeScript {
protected:
    const DispersionInstance &instance; // Reference to instance data owned by Options
    Gecode::IntVarArray x;
    Gecode::IntVar min_dist;

public:
    /// Constructor: Takes options object, retrieves instance, posts constraints/branching
    Dispersion(const DispersionOptions &opt) :
            Gecode::IntMaximizeScript(opt),
            instance(opt.instance()), // Get instance ref from options
            x(*this, instance.p, 0, instance.num_potential - 1),
            min_dist(*this, 0, instance.max_dist) {
        Gecode::Matrix<Gecode::IntArgs> dist_matrix_view(instance.flat_dist_storage, instance.num_potential,
                                                         instance.num_potential);
        Gecode::TupleSet dist_tuples(3);
        for (int i = 0; i < instance.num_potential; ++i) {
            for (int j = i + 1; j < instance.num_potential; ++j) {
                int d = dist_matrix_view(i, j);
                Gecode::IntArgs tuple(3);
                tuple[0] = i;
                tuple[1] = j;
                tuple[2] = d;
                dist_tuples.add(tuple);
                tuple[0] = j;
                tuple[1] = i;
                tuple[2] = d;
                dist_tuples.add(tuple);
            }
            Gecode::IntArgs tuple(3);
            tuple[0] = i;
            tuple[1] = i;
            tuple[2] = 0;
            dist_tuples.add(tuple);
        }
        dist_tuples.finalize();

        Gecode::distinct(*this, x, opt.ipl());
        Gecode::rel(*this, x, Gecode::IRT_LE);

        int num_dist_vars = instance.p * (instance.p - 1) / 2;
        Gecode::IntVarArgs d_vars(*this, num_dist_vars, 0, instance.max_dist);
        int k = 0;
        for (int i = 0; i < instance.p; ++i) {
            for (int j = i + 1; j < instance.p; ++j) {
                Gecode::extensional(*this, {x[i], x[j], d_vars[k]}, dist_tuples);
                k++;
            }
        }
        Gecode::min(*this, d_vars, min_dist);

        if (opt.use_pruner()) {
            post_heuristic_pruner(*this, x, min_dist, instance, opt.pruner_freq(), opt.pruner_use_ls());
            std::cout << "  (Heuristic Pruner Enabled" << (opt.pruner_use_ls() ? " with LS" : "") << ", Freq="
                      << opt.pruner_freq() << ")" << std::endl;
        } else { std::cout << "  (Heuristic Pruner Disabled)" << std::endl; }

        switch (opt.branching()) {
            case BRANCH_AFC_MIN:
                Gecode::branch(*this, x, Gecode::INT_VAR_AFC_MIN(opt.decay()), Gecode::INT_VAL_MIN());
                break;
            default:
                Gecode::branch(*this, x, Gecode::INT_VAR_AFC_MIN(opt.decay()), Gecode::INT_VAL_MIN());
                break;
        }
    }

    /// Return cost variable
    virtual Gecode::IntVar cost(void) const { return min_dist; }

    /// Print the solution
    virtual void print(std::ostream &os) const {
        os << "Solution Found:" << std::endl;
        os << "  Selected potential point indices (0-" << instance.num_potential - 1 << "): " << x << std::endl;
        os << "  Selected points coordinates: ";
        bool is_grid = (instance.potential_points.empty() || instance.potential_points[0].category == ' ');
        for (int i = 0; i < instance.p; ++i) {
            int potential_idx = x[i].val();
            // Use static cast for comparing signed int with unsigned size_t
            if (potential_idx >= 0 && static_cast<size_t>(potential_idx) < instance.potential_points.size()) {
                const Point &pt = instance.potential_points[potential_idx];
                if (is_grid) os << "(" << pt.x << "," << pt.y << ") ";
                else os << "(lat:" << pt.lat << ",lon:" << pt.lon << ") ";
            } else { os << "(invalid index:" << potential_idx << ") "; }
        }
        os << std::endl;
        os << "  Objective: Minimum Actual Distance = " << min_dist << std::endl;
        os << "--------------------" << std::endl;
    }

    /// Constructor for cloning
    Dispersion(Dispersion &s) : Gecode::IntMaximizeScript(s), instance(s.instance) {
        x.update(*this, s.x);
        min_dist.update(*this, s.min_dist);
    }

    /// Perform copying during cloning
    virtual Gecode::Space *copy(void) { return new Dispersion(*this); }
};


// =======================================
// Definitions for Member Functions / Propagator
// =======================================

/// Definition of DispersionOptions::create_instance
void DispersionOptions::create_instance() {
    if (instance_generated_) return;
    Gecode::Rnd rnd(seed());
    std::vector<Point> points;
    Gecode::IntArgs distances;
    int max_d = 0;
    int num_pot = 0;

    if (gen_type_ == GRID) {
        int current_n = grid_n();
        num_pot = grid_num_potential();
        int current_p = p();
        if (current_n <= 0) throw Gecode::Exception("Instance Creation", "n > 0 required");
        if (num_pot < current_p) throw Gecode::Exception("Instance Creation", "|P| >= p required");
        if (num_pot > current_n * current_n) throw Gecode::Exception("Instance Creation", "|P| <= n*n required");
        int total_grid_points = current_n * current_n;
        std::vector<Point> all_points(total_grid_points);
        for (int i = 0; i < total_grid_points; ++i) {
            // Use explicit assignment to avoid narrowing warnings
            Point p;
            p.x = i % current_n;
            p.y = i / current_n;
            p.id = i;
            all_points[i] = p;
        }
        std::vector<int> indices(total_grid_points);
        std::iota(indices.begin(), indices.end(), 0);
        std::default_random_engine engine(rnd.seed());
        std::shuffle(indices.begin(), indices.end(), engine);
        points.resize(num_pot);
        for (int i = 0; i < num_pot; ++i) points[i] = all_points[indices[i]];
        int matrix_size = num_pot * num_pot;
        distances = Gecode::IntArgs(matrix_size);
        for (int i = 0; i < num_pot; ++i) {
            distances[i * num_pot + i] = 0;
            for (int j = i + 1; j < num_pot; ++j) {
                int d = manhattan_distance(points[i], points[j]);
                distances[i * num_pot + j] = d;
                distances[j * num_pot + i] = d;
                if (d > max_d) max_d = d;
            }
        }
    } else if (gen_type_ == BIN) {
        const std::string &filename = bin_filename(); // Use getter returning std::string ref
        if (filename.empty())
            throw Gecode::Exception("Instance Creation", "BIN filename is empty (-binfile required).");
        std::ifstream infile(filename);
        if (!infile.is_open()) {
            std::string msg = "Cannot open BIN file: " + filename;
            throw Gecode::Exception("Instance Creation", msg.c_str());
        }
        std::string line;
        while (std::getline(infile, line)) {
            if (line.empty() || line[0] == '#') continue;
            std::stringstream ss(line);
            std::string segment;
            Point p_data;
            try {
                std::getline(ss, segment, ',');
                p_data.lat = std::stod(segment);
                std::getline(ss, segment, ',');
                p_data.lon = std::stod(segment);
                std::getline(ss, segment, ',');
                if (!segment.empty()) p_data.category = segment[0]; else throw std::runtime_error("Missing category");
                if (p_data.category != 'A' && p_data.category != 'B' && p_data.category != 'C' &&
                    p_data.category != 'D')
                    throw std::runtime_error("Invalid category");
                points.push_back(p_data);
            } catch (const std::exception &e) {
                std::string msg = "Error parsing BIN file line: '" + line + "'. Reason: " + e.what();
                throw Gecode::Exception("Instance Creation", msg.c_str());
            }
        }
        infile.close();
        num_pot = points.size();
        int current_p = p();
        if (num_pot < current_p) {
            std::string msg = "Num points from BIN file (" + std::to_string(num_pot) + ") must be >= p (" +
                              std::to_string(current_p) + ")";
            throw Gecode::Exception("Instance Creation", msg.c_str());
        }
        int matrix_size = num_pot * num_pot;
        distances = Gecode::IntArgs(matrix_size);
        for (int i = 0; i < num_pot; ++i) {
            distances[i * num_pot + i] = 0;
            for (int j = i + 1; j < num_pot; ++j) {
                int d = haversine_distance_meters(points[i], points[j]);
                distances[i * num_pot + j] = d;
                distances[j * num_pot + i] = d;
                if (d > max_d) max_d = d;
            }
        }
    }
    instance_data_ = std::make_unique<DispersionInstance>(p(), num_pot, std::move(points), std::move(distances), max_d);
    instance_generated_ = true;
}

/// Definition of HeuristicPruner::propagate
Gecode::ExecStatus HeuristicPruner::propagate(Gecode::Space &home, const Gecode::ModEventDelta &med) {
    (void) med; // Mark delta as unused
    exec_count++;
    if ((k_freq > 1) && (exec_count % k_freq != 1)) return Gecode::ES_FIX;
    int incumbent_cost = y.min(); // y is min_dist view
    if (incumbent_cost <= 0) return Gecode::ES_FIX;
    std::vector<int> assigned_indices, unassigned_indices;
    assigned_indices.reserve(instance.p);
    unassigned_indices.reserve(instance.p);
    for (int i = 0; i < instance.p; ++i) {
        if (x[i].assigned())
            assigned_indices.push_back(i);
        else unassigned_indices.push_back(i);
    }
    if (assigned_indices.size() == static_cast<size_t>(instance.p)) return home.ES_SUBSUMED(*this); // Cast fixed

    std::vector<int> greedy_assignment(instance.p);
    std::vector<int> greedily_assigned_indices;
    bool possible = true;
    for (int idx: assigned_indices) greedy_assignment[idx] = x[idx].val();
    // --- Full Greedy Logic ---
    for (int current_unassigned_idx: unassigned_indices) {
        int best_val = -1;
        int max_min_dist = -1;
        Gecode::Int::ViewValues<Gecode::Int::IntView> val_iter(x[current_unassigned_idx]);
        if (!val_iter()) {
            possible = false;
            break;
        }
        for (; val_iter(); ++val_iter) {
            int current_val = val_iter.val();
            int current_min_dist_for_val = std::numeric_limits<int>::max();
            for (int assigned_idx: assigned_indices) {
                current_min_dist_for_val = std::min(current_min_dist_for_val, instance.get_dist(current_val,
                                                                                                greedy_assignment[assigned_idx]));
            }
            for (int prev_greedy_idx: greedily_assigned_indices) {
                current_min_dist_for_val = std::min(current_min_dist_for_val,
                                                    instance.get_dist(current_val, greedy_assignment[prev_greedy_idx]));
            }
            if (current_min_dist_for_val > max_min_dist) {
                max_min_dist = current_min_dist_for_val;
                best_val = current_val;
            }
        }
        if (best_val == -1) {
            possible = false;
            break;
        }
        greedy_assignment[current_unassigned_idx] = best_val;
        greedily_assigned_indices.push_back(current_unassigned_idx);
    }
    // --- End Greedy Logic ---
    if (!possible) return Gecode::ES_FIX;

    int current_estimated_cost = std::numeric_limits<int>::max();
    std::vector<int> final_indices = assigned_indices;
    final_indices.insert(final_indices.end(), greedily_assigned_indices.begin(), greedily_assigned_indices.end());
    for (size_t i = 0; i < final_indices.size(); ++i) {
        for (size_t j = i + 1; j < final_indices.size(); ++j) {
            current_estimated_cost = std::min(current_estimated_cost,
                                              instance.get_dist(greedy_assignment[final_indices[i]],
                                                                greedy_assignment[final_indices[j]]));
        }
    }

    // --- Full LS Logic ---
    if (use_ls && current_estimated_cost <= incumbent_cost) {
        bool changed_by_ls = true;
        while (changed_by_ls) {
            changed_by_ls = false;
            if (current_estimated_cost > incumbent_cost) break;
            std::vector<int> culprits;
            std::vector<bool> is_culprit(instance.p, false);
            for (size_t i = 0; i < final_indices.size(); ++i) {
                for (size_t j = i + 1; j < final_indices.size(); ++j) {
                    if (instance.get_dist(greedy_assignment[final_indices[i]], greedy_assignment[final_indices[j]]) ==
                        current_estimated_cost) {
                        for (int greedy_idx: greedily_assigned_indices) {
                            if (final_indices[i] == greedy_idx && !is_culprit[final_indices[i]]) {
                                culprits.push_back(final_indices[i]);
                                is_culprit[final_indices[i]] = true;
                            }
                            if (final_indices[j] == greedy_idx && !is_culprit[final_indices[j]]) {
                                culprits.push_back(final_indices[j]);
                                is_culprit[final_indices[j]] = true;
                            }
                        }
                    }
                }
            }
            if (culprits.empty()) break;
            int best_ls_move_cost = -1;
            int best_ls_move_idx = -1;
            int best_ls_move_val = -1;
            for (int culprit_idx: culprits) {
                int original_val = greedy_assignment[culprit_idx];
                for (Gecode::Int::ViewValues<Gecode::Int::IntView> val_iter(x[culprit_idx]); val_iter(); ++val_iter) {
                    int new_val = val_iter.val();
                    if (new_val == original_val) continue;
                    greedy_assignment[culprit_idx] = new_val;
                    int temp_cost = std::numeric_limits<int>::max();
                    for (size_t i = 0; i < final_indices.size(); ++i) {
                        for (size_t j = i + 1; j < final_indices.size(); ++j) {
                            temp_cost = std::min(temp_cost, instance.get_dist(greedy_assignment[final_indices[i]],
                                                                              greedy_assignment[final_indices[j]]));
                        }
                    }
                    if (temp_cost > best_ls_move_cost) {
                        best_ls_move_cost = temp_cost;
                        best_ls_move_idx = culprit_idx;
                        best_ls_move_val = new_val;
                    }
                    greedy_assignment[culprit_idx] = original_val;
                }
            }
            if (best_ls_move_cost > current_estimated_cost) {
                current_estimated_cost = best_ls_move_cost;
                greedy_assignment[best_ls_move_idx] = best_ls_move_val;
                changed_by_ls = true;
            }
            else { changed_by_ls = false; }
        }
    }
    // --- End LS Logic ---

    if (current_estimated_cost <= incumbent_cost) { return Gecode::ES_FAILED; } else { return Gecode::ES_FIX; }
}


// =======================================
// Main Function
// =======================================

/** \brief Main entry point */
int
main(int argc, char *argv[]) {
    // Create options object (now only takes description and default parameters)
    DispersionOptions opt("p-Dispersion Solver", 10, 5, 30, 8);
    opt.ipl(Gecode::IPL_DOM);

    try {
        // Parse Gecode options (this will also parse -binfile via StringArgOption)
        opt.parse(argc, argv);
        opt.create_instance(); // Generate the instance data

        const DispersionInstance &instance = opt.instance(); // Get instance ref

        // --- Print Run Configuration Summary ---
        std::cout << "Running p-Dispersion:" << std::endl;
        if (opt.gentype() == GRID) { /* Print Grid Info */ } else { /* Print BIN Info */ }
        if (opt.gentype() == GRID) {
            std::cout << "  Generation Type: Grid\n  Grid Size: " << opt.grid_n() << "x" << opt.grid_n()
                      << "\n  Distance Metric: Manhattan" << std::endl;
        }
        else {
            std::cout << "  Generation Type: BIN\n  Input File: " << opt.bin_filename()
                      << "\n  Distance Metric: Haversine (meters)" << std::endl;
        }
        std::cout << "  Select p=" << instance.p << ", From |P|=" << instance.num_potential << " potential" << ", Seed="
                  << opt.seed() << std::endl;
        std::cout << "  Objective: Maximize Minimum Actual Distance" << std::endl;
        std::cout << "  Max actual distance in instance: " << instance.max_dist << std::endl;
        std::cout << "  Branching: " << get_branching_name(opt.branching()) << std::endl;
        std::cout << "--------------------" << std::endl;
        // Pruner status message printed from model constructor

        // --- Execute the Solver ---
        Gecode::Script::run<Dispersion, Gecode::BAB, DispersionOptions>(opt);

    } catch (const Gecode::Exception &e) {
        std::cerr << "Gecode Error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown Error" << std::endl;
        return 1;
    }
    return 0;
}