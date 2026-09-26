#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <vector>

struct Vec2 {
    double x{}, y{};

    Vec2 operator+(const Vec2& v) const {
        return {x + v.x, y + v.y};
    }

    Vec2 operator-(const Vec2& v) const {
        return {x - v.x, y - v.y};
    }

    Vec2 operator*(double s) const {
        return {x * s, y * s};
    }
};

double norm(const Vec2& v) {
    return std::sqrt(v.x * v.x + v.y * v.y);
}

struct Spacecraft {
    int id;
    Vec2 position;
    Vec2 velocity;
    double maneuverCost = 0.0;
};

struct Scenario {
    std::vector<Spacecraft> spacecraft;

    double dt = 1.0;
    int steps = 300;

    // Normalized research parameter.
    // This is NOT a real spacecraft collision radius.
    double collisionRadius = 1.0;
};

struct Metrics {
    bool collision = false;
    double minimumSeparation =
        std::numeric_limits<double>::infinity();

    double totalManeuverCost = 0.0;
    int maneuveringSpacecraft = 0;
};


// ------------------------------------------------------------
// BASIC SIMULATION
// ------------------------------------------------------------

void propagate(Spacecraft& spacecraft, double dt) {
    spacecraft.position =
        spacecraft.position +
        spacecraft.velocity * dt;
}

double distanceBetween(
    const Spacecraft& a,
    const Spacecraft& b
) {
    return norm(a.position - b.position);
}

Vec2 avoidanceDirection(
    const Spacecraft& a,
    const Spacecraft& b
) {
    Vec2 difference =
        a.position - b.position;

    double distance = norm(difference);

    if (distance < 1e-12) {
        return {1.0, 0.0};
    }

    return difference * (1.0 / distance);
}


// ------------------------------------------------------------
// CENTRALIZED CONTROL
// ------------------------------------------------------------
//
// One central controller observes the network and decides
// which spacecraft should maneuver.
//
// This is the baseline architecture.
// ------------------------------------------------------------

void centralizedController(Scenario& scenario) {

    double minimumDistance =
        std::numeric_limits<double>::infinity();

    int spacecraftA = -1;
    int spacecraftB = -1;

    for (int i = 0;
         i < static_cast<int>(scenario.spacecraft.size());
         ++i) {

        for (int j = i + 1;
             j < static_cast<int>(scenario.spacecraft.size());
             ++j) {

            double distance =
                distanceBetween(
                    scenario.spacecraft[i],
                    scenario.spacecraft[j]
                );

            if (distance < minimumDistance) {

                minimumDistance = distance;
                spacecraftA = i;
                spacecraftB = j;
            }
        }
    }

    // Trigger centralized intervention
    // when two spacecraft become sufficiently close.
    if (spacecraftA >= 0 &&
        minimumDistance < 5.0) {

        Vec2 direction =
            avoidanceDirection(
                scenario.spacecraft[spacecraftA],
                scenario.spacecraft[spacecraftB]
            );

        // Normalized maneuver.
        // Replace with a validated orbital maneuver model
        // when the aerospace equations are added.
        Vec2 deltaV =
            direction * 0.01;

        scenario.spacecraft[spacecraftA].velocity =
            scenario.spacecraft[spacecraftA].velocity +
            deltaV;

        scenario.spacecraft[spacecraftA].maneuverCost +=
            norm(deltaV);
    }
}


// ------------------------------------------------------------
// HYPERBOLIC NETWORK
// ------------------------------------------------------------
//
// Hyperbolic geometry is used here as a mathematical
// representation of network relationships.
//
// This does NOT claim that fungi literally perform
// hyperbolic computation.
// ------------------------------------------------------------

struct Edge {

    int a;
    int b;

    double strength = 1.0;
};


// Poincare-ball distance.
//
// This is the mathematical hyperbolic distance used by
// the network representation.
double poincareDistance(
    const Vec2& a,
    const Vec2& b
) {

    const double epsilon = 1e-9;

    double normA =
        std::min(norm(a), 1.0 - epsilon);

    double normB =
        std::min(norm(b), 1.0 - epsilon);

    double numerator =
        1.0 +
        2.0 * std::pow(norm(a - b), 2);

    double denominator =
        std::max(
            (1.0 - normA * normA) *
            (1.0 - normB * normB),
            epsilon
        );

    double argument =
        std::max(
            1.0,
            numerator / denominator
        );

    return std::acosh(argument);
}


