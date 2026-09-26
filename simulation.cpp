#include <cmath>
#include <algorithm>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#define API EMSCRIPTEN_KEEPALIVE
#else
#define API
#endif

/*
    INTELLIGENCE WITHOUT A CENTER

    C++ computational layer.

    Important:
    This is an engineering abstraction inspired by documented
    properties discussed in the supplied mycelium research.

    It does NOT claim to reproduce fungal intelligence.

    The model contains:

    1. Local node state
    2. Environmental disturbance
    3. Signal state
    4. Adaptive graph connections
    5. Hyperbolic graph distance
    6. Distributed network scoring

    All values are normalized simulation units.
*/

static constexpr int N = 6;
static constexpr int MAX_EDGES = 15;


struct Node {

    float x;
    float y;

    float vx;
    float vy;

    float risk;
    float signal;
};


static Node nodes[N];

static int edges[MAX_EDGES][2];

static float edge_strength[MAX_EDGES];

static int edge_count = 0;

static float simulation_time = 0.0f;

static float centralized_score = 0.0f;

static float distributed_score = 0.0f;


/*
    Poincare-disk hyperbolic distance.

    This is the mathematical hyperbolic component of the
    experimental network model.

    It is NOT a claim about biological fungal computation.
*/

static float hyperbolic_distance(
    int a,
    int b
) {

    const float x1 = nodes[a].x;
    const float y1 = nodes[a].y;

    const float x2 = nodes[b].x;
    const float y2 = nodes[b].y;


    const float r1 =
        std::min(
            0.92f,
            std::sqrt(
                x1*x1 +
                y1*y1
            )
        );


    const float r2 =
        std::min(
            0.92f,
            std::sqrt(
                x2*x2 +
                y2*y2
            )
        );


    const float dx =
        x1 - x2;

    const float dy =
        y1 - y2;


    const float euclidean_sq =
        dx*dx +
        dy*dy;


    const float denominator =
        std::max(
            0.00001f,
            (1.0f - r1*r1) *
            (1.0f - r2*r2)
        );


    const float argument =
        std::max(
            1.0f,
            1.0f +
            2.0f *
            euclidean_sq /
            denominator
        );


    return std::acosh(argument);
}


/*
    Add or update a graph connection.
*/

static void add_edge(
    int a,
    int b,
    float strength
) {

    if (a > b) {
        std::swap(a, b);
    }


    for (int i = 0; i < edge_count; i++) {

        if (
            edges[i][0] == a &&
            edges[i][1] == b
        ) {

            edge_strength[i] =
                strength;

            return;
        }
    }


    if (edge_count < MAX_EDGES) {

        edges[edge_count][0] =
            a;

        edges[edge_count][1] =
            b;

        edge_strength[edge_count] =
            strength;

        edge_count++;
    }
}


/*
    Rebuild adaptive graph.

    Connection strength combines:

    - hyperbolic proximity
    - similarity of local signal state

    This is an engineering rule.
*/

static void rebuild_graph() {

    edge_count = 0;


    for (int a = 0; a < N; a++) {

        for (int b = a + 1; b < N; b++) {

            const float hd =
                hyperbolic_distance(
                    a,
                    b
                );


            const float similarity =
                1.0f -
                std::min(
                    1.0f,
                    std::fabs(
                        nodes[a].signal -
                        nodes[b].signal
                    )
                );


            const float proximity =
                std::exp(
                    -hd
                );


            const float strength =
                0.65f *
                proximity
                +
                0.35f *
                similarity;


            if (strength > 0.34f) {

                add_edge(
                    a,
                    b,
                    strength
                );
            }
        }
    }
}


