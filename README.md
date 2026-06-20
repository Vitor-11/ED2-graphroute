# graph-route

Analisador de logs de traceroute com construção de Grafo Direcinado com menu interativo.  
Projeto desenvolvido para a disciplina de Estruturas de Dados II — UPF.

---

## Descrição

O programa lê arquivos de log no formato CSV gerados por medições de traceroute, constrói um **Grafo Direcionado Não Ponderado** em memória usando **Lista de Adjacência**, e disponibiliza um menu interativo com informações da topologia da rede analisada.

---

## Funcionalidades

---

| Opção | Descrição |
|-------|-----------|
| 1 | Calcular o diâmetro do grafo
| 2 | Identificar os Top 5 roteadores por grau de entrada (in-degree)
| 3 | Exibir o grafo completo via Graphviz (tela, PNG ou PDF)
| 4 | Encontrar o menor caminho entre dois IPs (BFS) com destaque visual
| 0 | Sair

---

## Como compilar e executar

**Pré-requisitos:** compilador com suporte a C++17 e [Graphviz](https://graphviz.org/download/) instalado.

```bash
# Compilar
g++ graph-route.cpp -o graph-route.exe

# Executar (o arquivo de log deve estar na mesma pasta)
./graph-route input_1.log
```
---

## Formato do arquivo de entrada

Arquivo CSV com cabeçalho obrigatório. As colunas relevantes são `hop_from` (coluna 4) e `hop_to` (coluna 5):

```
prb_id,probe_src,dst_addr,hop,hop_from,hop_to,rtt
1004740,82.66.191.65,20.157.222.42,1,82.66.191.65,192.168.3.1,0.203
1004740,82.66.191.65,20.157.222.42,2,192.168.3.1,194.149.162.248,7.273
1004740,82.66.191.65,20.157.222.42,3,194.149.162.248,194.149.162.250,7.494
```

### Regras de sanitização aplicadas na leitura

- Linhas onde `hop_to` é `*` são descartadas
- Linhas com `hop_from` ou `hop_to` em branco são descartadas
- Arestas duplicadas são ignoradas automaticamente

---

## Exemplo de uso

```
$ ./graph-route input_3.log

  Numero de vertices: 292
  Arestas inseridas: 296

======================================================
1. Calcular o Diametro do Grafo
2. Identificar Roteadores Criticos
3. Exibir Grafo Completo
4. Encontrar Menor Caminho
0. Sair
======================================================
Escolha uma opcao: 1

Diametro do grafo: 10 salto(s).

======================================================
1. Calcular o Diametro do Grafo
2. Identificar Roteadores Criticos
3. Exibir Grafo Completo
4. Encontrar Menor Caminho
0. Sair
======================================================
Escolha uma opcao: 2

Top 5 Roteadores Criticos:
  #  | Grau Entrada | IP
  ---+--------------+----------------------
  1  |      46       | 192.203.230.10
  2  |      3       | 192.168.0.1
  3  |      3       | 37.49.236.92
  4  |      2       | 208.53.207.54
  5  |      2       | 198.41.240.9

======================================================
1. Calcular o Diametro do Grafo
2. Identificar Roteadores Criticos
3. Exibir Grafo Completo
4. Encontrar Menor Caminho
0. Sair
======================================================
Escolha uma opcao: 0
Encerrando.
```

---

## Estrutura do código

```
graph-route.cpp
│
├── struct Grafo                  — estrutura principal (lista de adjacência)
│   ├── inserirVertice()          — garante que o IP existe no grafo
│   └── inserirAresta()           — insere aresta e atualiza grau de entrada
│   └── totalVertices()           — conta o total de vértices
│   └── totalArestas()            — conta o total de arestas
│   └── imprimir()                — imprime a lista de adjacencia em texto no terminal
│
├── trim() / splitCSV()           — sanitização e parsing do CSV
├── nomeBase()                    — extrai nome do arquivo sem diretório
│
├── carregarLog()                 — lê o arquivo e popula o grafo
│
├── bfs()                         — menor caminho (Busca em Largura)
├── calcularDiametro()            — diâmetro via BFS a partir de cada vértice
├── exibirTopRoteadores()         — Top 5 por grau de entrada
│
├── gerarDot()                    — gera conteúdo .dot com destaque de caminho
├── executarGraphviz()            — salva .dot e chama o Graphviz
├── submenuGraphviz()             — submenu de formato de saída (tela/PNG/PDF)
│
├── opcaoDiametro()               — handler da opção 1
├── opcaoRoteadoresCriticos()     — handler da opção 2
├── opcaoExibirGrafo()            — handler da opção 3
├── opcaoMenorCaminho()           — handler da opção 4
│
├── menuPrincipal()               — loop do menu interativo
└── main()                        — ponto de entrada, valida argc/argv
```

---

## Justificativa das escolhas de implementação

### Estrutura de dados: Lista de Adjacência com `unordered_map` + `unordered_set`

A Lista de Adjacência foi escolhida por ser a estrutura mais indicada para grafos onde o número de arestas é muito menor que o número máximo possível. Logs de traceroute geram exatamente esse perfil: centenas de IPs, mas cada um conectado a poucos vizinhos diretos.

Dentro da lista de adjacência, cada nó armazena seus vizinhos em um `unordered_set` em vez de um `vector`. Isso resolve o problema de **arestas duplicadas automaticamente**: como logs de traceroute repetem os mesmos saltos em medições diferentes, tentar inserir uma aresta já existente simplesmente não faz nada — sem precisar percorrer a lista procurando o elemento.

O `unordered_map` que indexa os nós por IP oferece busca em **tempo médio O(1)** (tabela hash).

### Algoritmo de menor caminho: BFS

A Busca em Largura (BFS) é o algoritmo correto para encontrar o **menor caminho em grafos não ponderados**. Como o grafo não tem pesos nas arestas — só interessa o número de saltos — o BFS garante que o primeiro caminho encontrado até o destino é sempre o mais curto.

O caminho é reconstruído a partir do mapa `predecessor`, que registra de qual nó chegamos a cada IP durante a exploração, permitindo percorrer o caminho de trás para frente ao chegar no destino.

### Diâmetro: BFS a partir de cada vértice

O diâmetro é o maior entre todos os menores caminhos do grafo. A abordagem adotada executa um BFS completo a partir de cada vértice, registrando as distâncias máximas alcançadas.

Para grafos da escala dos logs de traceroute (centenas de vértices), essa abordagem é perfeitamente viável.

### Grau de entrada: contador incremental

O `grauEntrada` é mantido como um `unordered_map<string, int>` atualizado a cada `inserirAresta()`. Isso evita ter que percorrer toda a lista de adjacência no momento da consulta — o custo é pago uma vez na inserção (O(1)), e a consulta final é apenas uma ordenação dos contadores já prontos.

### Visualização: formato DOT + Graphviz

O formato `.dot` é o padrão da ferramenta Graphviz, amplamente usada para visualização de grafos. Gerar o arquivo `.dot` programaticamente e delegar a renderização ao Graphviz evita implementar um motor gráfico do zero, mantendo o foco do código na lógica de grafos.

O destaque visual do caminho encontrado pelo BFS usa atributos nativos do Graphviz (`fillcolor`, `penwidth`, `color`), sem dependências externas adicionais.

---

## Dependências

- C++17 ou superior
- [Graphviz](https://graphviz.org/) (para as opções de saída visual)
