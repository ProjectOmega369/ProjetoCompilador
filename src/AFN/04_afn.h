// 04_afn.h — o autômato finito não determinístico, a construção de Thompson que
// o produz a partir da árvore de um padrão, e a bateria de casos conferidos à
// mão.
//
// A construção tem um caso por construtor do núcleo (02_regex.h): três folhas
// (símbolo, cadeia vazia, linguagem vazia) e três operadores (concatenação,
// alternância, fecho). Não há caso para `+`, `?`, classe, agrupamento nem
// coringa: a leitura os reduz ao núcleo antes de a árvore chegar aqui, e é isso
// que mantém esta peça em seis casos. Em compensação, a construção aceita
// QUALQUER árvore bem formada do núcleo — não existe operador reservado que
// "fica de fora" e reaparece depois como falha silenciosa no analisador léxico.
//
// Todo caso devolve um bloco com exatamente uma entrada e uma saída; essa
// interface uniforme é o que deixa compor blocos sem olhar dentro deles. As
// transições vazias mantêm cada caso local, e em troca a máquina se executa com
// um conjunto de estados ativos, não com um estado corrente.
//
// Diferença para a interface de referência colada no enunciado: aqui NÃO existe
// o rótulo `kQualquer`. O nosso 02_regex.h expande `.` em alternância sobre Σ
// durante a leitura, então todo rótulo de transição é um símbolo concreto ou a
// transição vazia.

#ifndef LINGUAGEM_MUSICAL_04_AFN_H
#define LINGUAGEM_MUSICAL_04_AFN_H

#include <cstddef>
#include <string>
#include <vector>

#include "02_regex.h"

namespace linguagem_musical {

// Um estado é um índice na tabela de transições do AFN.
using Estado = std::size_t;

// Rótulo da transição vazia, que não consome símbolo. É o byte zero porque o
// zero termina cadeias de caracteres e não aparece num padrão escrito à mão.
// Consequência: um nó Simbolo com '\0' não tem como ser representado, e
// construirThompson() o recusa em vez de confundi-lo com ε.
inline constexpr char kEpsilon = '\0';

struct TransicaoAfn {
    char simbolo = kEpsilon;
    Estado destino = 0;
};

// Um bloco de máquina com uma entrada e uma saída — o invariante da construção.
struct Bloco {
    Estado entrada = 0;
    Estado saida = 0;
};

// Um passo do traço da simulação: os estados ativos antes do primeiro símbolo
// (simboloLido igual a '\0') ou logo depois de ler um símbolo.
struct PassoDaSimulacao {
    char simboloLido = '\0';
    std::vector<Estado> ativos;  // ordenados e sem repetição
};

struct Simulacao {
    bool aceitou = false;
    std::vector<PassoDaSimulacao> passos;
    std::size_t maiorConjuntoAtivo = 0;
};

class Afn {
public:
    Estado novoEstado();

    // Lançam std::out_of_range se algum estado não existe.
    void adicionarTransicao(Estado origem, char simbolo, Estado destino);
    void definirInicial(Estado estado);
    void definirAceitacao(Estado estado);

    std::size_t quantidadeDeEstados() const;
    std::size_t quantidadeDeTransicoes() const;
    std::size_t quantidadeDeTransicoesVazias() const;

    Estado inicial() const { return inicial_; }
    Estado aceitacao() const { return aceitacao_; }

    // O fecho vazio de um conjunto: todos os estados alcançáveis a partir dele
    // sem consumir símbolo (o próprio conjunto incluído). A simulação o aplica
    // depois de cada símbolo e também ao conjunto INICIAL, que é a aplicação
    // fácil de esquecer: sem ela, `a?` não aceitaria a cadeia vazia. Termina
    // mesmo com ciclos de transições vazias, como em fecho(fecho(x)).
    // Devolve ordenado e sem repetição.
    std::vector<Estado> fechoVazio(const std::vector<Estado>& conjunto) const;

    bool aceita(const std::string& cadeia) const;
    Simulacao simular(const std::string& cadeia) const;

    std::string formatarSimulacao(const std::string& cadeia,
                                  const Simulacao& simulacao) const;

private:
    // Expande `conjunto` in loco até o fecho vazio, deixando-o ordenado e sem
    // repetição. `marca` tem um elemento por estado; `geracao` identifica esta
    // chamada, de modo que a simulação reaproveita o vetor sem zerá-lo a cada
    // símbolo (custo proporcional ao conjunto ativo, não ao tamanho da máquina).
    void expandirFecho(std::vector<Estado>& conjunto,
                       std::vector<std::size_t>& marca,
                       std::size_t geracao) const;

    // Roda a cadeia; preenche `traco` somente se não for nulo.
    bool executar(const std::string& cadeia, Simulacao* traco) const;

    std::vector<std::vector<TransicaoAfn>> transicoes_;
    Estado inicial_ = 0;
    Estado aceitacao_ = 0;
};

// Constrói a máquina a partir da árvore devolvida por analisarExpressao().
// Aceita qualquer árvore bem formada do núcleo e a percorre sem recursão, então
// a altura da árvore não limita o tamanho do padrão.
// Lança std::invalid_argument se a árvore for malformada: sem raiz, índice de
// filho fora da árvore ou ausente num operador, ciclo entre nós, nó Simbolo com
// '\0', ou tipo de nó desconhecido. Nunca devolve uma máquina parcial.
Afn construirThompson(const Arvore& arvore);

// Um caso da bateria: o padrão, a cadeia e o veredito determinado sem rodar o
// programa.
struct CasoDeTeste {
    std::string expressao;
    std::string cadeia;
    bool esperado = false;
    std::string origem;         // arquivo:linha, citado na mensagem de falha
    std::string erroDeLeitura;  // não vazio: a linha estava malformada
};

struct ResultadoDaBateria {
    std::size_t total = 0;
    std::size_t passaram = 0;
    std::vector<std::string> falhas;
};

// Lê os casos do arquivo, um por linha, no formato
//     padrão <TAB> cadeia <TAB> aceita|rejeita
// A cadeia pode ser vazia (dois TABs seguidos). Linhas em branco e linhas
// iniciadas por '#' são ignoradas. Devolve lista vazia se o arquivo não abre.
// Uma linha malformada NÃO é descartada: vira um caso com erroDeLeitura, e
// rodarBateria() a conta como falha — descartar em silêncio esconderia o caso.
std::vector<CasoDeTeste> lerCasos(const std::string& caminho);

// Executa cada caso (leitura do padrão, construção, simulação) e acumula as
// falhas, sem parar na primeira.
ResultadoDaBateria rodarBateria(const std::vector<CasoDeTeste>& casos);

}  // namespace linguagem_musical

#endif  // LINGUAGEM_MUSICAL_04_AFN_H
