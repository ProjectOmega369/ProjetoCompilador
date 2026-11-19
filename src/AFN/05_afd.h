// 05_afd.h — do AFN ao AFD (construção de subconjuntos) e ao AFD mínimo
// (refinamento de partições), mais a comparação das três máquinas.
//
// O conjunto vazio É um estado: quando nenhum estado do AFN tem transição por
// um símbolo, o AFD vai para o estado "conjunto vazio", que é o estado de erro
// (todas as suas transições voltam para ele e ele não aceita). Assim o AFD da
// determinização já sai completo. Para (a|b)*c sobre {a,b,c} são 5 estados.
// A minimização continua completando o AFD recebido (completar()) para aceitar
// também máquinas incompletas montadas por outros meios.

#ifndef LINGUAGEM_MUSICAL_05_AFD_H
#define LINGUAGEM_MUSICAL_05_AFD_H

#include <cstddef>
#include <string>
#include <vector>

#include "04_afn.h"

namespace linguagem_musical {

inline constexpr int kSemTransicao = -1;

struct Afd {
    std::string alfabeto;                  // símbolos, sem repetição
    std::vector<std::vector<int>> delta;   // delta[estado][índice do símbolo]
    int inicial = 0;
    std::vector<char> aceitacao;           // aceitacao[estado] != 0
    // Só para o AFD da determinização: o conjunto de estados do AFN que cada
    // estado do AFD representa (ordenado). Vazio nas máquinas derivadas.
    std::vector<std::vector<Estado>> origem;

    std::size_t quantidadeDeEstados() const { return delta.size(); }
    bool completo() const;
    // Símbolo fora do alfabeto recusa a cadeia.
    bool aceita(const std::string& cadeia) const;
};

// Construção de subconjuntos. `alfabeto` é o alfabeto da comparação: símbolos
// repetidos são ignorados. Lança std::invalid_argument se contiver '\0'.
Afd determinizar(const Afn& afn, const std::string& alfabeto);

Afd completar(const Afd& afd);          // acrescenta o estado de erro se faltar
Afd removerInacessiveis(const Afd& afd);

struct RelatorioMinimizacao {
    std::size_t estadosAntes = 0;
    std::size_t depoisDeCompletar = 0;
    std::size_t depoisDeInacessiveis = 0;
    std::size_t particaoInicial = 0;
    std::size_t rodadas = 0;       // rodadas de refinamento até o ponto fixo
    std::size_t estadosDepois = 0;
};

// Completa, descarta os inacessíveis, parte em {aceitação, resto} e refina até
// o ponto fixo. O resultado é completo e o estado de erro é contado.
Afd minimizar(const Afd& afd, RelatorioMinimizacao* relatorio = nullptr);

std::string formatarAfd(const Afd& afd);

struct ComparacaoDasTres {
    std::size_t total = 0;
    std::size_t aceitasAfn = 0, aceitasAfd = 0, aceitasMinimo = 0;
    std::size_t divergenciasAfnAfd = 0;     // AFN x AFD
    std::size_t divergenciasAfdMinimo = 0;  // AFD x mínimo
    std::size_t divergenciasAfnMinimo = 0;  // AFN x mínimo
    std::vector<std::string> exemplos;      // até 10 cadeias divergentes
    bool igual() const {
        return divergenciasAfnAfd == 0 && divergenciasAfdMinimo == 0 &&
               divergenciasAfnMinimo == 0;
    }
};

// Roda as três máquinas sobre toda cadeia do alfabeto com comprimento de 0 a
// `comprimentoMaximo` e compara os três pares (não só as duas pontas).
ComparacaoDasTres compararTres(const Afn& afn, const Afd& afd, const Afd& minimo,
                               const std::string& alfabeto,
                               std::size_t comprimentoMaximo);

}  // namespace linguagem_musical

#endif  // LINGUAGEM_MUSICAL_05_AFD_H
