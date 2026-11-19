// 05_interpretador.h — Interpretador de Nível 1 da Linguagem Musical (Salmus).
//
// Consome a sequência de comandos produzida por 04_parser.h e produz a
// "representação estruturada da composição" decidida em docs/01_recorte.md
// ("O que o sistema produz — Nível 1"): a lista ordenada de eventos
// musicais, cada um com nota, oitava e duração, com efeito/volume anexados
// quando presentes.
//
// Também é aqui — não no léxico nem no sintático — que vivem as duas
// checagens descritas em especificacao/padrao_arvore.md, seção 5:
//
//   1. BPM entre 20 e 300: poderia ser reconhecida por um autômato finito
//      (é um padrão regular sobre os dígitos), mas como o programa já tem
//      o valor como inteiro nesta etapa, validamos com uma comparação
//      numérica direta — o efeito observável é o mesmo.
//   2. Identificador de nota referenciado em "fx"/"amp" precisa ter sido
//      declarado antes por um "!": este requisito o próprio documento
//      identifica como fora da classe regular (exige memória ilimitada de
//      identificadores já vistos), por isso é resolvido aqui com uma
//      tabela de símbolos (std::unordered_map), não no autômato do léxico.

#ifndef LINGUAGEM_MUSICAL_05_INTERPRETADOR_H
#define LINGUAGEM_MUSICAL_05_INTERPRETADOR_H

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "04_parser.h"

namespace linguagem_musical {

struct Evento {
    std::string idNota;
    std::string nota;
    long oitava = 0;
    std::string duracao;

    std::optional<std::string> volumeTexto;    // de "amp", ex.: "+3"
    std::optional<std::string> efeitoNome;     // de "fx", ex.: "reverb"
    std::optional<std::string> efeitoParametroTexto;  // ex.: "0.3"
};

struct SaidaInterpretacao {
    std::optional<long> bpm;
    std::optional<std::string> instrumento;
    std::vector<Evento> eventos;

    // Mensagens de "wait" já formatadas, na ordem em que os comandos
    // "wait" apareceram no programa (podem intercalar com os eventos).
    std::vector<std::string> mensagensEspera;
};

struct ErroSemantico {
    std::string mensagem;
    std::size_t linha = 0;
};

// Executa os comandos em ordem e monta a saída de Nível 1.
// Lança ErroSemantico em BPM fora de [20, 300], identificador de nota
// referenciado antes de ser declarado, ou nota declarada duas vezes.
SaidaInterpretacao interpretar(const std::vector<Comando>& comandos);

// Formata a saída no texto ilustrado em docs/exemplo.icxc
// ("RESULTADO ESPERADO — saída de Nível 1").
std::string formatarSaida(const SaidaInterpretacao& saida);

}  // namespace linguagem_musical

#endif  // LINGUAGEM_MUSICAL_05_INTERPRETADOR_H