// Convert local spacecraft conditions into a
// normalized hyperbolic representation.
//
// This is an engineering abstraction, not a biological claim.
Vec2 hyperbolicEmbedding(
    const Spacecraft& spacecraft,
    double localRisk
) {

    double radius =
        std::tanh(0.5 * localRisk);

    double angle =
        std::atan2(
            spacecraft.velocity.y,
            spacecraft.velocity.x
        );

    return {
        radius * std::cos(angle),
        radius * std::sin(angle)
    };
}


// ------------------------------------------------------------
// MYCELIUM-INSPIRED DISTRIBUTED CONTROL
// ------------------------------------------------------------
//
// Biological inspiration:
//
// - interconnected network
// - local sensing
// - electrical activity
// - propagating signals
// - adaptive network relationships
//
// The controller is intentionally an abstraction of those
// properties rather than a claim that this reproduces
// fungal cognition.
// ------------------------------------------------------------

void distributedMycelialController(
    Scenario& scenario,
    std::vector<Edge>& edges
) {

    int n =
        static_cast<int>(
            scenario.spacecraft.size()
        );

    for (int i = 0; i < n; ++i) {

        double localRisk = 0.0;

        // Each spacecraft evaluates its own local
        // environment instead of waiting for a
        // centralized controller.
        for (int j = 0; j < n; ++j) {

            if (i == j)
                continue;

            double distance =
                distanceBetween(
                    scenario.spacecraft[i],
                    scenario.spacecraft[j]
                );

            if (distance < 5.0) {

                localRisk +=
                    (5.0 - distance) / 5.0;
            }
        }

        if (localRisk <= 0.0)
            continue;


        Vec2 currentEmbedding =
            hyperbolicEmbedding(
                scenario.spacecraft[i],
                localRisk
            );


        // Propagate the local signal through
        // neighboring network connections.
        for (auto& edge : edges) {

            if (edge.a != i &&
                edge.b != i) {

                continue;
            }

            int neighbor =
                (edge.a == i)
                    ? edge.b
                    : edge.a;


            double neighborRisk = 0.0;

            double distance =
                distanceBetween(
                    scenario.spacecraft[i],
                    scenario.spacecraft[neighbor]
                );

            if (distance < 5.0) {

                neighborRisk =
                    (5.0 - distance) / 5.0;
            }


            Vec2 neighborEmbedding =
                hyperbolicEmbedding(
                    scenario.spacecraft[neighbor],
                    neighborRisk
                );


            double hyperbolicDistance =
                poincareDistance(
                    currentEmbedding,
                    neighborEmbedding
                );


            // Local signal strength.
            //
            // Stronger local risk increases the signal.
            // Greater hyperbolic separation weakens it.
            double signal =
                localRisk /
                (1.0 + hyperbolicDistance);


            // Adaptive network connection.
            //
            // The edge responds to the local signal rather
            // than remaining permanently fixed.
            edge.strength =
                std::clamp(
                    0.95 * edge.strength +
                    0.05 * signal,
                    0.0,
                    1.0
                );


            // Local maneuver decision.
            //
            // There is no central authority selecting
            // which spacecraft acts.
            if (neighborRisk > 0.0 &&
                edge.strength > 0.15) {

                Vec2 direction =
                    avoidanceDirection(
                        scenario.spacecraft[i],
                        scenario.spacecraft[neighbor]
                    );


                double maneuverMagnitude =
                    0.01 * edge.strength;


                Vec2 deltaV =
                    direction *
                    maneuverMagnitude;


                scenario.spacecraft[i].velocity =
                    scenario.spacecraft[i].velocity +
                    deltaV;


                scenario.spacecraft[i].maneuverCost +=
                    norm(deltaV);
            }
        }
    }
}


// ------------------------------------------------------------
// RUN SIMULATION
// ------------------------------------------------------------

