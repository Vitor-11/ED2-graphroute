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
        cout << "\nLista de Adjacencia:\n";
        for (const auto &[ip, vizinhos] : adj)
        {
            cout << "  " << ip << " ->";
            if (vizinhos.empty())
            {
                cout << " (sem saida)";
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

const int COL_HOP_FROM = 4;
const int COL_HOP_TO = 5;
const int COLUNAS_MIN = 6;

bool carregarLog(const string &caminho, Grafo &grafo)
{
    ifstream arquivo(caminho);
    if (!arquivo.is_open())
    {
        cerr << "ERRO: NÃO FOI POSSÍVEL ABRIR O CAMINHO \"" << caminho << "\".\n";
        return false;
    }

    string linha;
    int numLinha = 0;

    if (!getline(arquivo, linha))
    {
        cerr << "ERRO: ARQUIVO VAZIO OU SEM CABEÇALHO.\n";
        return false;
    }
    numLinha++;

    while (getline(arquivo, linha))
    {
        numLinha++;

        if (trim(linha).empty()) { continue;}

        auto colunas = splitCSV(linha);

        if ((int)colunas.size() < COLUNAS_MIN)
        {
            cerr << "  [linha " << numLinha << "] colunas insuficientes.\n";
            continue;
        }

        const string &hopFrom = colunas[COL_HOP_FROM];
        const string &hopTo = colunas[COL_HOP_TO];

        if (hopFrom.empty() || hopTo.empty()) { continue;}

        if (hopTo == "*") { continue;}

        grafo.inserirAresta(hopFrom, hopTo);
    }

    return true;
}

int main()
{
    Grafo g;

    if (!carregarLog("input_1.log", g)) { return 1;}
   // if (!carregarLog("input_2.log", g)) { return 1;}
   // if (!carregarLog("input_3.log", g)) { return 1;}

    cout << "Carregamento concluido\n";
    cout << "  Numero de vertices: " << g.totalVertices() << "\n";
    cout << "  Arestas inseridas: " << g.totalArestas() << "\n";

    g.imprimir();

    return 0;
}