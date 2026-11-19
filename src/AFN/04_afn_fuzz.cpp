// 04_afn_fuzz.cpp — teste diferencial da construção de Thompson.
//
// Gera padrões aleatórios sobre {a,b,c} usando TODOS os operadores que a
// leitura aceita (|, *, +, ?, {n}, {n,}, {n,m}, (), [..], [^..], .) e compara,
// para toda cadeia de {a,b,c} com até 5 símbolos, o veredito do AFN com o do
// std::regex (ECMAScript, regex_match). Qualquer divergência é impressa.
//
// Uso:  ./04_afn_fuzz [quantidade_de_padroes] [semente]

#include <iostream>
#include <random>
#include <regex>
#include <string>
#include <vector>

#include "04_afn.h"

using namespace linguagem_musical;

namespace {

std::mt19937 g_rng;

int sorteio(int n) { return static_cast<int>(g_rng() % static_cast<unsigned>(n)); }

std::string gerarAlternancia(int profundidade);

std::string gerarAtomo(int profundidade) {
    const int escolha = sorteio(profundidade > 0 ? 8 : 6);
    switch (escolha) {
        case 0: return "a";
        case 1: return "b";
        case 2: return "c";
        case 3: return ".";
        case 4: return "[ab]";
        case 5: return sorteio(2) ? "[^a]" : "[a-c]";
        default: return "(" + gerarAlternancia(profundidade - 1) + ")";
    }
}

std::string gerarRepeticao(int profundidade) {
    std::string atomo = gerarAtomo(profundidade);
    // O std::regex (referência) faz backtracking e explode em quantificadores
    // ilimitados aninhados, como (x*|y)+ — por isso um grupo que já tem *, + ou
    // {n,} por dentro só recebe quantificadores limitados. As formas aninhadas,
    // como (a*b*)* e (a*)*, estão na bateria escrita à mão (casos_afn.txt).
    const bool grupoComIlimitado =
        atomo.front() == '(' &&
        (atomo.find_first_of("*+") != std::string::npos || atomo.find(",}") != std::string::npos);
    int sorteado = sorteio(9);
    if (grupoComIlimitado && (sorteado == 0 || sorteado == 1 || sorteado == 4)) sorteado = 8;
    switch (sorteado) {
        case 0: return atomo + "*";
        case 1: return atomo + "+";
        case 2: return atomo + "?";
        case 3: return atomo + "{" + std::to_string(sorteio(3)) + "}";
        case 4: return atomo + "{" + std::to_string(sorteio(3)) + ",}";
        case 5: {
            const int n = sorteio(3);
            return atomo + "{" + std::to_string(n) + "," + std::to_string(n + sorteio(3)) + "}";
        }
        default: return atomo;
    }
}

std::string gerarConcatenacao(int profundidade) {
    std::string saida;
    const int partes = 1 + sorteio(3);
    for (int i = 0; i < partes; ++i) saida += gerarRepeticao(profundidade);
    return saida;
}

std::string gerarAlternancia(int profundidade) {
    std::string saida = gerarConcatenacao(profundidade);
    while (sorteio(4) == 0) saida += "|" + gerarConcatenacao(profundidade);
    return saida;
}

void cadeiasAte(std::size_t tamanhoMaximo, std::vector<std::string>& saida) {
    saida.push_back("");
    std::size_t inicio = 0;
    for (std::size_t t = 1; t <= tamanhoMaximo; ++t) {
        const std::size_t fim = saida.size();
        for (std::size_t i = inicio; i < fim; ++i) {
            if (saida[i].size() + 1 != t) continue;
            for (char c : {'a', 'b', 'c'}) saida.push_back(saida[i] + c);
        }
        inicio = fim;
    }
}

}  // namespace

int main(int argc, char* argv[]) {
    const int quantidade = (argc > 1) ? std::stoi(argv[1]) : 3000;
    g_rng.seed((argc > 2) ? static_cast<unsigned>(std::stoul(argv[2])) : 12345u);

    std::vector<std::string> cadeias;
    cadeiasAte(5, cadeias);

    int divergencias = 0, comparacoes = 0, recusados = 0;
    for (int k = 0; k < quantidade; ++k) {
        const std::string padrao = gerarAlternancia(3);
        const Resultado leitura = analisarExpressao(padrao);
        if (!leitura.ok) {
            // O gerador só produz padrões válidos: recusa aqui já é um achado.
            std::cout << "RECUSADO: " << padrao << " — " << leitura.erro.mensagem << '\n';
            ++recusados;
            continue;
        }
        const Afn afn = construirThompson(leitura.arvore);
        const std::regex referencia(padrao, std::regex::ECMAScript);
        for (const std::string& w : cadeias) {
            const bool meu = afn.aceita(w);
            const bool ref = std::regex_match(w, referencia);
            ++comparacoes;
            if (meu != ref) {
                ++divergencias;
                if (divergencias <= 20) {
                    std::cout << "DIVERGE: padrão " << padrao << "  cadeia \"" << w
                              << "\"  AFN=" << (meu ? "aceita" : "recusa")
                              << "  std::regex=" << (ref ? "aceita" : "recusa") << '\n';
                }
            }
        }
    }
    std::cout << quantidade << " padrões, " << comparacoes << " comparações, " << divergencias
              << " divergências, " << recusados << " padrões recusados\n";
    return (divergencias == 0 && recusados == 0) ? 0 : 1;
}
