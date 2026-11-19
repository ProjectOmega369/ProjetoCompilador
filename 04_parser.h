// 04_parser.h — Analisador sintático da Linguagem Musical (Salmus).
//
// Consome a lista de tokens produzida por 03_lexer.h e reconhece a
// "Gramática Hospedeira" registrada em docs/01_recorte.md — a gramática do
// programa que o usuário final escreve (declarações + comandos musicais),
// que é livre de contexto, e não a mini-linguagem regular do núcleo
// (02_nucleo.h), que descreve apenas os PADRÕES usados para definir as
// classes léxicas.
//
// Implementado por descida recursiva: cada tipo de comando de
// especificacao/classes_lexicas.md ("Construções da linguagem") tem uma
// função de leitura própria.

#ifndef LINGUAGEM_MUSICAL_04_PARSER_H
#define LINGUAGEM_MUSICAL_04_PARSER_H

#include <cstddef>
#include <string>
#include <vector>

#include "03_lexer.h"

namespace linguagem_musical {

enum class TipoComando {
    DeclaracaoBpm,       // bpm ID "=" NUMERO
    DeclaracaoTimbre,    // "@" ID "=" TEXTO
    DeclaracaoNota,      // "!" ID "(" ID "," NUMERO "," ID ")"
    ChamadaEfeito,       // "fx" ID "(" ID "," NUMERO ")"
    ChamadaAmplificar,   // "amp" "(" ID "," NUMERO ")"
    Espera,              // "wait" NUMERO
};

// Um único nó da sequência de comandos. Em vez de uma hierarquia de classes
// (desnecessária para uma gramática tão pequena e sem aninhamento), usamos
// um struct "achatado": cada TipoComando usa só o subconjunto de campos
// comentado ao lado dele.
struct Comando {
    TipoComando tipo;
    std::size_t linha = 0;

    std::string nomeVariavel;  // DeclaracaoBpm / DeclaracaoTimbre: nome à esquerda do "="
    std::string valorTexto;    // DeclaracaoTimbre: conteúdo da string

    std::string idNota;        // DeclaracaoNota: nome que identifica a nota (ex.: nota1)
    std::string notaLetra;     // DeclaracaoNota: nota musical (ex.: C, E, G)
    std::string duracaoNome;   // DeclaracaoNota: duração (ex.: seminima, colcheia)
    long oitava = 0;           // DeclaracaoNota

    std::string nomeEfeito;    // ChamadaEfeito: nome do efeito (ex.: reverb)
    std::string idAlvo;        // ChamadaEfeito / ChamadaAmplificar: nota afetada
    std::string parametroTexto;// ChamadaEfeito: parâmetro bruto (ex.: "0.3")

    std::string deltaVolumeTexto;  // ChamadaAmplificar: variação bruta (ex.: "+3")

    long duracaoMs = 0;        // Espera
    long valorInteiro = 0;     // DeclaracaoBpm
};

struct ErroSintatico {
    std::string mensagem;
    std::size_t linha = 0;
};

// Reconhece o programa inteiro (sequência de comandos) e devolve os
// comandos na ordem em que aparecem no texto-fonte. Lança ErroSintatico
// no primeiro token que não se encaixa na gramática.
std::vector<Comando> analisar(const std::vector<Token>& tokens);

}  // namespace linguagem_musical

#endif  // LINGUAGEM_MUSICAL_04_PARSER_H