extern "C" {


API void reset_simulation() {

    /*
        Six nodes:

        0 = spacecraft A
        1 = spacecraft B
        2 = spacecraft C
        3 = spacecraft D
        4 = local AI
        5 = human operator
    */

    const float initial[N][2] = {

        {-0.70f, -0.25f},

        {-0.18f,  0.48f},

        { 0.48f,  0.20f},

        { 0.66f, -0.45f},

        {-0.25f, -0.52f},

        { 0.05f,  0.02f}

    };


    for (int i = 0; i < N; i++) {

        nodes[i].x =
            initial[i][0];

        nodes[i].y =
            initial[i][1];

        nodes[i].vx =
            0.0f;

        nodes[i].vy =
            0.0f;

        nodes[i].risk =
            0.15f +
            0.05f *
            i;

        nodes[i].signal =
            0.2f +
            0.1f *
            i;
    }


    simulation_time =
        0.0f;

    centralized_score =
        0.0f;

    distributed_score =
        0.0f;


    rebuild_graph();
}


/*
    One simulation step.

    The disturbance field is synthetic and deterministic.

    It is deliberately NOT presented as real orbital telemetry.
*/

API void step_simulation(
    float dt
) {

    simulation_time +=
        dt;


    for (int i = 0; i < N; i++) {

        /*
            Synthetic environmental disturbance.
        */

        const float disturbance =
            0.5f +
            0.5f *
            std::sin(
                simulation_time *
                0.9f
                +
                i *
                1.37f
            );


        /*
            Local risk state.
        */

        nodes[i].risk =
            0.12f +
            0.72f *
            disturbance;


        /*
            Synthetic local signal.
        */

        nodes[i].signal =
            0.5f +
            0.5f *
            std::sin(
                simulation_time *
                1.2f
                +
                i *
                0.83f
            );


        /*
            Movement inside normalized
            Poincare-disk coordinates.
        */

        const float angle =
            simulation_time *
            (0.07f + i * 0.009f)
            +
            i;


        nodes[i].x +=
            0.0045f *
            std::cos(angle);


        nodes[i].y +=
            0.0045f *
            std::sin(angle);


        /*
            Keep nodes inside a bounded
            normalized simulation region.
        */

        if (nodes[i].x > 0.9f)
            nodes[i].x = -0.9f;

        if (nodes[i].x < -0.9f)
            nodes[i].x = 0.9f;

        if (nodes[i].y > 0.65f)
            nodes[i].y = -0.65f;

        if (nodes[i].y < -0.65f)
            nodes[i].y = 0.65f;
    }


    /*
        Recompute network topology.
    */

    rebuild_graph();


    /*
        Aggregate environmental risk.
    */

    float total_risk =
        0.0f;


    for (int i = 0; i < N; i++) {

        total_risk +=
            nodes[i].risk;
    }


    /*
        Centralized reference score.

        This is a normalized computational
        reference, not an empirical performance claim.
    */

    centralized_score =
        std::max(
            0.0f,
            1.0f -
            total_risk /
            N
        );


    /*
        Distributed score.

        Network connectivity contributes to
        the experimental model.
    */

    float connected =
        0.0f;


    for (int e = 0; e < edge_count; e++) {

        connected +=
            edge_strength[e];
    }


    const float connectivity =
        connected /
        MAX_EDGES;


    distributed_score =
        std::max(
            0.0f,

            1.0f -
            total_risk /
            N *
            0.72f

            +

            connectivity *
            0.28f
        );
}


/* DATA ACCESS */

API int get_node_count() {

    return N;
}


API float get_node_x(
    int i
) {

    if (
        i < 0 ||
        i >= N
    ) return 0.0f;

    return nodes[i].x;
}


API float get_node_y(
    int i
) {

    if (
        i < 0 ||
        i >= N
    ) return 0.0f;

    return nodes[i].y;
}


API float get_node_risk(
    int i
) {

    if (
        i < 0 ||
        i >= N
    ) return 0.0f;

    return nodes[i].risk;
}


API float get_node_signal(
    int i
) {

    if (
        i < 0 ||
        i >= N
    ) return 0.0f;

    return nodes[i].signal;
}


API int get_edge_count() {

    return edge_count;
}


API int get_edge_a(
    int e
) {

    if (
        e < 0 ||
        e >= edge_count
    ) return -1;

    return edges[e][0];
}


API int get_edge_b(
    int e
) {

    if (
        e < 0 ||
        e >= edge_count
    ) return -1;

    return edges[e][1];
}


API float get_edge_strength(
    int e
) {

    if (
        e < 0 ||
        e >= edge_count
    ) return 0.0f;

    return edge_strength[e];
}


API float get_hyperbolic_distance(
    int a,
    int b
) {

    if (
        a < 0 ||
        a >= N ||
        b < 0 ||
        b >= N
    ) return 0.0f;

    return hyperbolic_distance(
        a,
        b
    );
}


API float get_centralized_score() {

    return centralized_score;
}


API float get_distributed_score() {

    return distributed_score;
}


API float get_time() {

    return simulation_time;
}


}
