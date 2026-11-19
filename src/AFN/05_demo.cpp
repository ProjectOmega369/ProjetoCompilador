// 05_demo.cpp — do padrão ao AFN, ao AFD e ao AFD mínimo.
//
// Uso:  ./05_demo                      (exemplo (a|b)*c e tabela de explosão)
//       ./05_demo PADRAO ALFABETO [N]  (N = comprimento máximo da comparação, 4)

#include <iostream>
#include <string>

#include "05_afd.h"

using namespace linguagem_musical;

namespace {

bool analisar(const std::string& padrao, const std::string& alfabeto, std::size_t n,
              bool detalhado) {
    const Resultado leitura = analisarExpressao(padrao);
    if (!leitura.ok) {
        std::cout << formatarErro(padrao, leitura.erro) << '\n';
        return false;
    }
    const Afn afn = construirThompson(leitura.arvore);
    const Afd afd = determinizar(afn, alfabeto);
    RelatorioMinimizacao rel;
    const Afd minimo = minimizar(afd, &rel);

    std::cout << "padrão " << padrao << "  alfabeto \"" << alfabeto << "\"\n"
              << "  AFN:  " << afn.quantidadeDeEstados() << " estados, "
              << afn.quantidadeDeTransicoes() << " transições ("
              << afn.quantidadeDeTransicoesVazias() << " vazias)\n"
              << "  AFD:  " << afd.quantidadeDeEstados() << " estados (antes da minimização)\n"
              << "  mínimo: " << minimo.quantidadeDeEstados()
              << " estados (depois da minimização; completo, estado de erro contado)\n"
              << "  minimização: completar " << rel.estadosAntes << "→" << rel.depoisDeCompletar
              << ", inacessíveis →" << rel.depoisDeInacessiveis << ", partição inicial "
              << rel.particaoInicial << " blocos, " << rel.rodadas << " rodadas\n";
    if (detalhado) {
        std::cout << "AFD:\n" << formatarAfd(afd) << "AFD mínimo:\n" << formatarAfd(minimo);
    }
    const ComparacaoDasTres c = compararTres(afn, afd, minimo, alfabeto, n);
    std::cout << "  cadeias de comprimento até " << n << ": " << c.total
              << "; aceitas  AFN=" << c.aceitasAfn << " AFD=" << c.aceitasAfd
              << " mínimo=" << c.aceitasMinimo << "; divergências AFN×AFD="
              << c.divergenciasAfnAfd << " AFD×mín=" << c.divergenciasAfdMinimo
              << " AFN×mín=" << c.divergenciasAfnMinimo << "\n\n";
    return c.igual();
}

}  // namespace

int main(int argc, char* argv[]) {
    if (argc >= 3) {
        const std::size_t n = argc > 3 ? std::stoul(argv[3]) : 4;
        return analisar(argv[1], argv[2], n, true) ? 0 : 1;
    }

    std::cout << "=== (a|b)*c sobre {a,b,c} ===\n\n";
    bool ok = analisar("(a|b)*c", "abc", 4, true);

    std::cout << "=== explosão de estados: o n-ésimo símbolo a partir do fim é 'a' ===\n"
              << "padrão (a|b)*a(a|b){n-1} sobre {a,b}\n\n"
              << "   n   AFN   AFD  mínimo  2^n\n";
    for (std::size_t n = 1; n <= 12; ++n) {
        const std::string padrao = "(a|b)*a(a|b){" + std::to_string(n - 1) + "}";
        const Afn afn = construirThompson(analisarExpressao(padrao).arvore);
        const Afd afd = determinizar(afn, "ab");
        const Afd minimo = minimizar(afd);
        std::cout.width(4);  std::cout << n;
        std::cout.width(6);  std::cout << afn.quantidadeDeEstados();
        std::cout.width(6);  std::cout << afd.quantidadeDeEstados();
        std::cout.width(8);  std::cout << minimo.quantidadeDeEstados();
        std::cout.width(6);  std::cout << (std::size_t{1} << n) << '\n';
        if (n <= 8) ok = compararTres(afn, afd, minimo, "ab", 10).igual() && ok;
    }
    return ok ? 0 : 1;
}
