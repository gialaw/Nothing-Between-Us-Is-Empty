const ALLOWED_ORIGIN =
    "https://gialaw.github.io";

const MODEL =
    "gpt-5.6-luna";


const SYSTEM_INSTRUCTIONS = `
You are the research assistant inside the
"Intelligence Without a Center" spacecraft
coordination research interface.

The project compares:

1. A documented operator-centered orbital
   collision-avoidance baseline.

2. An experimental decentralized architecture
   inspired by mycelial network behavior.

3. Hyperbolic graph computation as a mathematical
   representation used by the experimental model.

IMPORTANT SCIENTIFIC RULES:

Do not claim that fungi literally perform
hyperbolic computation.

Do not claim that this simulation reproduces
fungal intelligence.

Do not present the synthetic simulation values
as real spacecraft telemetry.

Distinguish documented research from the project's
engineering abstraction.

The supplied mycelium research discusses distributed
agency, environmental responsiveness and fungal
electrical activity, while explicitly noting that
fungal electrical signaling is an evolving field
and should not automatically be interpreted as
evidence of cognition or communication.

The orbital research supplied by the project discusses
spacecraft collision-avoidance maneuver design,
ground/operator workflows, autonomous approaches,
and constellation-scale challenges.

When discussing the simulation, use terms such as:

"experimental model"
"computational abstraction"
"normalized simulation value"
"engineering hypothesis"

Do not invent empirical spacecraft performance results.

If the user asks whether the experimental architecture
is better, explain what metrics would need to be tested
rather than declaring a winner.

You can analyze:

- graph connectivity
- node state
- adaptive topology
- distributed coordination
- hyperbolic distance
- normalized risk
- simulation behavior
- centralized versus distributed architecture
- research implications

You may perform arithmetic or reason over the numerical
simulation state provided by the interface.
`;


function corsHeaders() {

    return {

        "Access-Control-Allow-Origin":
            ALLOWED_ORIGIN,

        "Access-Control-Allow-Headers":
            "Content-Type",

        "Access-Control-Allow-Methods":
            "POST, OPTIONS",

        "Content-Type":
            "application/json"

    };
}


function response(body, status = 200) {

    return new Response(
        JSON.stringify(body),
        {
            status,
            headers: corsHeaders()
        }
    );
}


export default {

    async fetch(request, env) {

        if (
            request.method ===
            "OPTIONS"
        ) {

            return new Response(
                null,
                {
                    status: 204,
                    headers: corsHeaders()
                }
            );
        }


        if (
            request.method !==
            "POST"
        ) {

            return response(
                {
                    error:
                        "POST requests only."
                },
                405
            );
        }


        try {

            const body =
                await request.json();


            const question =
                String(
                    body.question ||
                    ""
                ).slice(
                    0,
                    4000
                );


            if (!question) {

                return response(
                    {
                        error:
                            "Question required."
                    },
                    400
                );
            }


            const selectedNode =
                body.selectedNode ||
                "none";


            const metrics =
                body.metrics ||
                {};


            const context = `
SELECTED NODE:
${selectedNode}

CURRENT SIMULATION STATE:
${JSON.stringify(metrics, null, 2)}

USER QUESTION:
${question}
`;


            const openAIResponse =
                await fetch(
                    "https://api.openai.com/v1/responses",
                    {

                        method: "POST",

                        headers: {

                            "Content-Type":
                                "application/json",

                            "Authorization":
                                `Bearer ${env.OPENAI_API_KEY}`

                        },

                        body: JSON.stringify({

                            model: MODEL,

                            instructions:
                                SYSTEM_INSTRUCTIONS,

                            input:
                                context,

                            max_output_tokens:
                                900

                        })

                    }
                );


            if (
                !openAIResponse.ok
            ) {

                const errorText =
                    await openAIResponse.text();

                console.error(
                    errorText
                );

                return response(
                    {
                        error:
                            "OpenAI request failed."
                    },
                    502
                );
            }


            const data =
                await openAIResponse.json();


            let answer =
                data.output_text;


            if (
                !answer &&
                Array.isArray(data.output)
            ) {

                answer =
                    data.output
                        .flatMap(
                            item =>
                                item.content || []
                        )
                        .filter(
                            item =>
                                item.type ===
                                "output_text"
                        )
                        .map(
                            item =>
                                item.text
                        )
                        .join("\n");
            }


            return response({

                answer:
                    answer ||
                    "No analysis returned."

            });


        } catch (error) {

            console.error(
                error
            );

            return response(
                {
                    error:
                        "Server error."
                },
                500
            );
        }
    }
};
