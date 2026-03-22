#include "cache/cluster_cache_no_program.h"
#include "cache/cluster_cache_with_program.h"

#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

int main() {
    {
        ClusterCacheNoProgram cache(2);

        ClusterEmbedding c1 = {1.0, 2.0};
        ClusterEmbedding c2 = {3.0, 4.0};
        ClusterEmbedding c3 = {5.0, 6.0};

        QueryEmbeddingList q1 = {{0.1, 0.2}};
        QueryEmbeddingList q2 = {{0.3, 0.4}};
        QueryEmbeddingList q3 = {{0.5, 0.6}};

        cache.put(c1, q1);
        cache.put(c2, q2);

        QueryEmbeddingList out;
        bool found = cache.get(c1, &out);
        assert(found);
        assert(out == q1);

        cache.put(c3, q3);

        QueryEmbeddingList evicted_out;
        bool evicted_found = cache.get(c2, &evicted_out);
        assert(!evicted_found);

        bool still_found = cache.get(c1, &out);
        assert(still_found);
        assert(out == q1);

        assert(cache.size() == 2);
    }

    {
        ClusterCacheWithProgram cache(2);

        std::vector<double> c1 = {1.0};
        std::vector<double> c2 = {2.0};
        std::vector<double> c3 = {3.0};

        cache.put(c1, "print('one')", {10.0});
        cache.put(c2, "print('two')", {20.0});

        std::string program;
        std::vector<double> embedding;
        bool found = cache.get(c1, &program, &embedding);
        assert(found);
        assert(program == "print('one')");
        assert(embedding == std::vector<double>({10.0}));

        cache.put(c3, "print('three')", {30.0});

        bool evicted_found = cache.get(c2, &program, &embedding);
        assert(!evicted_found);

        bool still_found = cache.get(c1, &program, &embedding);
        assert(still_found);
        assert(program == "print('one')");

        {
            const std::string input_py_path = "cache_input_test.py";
            const std::string output_py_path = "cache_output_test.py";
            {
                std::ofstream input(input_py_path);
                assert(static_cast<bool>(input));
                input << "def cached_fn():\n";
                input << "    return 42\n";
            }

            bool put_file_ok = cache.putFromFile({9.0}, input_py_path, {90.0});
            assert(put_file_ok);

            std::vector<double> file_embedding;
            bool get_file_ok = cache.getToFile({9.0}, output_py_path, &file_embedding);
            assert(get_file_ok);
            assert(file_embedding == std::vector<double>({90.0}));

            std::ifstream output(output_py_path);
            assert(static_cast<bool>(output));
            std::string output_contents((std::istreambuf_iterator<char>(output)), std::istreambuf_iterator<char>());
            assert(output_contents.find("def cached_fn():") != std::string::npos);
            assert(output_contents.find("return 42") != std::string::npos);

            // std::remove(input_py_path.c_str());
            // std::remove(output_py_path.c_str());
        }
    }

    std::cout << "cluster cache tests passed\n";
    return 0;
}