Metrics runSimulation(
    Scenario scenario,
    bool distributed
) {

    std::vector<Edge> edges;

    // Initially every spacecraft can communicate.
    for (int i = 0;
         i < static_cast<int>(scenario.spacecraft.size());
         ++i) {

        for (int j = i + 1;
             j < static_cast<int>(scenario.spacecraft.size());
             ++j) {

            edges.push_back({
                i,
                j,
                1.0
            });
        }
    }


    Metrics metrics;


    for (int step = 0;
         step < scenario.steps;
         ++step) {

        if (distributed) {

            distributedMycelialController(
                scenario,
                edges
            );

        } else {

            centralizedController(
                scenario
            );
        }


        // Propagate all spacecraft.
        for (auto& spacecraft :
             scenario.spacecraft) {

            propagate(
                spacecraft,
                scenario.dt
            );
        }


        // Measure network state.
        for (int i = 0;
             i < static_cast<int>(
                     scenario.spacecraft.size());
             ++i) {

            for (int j = i + 1;
                 j < static_cast<int>(
                         scenario.spacecraft.size());
                 ++j) {

                double distance =
                    distanceBetween(
                        scenario.spacecraft[i],
                        scenario.spacecraft[j]
                    );


                metrics.minimumSeparation =
                    std::min(
                        metrics.minimumSeparation,
                        distance
                    );


                if (distance <
                    scenario.collisionRadius) {

                    metrics.collision = true;
                }
            }
        }
    }


    for (const auto& spacecraft :
         scenario.spacecraft) {

        metrics.totalManeuverCost +=
            spacecraft.maneuverCost;

        if (spacecraft.maneuverCost > 0.0) {

            metrics.maneuveringSpacecraft++;
        }
    }


    return metrics;
}


// ------------------------------------------------------------
// EXPERIMENT
// ------------------------------------------------------------

Scenario createScenario() {

    Scenario scenario;


    // Controlled test case.
    //
    // These are normalized simulation coordinates,
    // NOT real orbital parameters.

    scenario.spacecraft = {

        {
            0,
            {-10.0, 0.0},
            {0.10, 0.00}
        },

        {
            1,
            {10.0, 0.2},
            {-0.10, 0.00}
        },

        {
            2,
            {0.0, 8.0},
            {0.00, -0.02}
        }
    };


    return scenario;
}


// ------------------------------------------------------------
// MAIN
// ------------------------------------------------------------

int main() {

    Scenario scenario =
        createScenario();


    Metrics centralized =
        runSimulation(
            scenario,
            false
        );


    Metrics mycelial =
        runSimulation(
            scenario,
            true
        );


    std::cout
        << std::fixed
        << std::setprecision(6);


    std::cout
        << "\n=====================================\n"
        << " CENTRALIZED COORDINATION\n"
        << "=====================================\n";


    std::cout
        << "Collision: "
        << centralized.collision
        << "\n";


    std::cout
        << "Minimum separation: "
        << centralized.minimumSeparation
        << "\n";


    std::cout
        << "Total maneuver cost: "
        << centralized.totalManeuverCost
        << "\n";


    std::cout
        << "Maneuvering spacecraft: "
        << centralized.maneuveringSpacecraft
        << "\n";


    std::cout
        << "\n=====================================\n"
        << " MYCELIUM + HYPERBOLIC NETWORK\n"
        << "=====================================\n";


    std::cout
        << "Collision: "
        << mycelial.collision
        << "\n";


    std::cout
        << "Minimum separation: "
        << mycelial.minimumSeparation
        << "\n";


    std::cout
        << "Total maneuver cost: "
        << mycelial.totalManeuverCost
        << "\n";


    std::cout
        << "Maneuvering spacecraft: "
        << mycelial.maneuveringSpacecraft
        << "\n";


    // Save results for later statistical analysis.
    std::ofstream results(
        "results.csv"
    );


    results
        << "architecture,"
        << "collision,"
        << "minimum_separation,"
        << "maneuver_cost,"
        << "maneuvering_spacecraft\n";


    results
        << "centralized,"
        << centralized.collision
        << ","
        << centralized.minimumSeparation
        << ","
        << centralized.totalManeuverCost
        << ","
        << centralized.maneuveringSpacecraft
        << "\n";


    results
        << "mycelium_hyperbolic,"
        << mycelial.collision
        << ","
        << mycelial.minimumSeparation
        << ","
        << mycelial.totalManeuverCost
        << ","
        << mycelial.maneuveringSpacecraft
        << "\n";


    std::cout
        << "\nResults saved to results.csv\n";


    return 0;
}
