// 04_demo.cpp — demonstração da construção de Thompson e da simulação.
//
// Parte 1: o traço do conjunto ativo antes do primeiro símbolo e depois de cada
//          um, para dois padrões pequenos. É no traço que o não determinismo
//          aparece.
// Parte 2: as classes léxicas do Salmus (especificacao/padrao_arvore.md) sobre
//          tokens reais do docs/exemplo.icxc, mostrando o tamanho da árvore, o
//          da máquina, o conjunto inicial e o maior conjunto ativo.
//
// Uso:  ./04_demo

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "04_afn.h"

using namespace linguagem_musical;

namespace {

void traco(const std::string& padrao, const std::string& cadeia) {
    const Resultado leitura = analisarExpressao(padrao);
    if (!leitura.ok) {
        std::cout << formatarErro(padrao, leitura.erro) << '\n';
        return;
    }
    const Afn afn = construirThompson(leitura.arvore);
    std::cout << "padrão " << padrao << "   árvore: " << formatarArvore(leitura.arvore) << "\n"
              << "máquina: " << afn.quantidadeDeEstados() << " estados, "
              << afn.quantidadeDeTransicoes() << " transições ("
              << afn.quantidadeDeTransicoesVazias() << " vazias)\n"
              << afn.formatarSimulacao(cadeia, afn.simular(cadeia)) << '\n';
}

// Preenche à direita até `largura` colunas; conta caracteres, não bytes UTF-8
// (std::setw erraria o alinhamento nas linhas com acento).
std::string coluna(const std::string& texto, std::size_t largura) {
    std::size_t colunas = 0;
    for (unsigned char c : texto) {
        if ((c & 0xC0) != 0x80) ++colunas;
    }
    return texto + std::string(colunas < largura ? largura - colunas : 0, ' ');
}

struct Linha {
    const char* classe;
    const char* padrao;
    const char* token;
};

}  // namespace

int main() {
    std::cout << "=== Parte 1: o traço do conjunto ativo ===\n\n";
    traco("a|b", "a");
    traco("(a|b)*c", "abc");

    std::cout << "=== Parte 2: classes léxicas do Salmus sobre tokens do exemplo.icxc ===\n\n";
    const std::vector<Linha> linhas = {
        {"Identificador", "[a-z][a-z0-9_]*", "instrumento"},
        {"Identificador", "[a-z][a-z0-9_]*", "1nome"},
        {"Número inteiro", "[0-9]+", "120"},
        {"Número com sinal", "[+-]?[0-9]+", "+3"},
        {"Número com sinal", "[+-]?[0-9]+", "+"},
        {"Texto entre aspas", "\"[^\"]*\"", "\"piano\""},
        {"Palavra fixa", "bpm|wait|amp|fx", "wait"},
        {"Sinal", "=|\\(|\\)", "="},
        {"BPM de 20 a 300", "[2-9][0-9]|1[0-9][0-9]|2[0-9][0-9]|300", "120"},
        {"BPM de 20 a 300", "[2-9][0-9]|1[0-9][0-9]|2[0-9][0-9]|300", "310"},
    };

    std::cout << coluna("classe", 19) << coluna("token", 14) << std::right << std::setw(5)
              << "nós" << std::setw(9) << "estados" << std::setw(9) << "inicial"
              << std::setw(8) << "maior" << "  veredito\n";
    for (const Linha& l : linhas) {
        const Resultado leitura = analisarExpressao(l.padrao);
        if (!leitura.ok) {
            std::cout << l.classe << ": padrão recusado — " << leitura.erro.mensagem << '\n';
            continue;
        }
        const Afn afn = construirThompson(leitura.arvore);
        const Simulacao s = afn.simular(l.token);
        std::cout << coluna(l.classe, 19) << coluna(l.token, 14) << std::right
                  << std::setw(5) << tamanho(leitura.arvore) << std::setw(9)
                  << afn.quantidadeDeEstados() << std::setw(9) << s.passos[0].ativos.size()
                  << std::setw(8) << s.maiorConjuntoAtivo << "  "
                  << (s.aceitou ? "ACEITA" : "RECUSA") << '\n';
    }
    std::cout << "\n(inicial = estados ativos antes do primeiro símbolo; maior = maior conjunto\n"
                 " ativo observado na simulação.)\n";
    return 0;
}
