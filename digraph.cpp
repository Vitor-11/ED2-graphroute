#include <cstdlib>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>

namespace UPF
{

    template <typename T>
    class digraph
    {
    private:
        struct node
        {
            T value;
            std::vector<node *> links;
        };

        std::unordered_map<T, node> graph;

        node *find(const T &val)
        {
            auto it = graph.find(val);
            if (it == graph.end())
            { // Not found!
                return nullptr;
            }
            return &it->second;
        }

        void visit_node(node *p, std::unordered_set<node *> &visited, int level)
        {
            std::cout << std::string(level, '\t') << p->value << "\n";
            visited.insert(p);
            for (auto adj : p->links)
            {
                if (visited.count(adj) == 0)
                    visit_node(adj, visited, level + 1);
            }
        }

    public:
        void insert_node(const T &val)
        {
            if (graph.count(val) != 0)
                return;
            node aux;
            aux.value = val;
            graph[val] = aux;
        }

        void insert_link(const T &from, const T &to)
        {
            auto pfrom = find(from);
            if (!pfrom)
                return;
            auto pto = find(to);
            if (!pto)
                return;

            pfrom->links.push_back(pto);
        }

        void show()
        {
            std::ofstream dot("/tmp/g123456.dot");
            dot << "digraph{\n";
            for (const auto &[key, node] : graph)
            {
                dot << "\t\"" << key << "\" -> {";
                for (const auto &link : node.links)
                {
                    dot << "\"" << link->value << "\" ";
                }
                dot << "};\n";
            }
            dot << "}\n";
            dot.close();
            system("dot -Tx11 /tmp/g123456.dot");
        }

        void DFS_from(const T &from)
        {
            auto p = find(from);
            if (!p)
                return;

            std::unordered_set<node *> visited;
            visit_node(p, visited, 0);
        }
    };
}
