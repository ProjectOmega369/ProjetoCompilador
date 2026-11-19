// 02_regex.h — Leitura de uma expressão da Linguagem Musical e conversão em árvore.
//
// Recebe o texto de um pattern musical 
// que utilizarão o alfabeto para formar os tipos léxicos
// e devolve a árvore já reduzida ao
// núcleo mínimo. Nenhuma peça posterior precisa conhecer a notação de
// conveniência utilizada pelo usuário.
//
// O núcleo contém três operadores:
//   - concatenação;
//   - alternância;
//   - fecho.
//
// E três folhas:
//   - símbolo;
//   - cadeia vazia (ε);
//   - linguagem vazia (∅).
//
// O coringa `.` é aceito na entrada, mas é reduzido durante a leitura para
// uma alternância contendo todos os símbolos do alfabeto musical Σ.
//
// A árvore não contém nós para `+`, `?`, classes, agrupamentos ou coringa.
// Essas construções desaparecem durante a análise.

#ifndef LINGUAGEM_MUSICAL_02_REGEX_H
#define LINGUAGEM_MUSICAL_02_REGEX_H

#include <cstddef>
#include <string>
#include <vector>

namespace linguagem_musical {

inline constexpr std::size_t kSemFilho =
    static_cast<std::size_t>(-1);

// -----------------------------------------------------------------------------
// Núcleo mínimo
// -----------------------------------------------------------------------------

enum class TipoDeNo {
    Simbolo,        // folha: símbolo literal do alfabeto musical
    CadeiaVazia,    // folha: ε
    LinguagemVazia, // folha: ∅

    Concatenacao,   // operador: AB
    Alternancia,    // operador: A|B
    Fecho,          // operador: A*
};

struct No {
    TipoDeNo tipo = TipoDeNo::CadeiaVazia;

    // Significativo somente quando tipo == Simbolo.
    char simbolo = '\0';

    // Concatenacao e Alternancia usam os dois.
    // Fecho usa somente esquerda.
    std::size_t esquerda = kSemFilho;
    std::size_t direita = kSemFilho;
};

struct Arvore {
    std::vector<No> nos;
    std::size_t raiz = kSemFilho;

    bool vazia() const;
};



struct ErroDeSintaxe {
    std::size_t posicao = 0;
    std::string mensagem;
};

struct Resultado {
    bool ok = false;
    Arvore arvore;
    ErroDeSintaxe erro;
};

// -----------------------------------------------------------------------------
// Interface
// -----------------------------------------------------------------------------

// Lê a expressão musical e devolve a árvore reduzida ao núcleo,
// ou o primeiro erro encontrado.
Resultado analisarExpressao(const std::string& expressao);

// Forma prefixa canônica da árvore.
std::string formatarArvore(const Arvore& arvore);

// Formata o erro para apresentação ao usuário.
std::string formatarErro(
    const std::string& expressao,
    const ErroDeSintaxe& erro);

// Número de nós da árvore.
std::size_t tamanho(const Arvore& arvore);

}  // namespace linguagem_musical

#endif  // LINGUAGEM_MUSICAL_02_REGEX_H
