#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <queue>
#include <algorithm>
#include <limits>
#include <filesystem>

using namespace std;

struct Grafo
{
    unordered_map<string, unordered_set<string>> adj;
    unordered_map<string, int> grauEntrada;
    unordered_set<string> vertices;

    void inserirVertice(const string &ip)
    {
        vertices.insert(ip);
        if (adj.find(ip) == adj.end()) { adj[ip] = {}; }
        if (grauEntrada.find(ip) == grauEntrada.end()) { grauEntrada[ip] = 0; }
    }

    bool inserirAresta(const string &origem, const string &destino)
    {
        inserirVertice(origem);
        inserirVertice(destino);
        auto [it, inserida] = adj[origem].insert(destino);
        if (inserida) { grauEntrada[destino]++; }
        return inserida;
    }

    int totalVertices() const { return (int)vertices.size(); }

    int totalArestas() const
    {
        int soma = 0;
        for (const auto &[ip, vizinhos] : adj) { soma += (int)vizinhos.size(); }
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

string nomeBase(const string& caminho) {
    return filesystem::path(caminho).filename().string();
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

vector<string> bfs(const Grafo& g, const string& origem, const string& destino)
{
    if (g.vertices.count(origem) == 0 || g.vertices.count(destino) == 0)
        return {};
 
    unordered_map<string, string> predecessor;
    queue<string> fila;
    unordered_set<string> visitado;
 
    fila.push(origem);
    visitado.insert(origem);
    predecessor[origem] = ""; 

    while (!fila.empty()) {
        string atual = fila.front();
        fila.pop();
 
        if (atual == destino) {
            vector<string> caminho;
            string no = destino;
            while (!no.empty()) {
                caminho.push_back(no);
                no = predecessor[no];
            }
            reverse(caminho.begin(), caminho.end());
            return caminho;
        }
 
        auto it = g.adj.find(atual);
        if (it == g.adj.end()) continue;
 
        for (const string& viz : it->second) {
            if (visitado.count(viz) == 0) {
                visitado.insert(viz);
                predecessor[viz] = atual;
                fila.push(viz);
            }
        }
    }
    return {};
}

 
int calcularDiametro(const Grafo& g) {
    int diametro = 0;
 
    for (const string& origem : g.vertices) {
        unordered_map<string, int> dist;
        queue<string> fila;
 
        dist[origem] = 0;
        fila.push(origem);
 
        while (!fila.empty()) {
            string atual = fila.front();
            fila.pop();
 
            auto it = g.adj.find(atual);
            if (it == g.adj.end()) continue;
 
            for (const string& viz : it->second) {
                if (dist.count(viz) == 0) {
                    dist[viz] = dist[atual] + 1;
                    if (dist[viz] > diametro) diametro = dist[viz];
                    fila.push(viz);
                }
            }
        }
    }
    return diametro;
}
 
void exibirTopRoteadores(const Grafo& g) {
    
    vector<pair<int, string>> lista;
    for (const auto& [ip, grau] : g.grauEntrada) {
        lista.push_back({grau, ip});
    }
 
    sort(lista.begin(), lista.end(),
              [](const auto& a, const auto& b){ return a.first > b.first; });
 
    cout << "\nTop 5 Roteadores Criticos:\n";
    cout << "  #  | Grau Entrada | IP\n";
    cout << "  ---+--------------+----------------------\n";
    int limite = min((int)lista.size(), 5);
    for (int i = 0; i < limite; i++) {
        cout << "  " << (i+1) << "  |      "
                  << lista[i].first << "       | "
                  << lista[i].second << "\n";
    }
}

string gerarDot(const Grafo& g, const vector<string>& caminho = {}) {
    
    unordered_set<string> nosDestacados;
    unordered_set<string> arestasDestacadas;  

    for (int i = 0; i < (int)caminho.size(); i++) {
        nosDestacados.insert(caminho[i]);
        if (i + 1 < (int)caminho.size()) {
            arestasDestacadas.insert(caminho[i] + "->" + caminho[i+1]);
        }
    }
 
    ostringstream dot;
    dot << "digraph {\n";
    dot << "    rankdir=LR;\n";
    dot << "    node [shape=ellipse, style=filled, fillcolor=lightblue];\n";
    dot << "    edge [color=gray];\n\n";
 
    for (const string& no : nosDestacados) {
        dot << "    \"" << no << "\" [fillcolor=orange, penwidth=2];\n";
    }
    if (!caminho.empty()) {
        dot << "    \"" << caminho.front() << "\" [fillcolor=green, penwidth=2];\n";
        dot << "    \"" << caminho.back()  << "\" [fillcolor=red,   penwidth=2];\n";
    }
    dot << "\n";
 
    for (const auto& [ip, vizinhos] : g.adj) {
        for (const string& viz : vizinhos) {
            string chave = ip + "->" + viz;
            if (arestasDestacadas.count(chave)) {
                dot << "    \"" << ip << "\" -> \"" << viz
                    << "\" [color=orange, penwidth=2.5];\n";
            } else {
                dot << "    \"" << ip << "\" -> \"" << viz << "\";\n";
            }
        }
    }
    dot << "}\n";
    return dot.str();
}

void executarGraphviz(const string& conteudoDot, const string& nomeArquivoEntrada, int formato) {
   
    //const string dotPath = "/tmp/graphroute_tmp.dot";
    const string dotPath = "graphroute_tmp.dot";
    ofstream dotFile(dotPath);
    dotFile << conteudoDot;
    dotFile.close();
 
    string base = nomeBase(nomeArquivoEntrada);
 
    if (formato == 1) {
        string cmd = "dot -Tx11 " + dotPath;
        system(cmd.c_str());
 
    } else if (formato == 2) {
    
        string saida = base + ".png";
        string cmd = "dot -Tpng " + dotPath + " -o " + saida;
        if (system(cmd.c_str()) == 0)
            cout << "Arquivo " << saida << " gerado com sucesso.\n";
        else
            cerr << "Erro ao gerar PNG.\n";
 
    } else if (formato == 3) {
        string saida = base + ".pdf";
        string cmd = "dot -Tpdf " + dotPath + " -o " + saida;
        if (system(cmd.c_str()) == 0)
            cout << "Arquivo " << saida << " gerado com sucesso.\n";
        else
            cerr << "Erro ao gerar PDF.\n";
    }
}

void opcaoDiametro(const Grafo& g) {
    int d = calcularDiametro(g);
    if (d == 0)
        cout << "\nO grafo nao possui caminhos entre vertices distintos (diametro = 0).\n";
    else
        cout << "\nDiametro do grafo: " << d << " salto(s).\n";
}
 
void opcaoRoteadoresCriticos(const Grafo& g) {
    exibirTopRoteadores(g);
}

void submenuGraphviz(const Grafo& g,
                     const string& nomeArquivoEntrada,
                     const vector<string>& caminho = {})
{
    cout << "\nSelecione o formato de saida do Graphviz:\n";
    cout << "  1. Tela\n";
    cout << "  2. Imagem (PNG)\n";
    cout << "  3. Documento (PDF)\n";
    cout << "Opcao: ";
 
    int opcao;
    cin >> opcao;
 
    if (opcao < 1 || opcao > 3) {
        cout << "Opcao invalida.\n";
        return;
    }
 
    string dot = gerarDot(g, caminho);
    executarGraphviz(dot, nomeArquivoEntrada, opcao);
}

void opcaoExibirGrafo(const Grafo& g, const string& nomeArquivo) {
    submenuGraphviz(g, nomeArquivo);
}
 
void opcaoMenorCaminho(const Grafo& g, const string& nomeArquivo) {
    string origem, destino;
    cout << "Digite o IP de Origem : ";
    cin >> origem;
    cout << "Digite o IP de Destino: ";
    cin >> destino;
 
    if (g.vertices.count(origem) == 0) {
        cout << "IP de origem nao encontrado no grafo: " << origem << "\n";
        return;
    }
    if (g.vertices.count(destino) == 0) {
        cout << "IP de destino nao encontrado no grafo: " << destino << "\n";
        return;
    }
 
    vector<string> caminho = bfs(g, origem, destino);
 
    if (caminho.empty()) {
        cout << "Sem conectividade entre " << origem << " e " << destino << ".\n";
        submenuGraphviz(g, nomeArquivo); 
        return;
    }
 
    int saltos = (int)caminho.size() - 1;
    cout << "\nCaminho encontrado (" << saltos << " salto"
              << (saltos != 1 ? "s" : "") << "):\n";
    for (int i = 0; i < (int)caminho.size(); i++) {
        if (i > 0) cout << " -> ";
        cout << caminho[i];
    }
    cout << "\n";
 
    submenuGraphviz(g, nomeArquivo, caminho);
}

void menuPrincipal(const Grafo& g, const string& nomeArquivo) {
    int opcao = -1;
    while (opcao != 0) {
        cout << "\n======================================================\n";
        cout << "1. Calcular o Diametro do Grafo\n";
        cout << "2. Identificar Roteadores Criticos\n";
        cout << "3. Exibir Grafo Completo\n";
        cout << "4. Encontrar Menor Caminho\n";
        cout << "0. Sair\n";
        cout << "======================================================\n";
        cout << "Escolha uma opcao: ";
        cin >> opcao;
 
        switch (opcao) {
            case 1: opcaoDiametro(g); break;
            case 2: opcaoRoteadoresCriticos(g); break;
            case 3: opcaoExibirGrafo(g, nomeArquivo); break;
            case 4: opcaoMenorCaminho(g, nomeArquivo); break;
            case 0: cout << "Encerrando.\n"; break;
            default: cout << "Opcao invalida.\n"; break;
        }
    }
}


int main()
{
    string caminho;
    int opcao = -1;
    cout << "\n===============Escolha o arquivo de log===============\n";
    cout << "1. input_1.log\n";
    cout << "2. input_2.log\n";
    cout << "3. input_3.log\n";
    cout << "0. Sair\n";
    cin >> opcao;
    switch (opcao) {
        case 1: caminho = "input_1.log"; cout << "Carregando arquivo: " << caminho << "\n"; break;
        case 2: caminho = "input_2.log"; cout << "Carregando arquivo: " << caminho << "\n"; break;
        case 3: caminho = "input_3.log"; cout << "Carregando arquivo: " << caminho << "\n"; break;
        case 0: cout << "Encerrando.\n"; break;                
        default: cout << "Opcao invalida.\n"; break;
    }
    Grafo g;

    if (!carregarLog(caminho, g))
    {
        return 1;
    }

    
    cout << "\n  Numero de vertices: " << g.totalVertices() << "\n";
    cout << "  Arestas inseridas: " << g.totalArestas() << "\n";
    
    menuPrincipal(g, caminho);
    return 0;
}