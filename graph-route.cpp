#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std;

struct Grafo
{
    unordered_map<string, unordered_set<string>> adj;

    unordered_set<string> vertices;

    void inserirVertice(const string &ip)
    {
        vertices.insert(ip);
        if (adj.find(ip) == adj.end())
        {
            adj[ip] = {};
        }
    }

    bool inserirAresta(const string &origem, const string &destino)
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
        cout << "\nLista de Adjacência:\n";
        for (const auto &[ip, vizinhos] : adj)
        {
            cout << "  " << ip << " →";
            if (vizinhos.empty())
            {
                cout << " (sem saída)";
            }
            for (const auto &v : vizinhos)
            {
                cout << " " << v;
            }
            cout << "\n";
        }
    }
};

string trim(const string &s)
{
    const string brancos = " \t\r\n";
    size_t inicio = s.find_first_not_of(brancos);
    if (inicio == string::npos)
        return "";
    size_t fim = s.find_last_not_of(brancos);
    return s.substr(inicio, fim - inicio + 1);
}

vector<string> splitCSV(const string &linha, char delim = ',')
{
    vector<string> colunas;
    stringstream ss(linha);
    string campo;
    while (getline(ss, campo, delim))
    {
        colunas.push_back(trim(campo));
    }
    return colunas;
}

int main()
{
    Grafo g;

    g.inserirAresta("82.66.191.65", "192.168.3.1");
    g.inserirAresta("192.168.3.1", "194.149.162.248");
    g.inserirAresta("194.149.162.248", "194.149.162.250");

    cout << "total de vertices: " << g.totalVertices() << endl;
    cout << "total de arestas: " << g.totalArestas() << endl;
}