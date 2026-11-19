// 03_lexer.h — Analisador léxico da Linguagem Musical (Salmus).
//
// Recebe o texto-fonte de um programa .icxc (o programa que o USUÁRIO final
// da linguagem escreve — ver a "Gramática Hospedeira" em docs/01_recorte.md)
// e devolve a sequência de tokens reconhecidos, na ordem em que aparecem.
//
// As classes léxicas implementadas aqui são exatamente as descritas em
// especificacao/classes_lexicas.md: identificador, inteiro (com sinal
// opcional, usado em amp), float, string entre aspas, e as seis palavras/
// símbolos reservados (bpm, @, !, wait, amp, fx), além dos símbolos de
// pontuação (=, (, ), ,).
//
// Este módulo é independente do 02_nucleo.h: aquele converte um PADRÃO
// (uma expressão regular) em árvore; este converte um PROGRAMA (uma cadeia
// do alfabeto Σ) em tokens. Os dois compartilham o mesmo alfabeto, mas
// resolvem problemas diferentes.

#ifndef LINGUAGEM_MUSICAL_03_LEXER_H
#define LINGUAGEM_MUSICAL_03_LEXER_H

#include <cstddef>
#include <string>
#include <vector>

namespace linguagem_musical {

enum class TipoToken {
    Bpm,            // palavra reservada "bpm"
    Timbre,         // símbolo reservado "@"
    NotaTok,        // símbolo reservado "!"
    Fx,             // palavra reservada "fx"
    Amp,            // palavra reservada "amp"
    Wait,           // palavra reservada "wait"

    Identificador,  // [a-zA-Z_][a-zA-Z0-9_]*
    Numero,         // [+-]?[0-9]+ ou [+-]?[0-9]+"."[0-9]+  (lexema bruto preservado)
    Texto,          // "[^"]*"   (conteúdo já sem as aspas)

    Igual,          // =
    AbreParen,      // (
    FechaParen,     // )
    Virgula,        // ,

    FimDeArquivo,
};

struct Token {
    TipoToken tipo;
    std::string lexema;    // texto bruto do token (preserva sinal/ponto/aspas removidas)
    std::size_t linha = 1;
    std::size_t coluna = 1;
};

// Erro léxico: caractere ou construção que nenhuma classe léxica reconhece.
struct ErroLexico {
    std::string mensagem;
    std::size_t linha = 0;
    std::size_t coluna = 0;
};

// Percorre `fonte` de ponta a ponta e devolve os tokens encontrados,
// terminados por um token FimDeArquivo. Comentários de linha ("//" até o
// fim da linha) e espaços em branco são descartados e não geram token.
//
// Lança ErroLexico (por valor) ao encontrar um caractere fora do alfabeto
// Σ (especificacao/alfabeto.md) ou uma construção malformada (string ou
// comentário sem fechamento, símbolo isolado sem sentido léxico).
std::vector<Token> tokenizar(const std::string& fonte);

// Nome legível do tipo de token, usado em mensagens de erro do parser.
std::string nomeToken(TipoToken tipo);

}  // namespace linguagem_musical

#endif  // LINGUAGEM_MUSICAL_03_LEXER_H
