#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct Grafo
{
    std::unordered_map<std::string, std::unordered_set<std::string>> adj;

    std::unordered_set<std::string> vertices;

    void inserirVertice(const std::string &ip)
    {
        vertices.insert(ip);
        if (adj.find(ip) == adj.end())
        {
            adj[ip] = {};
        }
    }

    bool inserirAresta(const std::string &origem, const std::string &destino)
    {
        inserirVertice(origem);
        inserirVertice(destino);
        auto [it, inserida] = adj[origem].insert(destino);
        return inserida;
    }

    int totalVertices() const { return (int)vertices.size(); }

    int totalArestas() const
    {
        int soma = 0;
        for (const auto &[ip, vizinhos] : adj)
        {
            soma += (int)vizinhos.size();
        }
        return soma;
    }

    void imprimir() const
    {
        std::cout << "\nLista de Adjacência:\n";
        for (const auto &[ip, vizinhos] : adj)
        {
            std::cout << "  " << ip << " →";
            if (vizinhos.empty())
            {
                std::cout << " (sem saída)";
            }
            for (const auto &v : vizinhos)
            {
                std::cout << " " << v;
            }
            std::cout << "\n";
        }
    }
};
