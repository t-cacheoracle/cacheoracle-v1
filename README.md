## A semantic cache layer that adjusts the cache as you prompt more, increasing accruacy and reduces latency + cost for LLM applications copmared to existing solutions.

Rather than storing verbatim LLM responses (exact-match caching) or returning potentially incorrect cached results for semantically similar prompts (semantic caching), generative caching identifies structural patterns across prompt-response pairs and synthesizes a lightweight Python program that can generate correct, variation-aware responses locally without calling the LLM.

This approach is based on the research paper GenCache: Generative Caching for Structurally Similar Prompts and Responses (NeurIPS 2025, Microsoft Research / UIUC), which demonstrated 83–98% cache hit rates and up to 34% reduction in end-to-end latency on agentic workflows.

## Workflow
The appraoch used for this project is based off the workflow below, with modifications to the caching system and prompt/result storage validation.
<img width="1316" height="1200" alt="image" src="https://github.com/user-attachments/assets/3090767f-5ee0-47d6-95a0-e9f9af3a10f1" />
Figure 2.a, (NeurIPS 2025, Microsoft Research / UIUC)

Using 3 LRU caches, One for storing semantic value and prompt/response, another for clustering with no programs, and lastly for clusters with programs.

The process begins with a prompt, we generate it's semantic value using a sentence transformer in Python.
This value is then compared with the cluster with programs, to see if there is a cluster with an existing program that can be ran immediately.
Then it is compared with the standalone prompts/responses, to see if there is an very close/exact semantic match, where we can immediatley return a value.
If none, it makes it's own record inside the cache, then checking with the clusters with no programs to see if it needs it's own cluster.

Once a cluster with no programs reaches a threshold number, it well then call into a code generator to create a program based off all the prompt and values within the cluster with no programs. It is then moved into the clusters with programs cache and then clean up occurs for the rest of the caches.

 ## Target
 The current implementation is mainly targeted for repeated questioning. Eg: Customer service questions, web navigation and chatbot agents. More complex agents such as coding agents and tool calling might not work well with the semantic cache model of the standalone prompts/responses cache. Although this can be supported by removing said layer, and adjusting the code generator to support tools calls within the generated program.

 ## How to build
 cmake -S src -B build && cmake --build build -- -j 4
 cd build
 cmake ../src -DgRPC_DIR=/opt/homebrew/Cellar/grpc/1.78.1_3/lib/cmake/grpc -DProtobuf_DIR=/opt/homebrew/Cellar/protobuf/34.1/lib/cmake/protobuf
 make -j4

 Run server
./cacheoracle 0.0.0.0:50051

Run gRPC test client
./build/cacheoracle_client localhost:50051 "hello grpc"


