#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct Grafo
{
    // Cada IP mapeia para o conjunto de IPs para os quais ele aponta.
    // Usamos unordered_set para evitar arestas duplicadas automaticamente.
    std::unordered_map<std::string, std::unordered_set<std::string>> adj;

    // Conjunto de todos os vértices únicos (IPs).
    std::unordered_set<std::string> vertices;

    // Insere um vértice sem arestas (garante que ele apareça no grafo).
    void inserirVertice(const std::string &ip)
    {
        vertices.insert(ip);
        // Garante entrada no mapa de adjacência, mesmo sem vizinhos.
        if (adj.find(ip) == adj.end())
        {
            adj[ip] = {};
        }
    }

    // Insere uma aresta dirigida de "origem" para "destino".
    // Retorna true se a aresta foi inserida (era nova), false se já existia.
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

    // Imprime a lista de adjacência completa para depuração.
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

// ─────────────────────────────────────────────────────────────────────────────
// Funções auxiliares
// ─────────────────────────────────────────────────────────────────────────────

// Remove espaços e caracteres invisíveis das bordas de uma string.
std::string trim(const std::string &s)
{
    const std::string brancos = " \t\r\n";
    size_t inicio = s.find_first_not_of(brancos);
    if (inicio == std::string::npos)
        return "";
    size_t fim = s.find_last_not_of(brancos);
    return s.substr(inicio, fim - inicio + 1);
}

// Divide uma linha CSV pelo delimitador e retorna as colunas.
std::vector<std::string> splitCSV(const std::string &linha, char delim = ',')
{
    std::vector<std::string> colunas;
    std::stringstream ss(linha);
    std::string campo;
    while (std::getline(ss, campo, delim))
    {
        colunas.push_back(trim(campo));
    }
    return colunas;
}

// ─────────────────────────────────────────────────────────────────────────────
// Leitura e sanitização do arquivo de log
// ─────────────────────────────────────────────────────────────────────────────

// Índices das colunas no CSV (baseado no cabeçalho do exemplo).
// prb_id,probe_src,dst_addr,hop,hop_from,hop_to,rtt
//   0        1        2      3     4        5    6
const int COL_HOP_FROM = 4;
const int COL_HOP_TO = 5;
const int COLUNAS_MIN = 6; // precisamos de ao menos 6 colunas (índice 0..5)

bool carregarLog(const std::string &caminho, Grafo &grafo)
{
    std::ifstream arquivo(caminho);
    if (!arquivo.is_open())
    {
        std::cerr << "Erro: não foi possível abrir o arquivo \"" << caminho << "\".\n";
        return false;
    }

    std::string linha;
    int numLinha = 0;
    int descartadas = 0;

    // Pula o cabeçalho (primeira linha).
    if (!std::getline(arquivo, linha))
    {
        std::cerr << "Erro: arquivo vazio ou sem cabeçalho.\n";
        return false;
    }
    numLinha++;

    while (std::getline(arquivo, linha))
    {
        numLinha++;

        // Ignora linhas totalmente em branco.
        if (trim(linha).empty())
        {
            descartadas++;
            continue;
        }

        auto colunas = splitCSV(linha);

        // Verifica se a linha tem colunas suficientes.
        if ((int)colunas.size() < COLUNAS_MIN)
        {
            std::cerr << "  [linha " << numLinha << "] Ignorada: colunas insuficientes.\n";
            descartadas++;
            continue;
        }

        const std::string &hopFrom = colunas[COL_HOP_FROM];
        const std::string &hopTo = colunas[COL_HOP_TO];

        // Sanitização 1: descarta se hop_from ou hop_to estiverem em branco.
        if (hopFrom.empty() || hopTo.empty())
        {
            descartadas++;
            continue;
        }

        // Sanitização 2: descarta timeouts (hop_to == "*").
        if (hopTo == "*")
        {
            descartadas++;
            continue;
        }

        // Aresta válida: insere no grafo.
        // Se a aresta já existia, inserirAresta retorna false (sem duplicata).
        grafo.inserirAresta(hopFrom, hopTo);
    }

    if (descartadas > 0)
    {
        std::cout << "  (" << descartadas << " linha(s) descartada(s) por sanitização)\n";
    }

    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Ponto de entrada
// ─────────────────────────────────────────────────────────────────────────────

int main(int argc, char *argv[])
{

    // Valida argumento de linha de comando.
    if (argc < 2)
    {
        std::cerr << "Uso: " << argv[0] << " <caminho_do_arquivo_de_log>\n";
        std::cerr << "Exemplo: " << argv[0] << " traceroute.log\n";
        return 1;
    }

    const std::string caminho = argv[1];
    std::cout << "Carregando arquivo: " << caminho << "\n";

    Grafo grafo;

    if (!carregarLog(caminho, grafo))
    {
        return 1; // mensagem de erro já foi exibida dentro de carregarLog
    }

    // Exibe resumo de sucesso.
    std::cout << "Carregamento concluído com sucesso!\n";
    std::cout << "  Vértices únicos : " << grafo.totalVertices() << "\n";
    std::cout << "  Arestas inseridas: " << grafo.totalArestas() << "\n";

    // Opcional: exibe a lista de adjacência.
    grafo.imprimir();

    return 0;
}
