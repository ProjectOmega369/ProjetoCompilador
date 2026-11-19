// 05_afd_teste.cpp — testes de 05_afd (subconjuntos, minimização, comparação).
//
// Uso:  ./05_afd_teste [casos_afn.txt]
// Sai com código 0 se tudo passou, 1 caso contrário.
//
// Contagens de estados e vereditos foram calculados à mão no papel.

#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "05_afd.h"

using namespace linguagem_musical;

namespace {

int g_total = 0;
int g_falhas = 0;

void conferir(bool ok, const std::string& descricao) {
    ++g_total;
    if (!ok) {
        ++g_falhas;
        std::cout << "  FALHOU: " << descricao << '\n';
    }
}

void secao(const std::string& titulo) { std::cout << "\n== " << titulo << " ==\n"; }

Afn afnDe(const std::string& padrao) {
    const Resultado r = analisarExpressao(padrao);
    if (!r.ok) throw std::runtime_error("padrão de teste inválido: " + padrao);
    return construirThompson(r.arvore);
}

struct Contagens {
    std::size_t afn, afd, minimo;
};

Contagens contagens(const std::string& padrao, const std::string& alfabeto) {
    const Afn afn = afnDe(padrao);
    const Afd afd = determinizar(afn, alfabeto);
    return {afn.quantidadeDeEstados(), afd.quantidadeDeEstados(),
            minimizar(afd).quantidadeDeEstados()};
}

void testarMoverEFecho() {
    secao("movimento por símbolo e fecho vazio");
    // a|b: 0-1 = 'a', 2-3 = 'b', 4 = entrada, 5 = saída; ε: 4→0, 4→2, 1→5, 3→5.
    const Afn m = afnDe("a|b");
    conferir(m.mover({0, 2, 4}, 'a') == std::vector<Estado>({1}), "mover({0,2,4},'a') = {1}");
    conferir(m.mover({0, 2, 4}, 'b') == std::vector<Estado>({3}), "mover({0,2,4},'b') = {3}");
    conferir(m.mover({0, 2, 4}, 'c').empty(), "mover por símbolo ausente = {}");
    conferir(m.mover({4}, 'a').empty(), "mover não aplica o fecho vazio");
    conferir(m.fechoVazio(m.mover({0, 2, 4}, 'a')) == std::vector<Estado>({1, 5}),
             "fecho(mover) = {1,5}");
    conferir(m.mover({0, 0}, 'a') == std::vector<Estado>({1}), "sem repetição");
    conferir(m.alfabetoUsado() == "ab", "alfabetoUsado = ab");
}

void testarContagensConferidasAMao() {
    secao("contagens de estados conferidas à mão");
    {
        const Afn afn = afnDe("(a|b)*c");
        const Afd afd = determinizar(afn, "abc");
        RelatorioMinimizacao rel;
        const Afd min = minimizar(afd, &rel);
        conferir(afn.quantidadeDeEstados() == 10 && afn.quantidadeDeTransicoes() == 12 &&
                     afn.quantidadeDeTransicoesVazias() == 9,
                 "(a|b)*c: AFN 10 estados, 12 transições, 9 vazias");
        conferir(afd.quantidadeDeEstados() == 5, "(a|b)*c: AFD de 5 estados (com o erro)");
        conferir(afd.completo(), "(a|b)*c: o AFD da determinização já sai completo");
        conferir(min.quantidadeDeEstados() == 3 && min.completo(),
                 "(a|b)*c: mínimo completo de 3 estados");
        conferir(rel.estadosAntes == 5 && rel.estadosDepois == 3, "relatório 5 → 3");
        // Conjuntos do AFN que cada estado do AFD representa.
        conferir(afd.origem[afd.inicial] == std::vector<Estado>({0, 2, 4, 6, 7, 8}),
                 "estado inicial do AFD = fecho vazio do inicial do AFN = {0,2,4,6,7,8}");
        std::size_t vazios = 0;
        for (const auto& o : afd.origem) vazios += o.empty();
        conferir(vazios == 1, "exatamente um estado do AFD representa o conjunto vazio");
        for (std::size_t e = 0; e < afd.quantidadeDeEstados(); ++e) {
            if (afd.origem[e].empty()) {
                conferir(!afd.aceitacao[e], "o estado de erro não aceita");
                for (int d : afd.delta[e]) conferir(d == static_cast<int>(e), "erro volta ao erro");
            }
        }
    }
    const struct {
        const char* padrao;
        const char* alfabeto;
        std::size_t afd, minimo;
    } tabela[] = {
        {"a|b", "ab", 4, 3},   // {0,2,4}, {1,5}, {3,5}, {}; os dois finais se fundem
        {"a*", "a", 2, 1},     // {0,2,3} e {0,1,3} equivalem
        {"ab", "ab", 4, 4},    // início, depois de a, depois de ab, erro
        {"a", "a", 3, 3},      // início, aceita, erro
        {"(a|b)*", "ab", 3, 1},   // o inicial (com a entrada do fecho) fica isolado; depois de a e de b, 2 conjuntos
    };
    for (const auto& t : tabela) {
        const Contagens c = contagens(t.padrao, t.alfabeto);
        conferir(c.afd == t.afd && c.minimo == t.minimo,
                 std::string(t.padrao) + ": esperado AFD " + std::to_string(t.afd) +
                     " e mínimo " + std::to_string(t.minimo) + ", obtido " +
                     std::to_string(c.afd) + " e " + std::to_string(c.minimo));
    }
}

void testarMinimizacaoPorEtapas() {
    secao("minimização: completar, inacessíveis, partição, ponto fixo");
    {  // incompleto: 0 -a-> 1 (aceita); falta tudo o mais
        Afd a;
        a.alfabeto = "ab";
        a.delta = {{1, kSemTransicao}, {kSemTransicao, kSemTransicao}};
        a.aceitacao = {0, 1};
        conferir(!a.completo(), "AFD de partida incompleto");
        const Afd c = completar(a);
        conferir(c.quantidadeDeEstados() == 3 && c.completo(), "completar: +1 estado de erro");
        conferir(completar(c).quantidadeDeEstados() == 3, "completar é idempotente");
        conferir(a.aceita("a") && !a.aceita("b") && !a.aceita("aa") && !a.aceita(""),
                 "incompleto: só 'a' é aceita");
        RelatorioMinimizacao rel;
        const Afd m = minimizar(a, &rel);
        conferir(m.quantidadeDeEstados() == 3 && rel.depoisDeCompletar == 3,
                 "incompleto: mínimo de 3 estados (início, aceita, erro)");
        conferir(m.aceita("a") && !m.aceita("aa"), "mesma linguagem depois de minimizar");
    }
    {  // estado inacessível aceitando
        Afd a;
        a.alfabeto = "a";
        a.delta = {{1}, {1}, {2}};  // 2 inacessível
        a.aceitacao = {0, 1, 1};
        conferir(removerInacessiveis(a).quantidadeDeEstados() == 2, "descarta o inacessível");
        RelatorioMinimizacao rel;
        const Afd m = minimizar(a, &rel);
        conferir(rel.depoisDeInacessiveis == 2 && m.quantidadeDeEstados() == 2,
                 "mínimo de a+ = 2 estados");
    }
    {  // 1 e 2 equivalentes: 0 -a-> 1, 0 -b-> 2, ambos aceitam e vão a 3 (erro)
        Afd a;
        a.alfabeto = "ab";
        a.delta = {{1, 2}, {3, 3}, {3, 3}, {3, 3}};
        a.aceitacao = {0, 1, 1, 0};
        RelatorioMinimizacao rel;
        const Afd m = minimizar(a, &rel);
        conferir(m.quantidadeDeEstados() == 3 && rel.particaoInicial == 2,
                 "estados equivalentes se fundem: 4 → 3");
    }
    {  // refinamento que precisa de mais de uma rodada: contador módulo 3 em 'a'
        // com aceitação só no 0, escrito com 6 estados (ciclo duplicado) → 3.
        Afd a;
        a.alfabeto = "a";
        a.delta = {{1}, {2}, {3}, {4}, {5}, {0}};
        a.aceitacao = {1, 0, 0, 1, 0, 0};
        RelatorioMinimizacao rel;
        const Afd m = minimizar(a, &rel);
        conferir(m.quantidadeDeEstados() == 3 && rel.rodadas >= 2,
                 "módulo 6 com aceitação em 0 e 3 = módulo 3: 6 → 3, várias rodadas");
        conferir(m.aceita("") && m.aceita("aaa") && !m.aceita("a") && m.aceita("aaaaaa"),
                 "linguagem preservada");
    }
    {  // um estado só / nenhum estado
        Afd vazio;
        conferir(minimizar(vazio).quantidadeDeEstados() == 0, "AFD sem estados fica sem estados");
        conferir(!vazio.aceita(""), "AFD sem estados não aceita nada");
    }
    {  // símbolo fora do alfabeto
        const Afd d = determinizar(afnDe("a"), "a");
        conferir(d.aceita("a") && !d.aceita("b") && !d.aceita("ab"), "símbolo fora do alfabeto recusa");
    }
    {
        bool lancou = false;
        try {
            determinizar(afnDe("a"), std::string("a") + '\0');
        } catch (const std::invalid_argument&) {
            lancou = true;
        }
        conferir(lancou, "alfabeto com '\\0' é recusado");
        conferir(determinizar(afnDe("a"), "aaa").alfabeto == "a", "alfabeto repetido é reduzido");
    }
}

void testarComparacaoDasTres() {
    secao("comparação das três máquinas: (a|b)*c sobre {a,b,c}, até comprimento 4");
    const Afn afn = afnDe("(a|b)*c");
    const Afd afd = determinizar(afn, "abc");
    const Afd min = minimizar(afd);
    const ComparacaoDasTres c = compararTres(afn, afd, min, "abc", 4);
    // 1+3+9+27+81 = 121 cadeias; aceitas: terminam em c e o resto é {a,b}*:
    // 1+2+4+8 = 15 (c, ac, bc, aac, ..., até 3 símbolos antes do c).
    conferir(c.total == 121, "121 cadeias");
    conferir(c.aceitasAfn == 15 && c.aceitasAfd == 15 && c.aceitasMinimo == 15,
             "as três aceitam as mesmas 15");
    conferir(c.igual(), "nenhuma divergência em nenhum dos três pares");

    // Por que só as pontas não bastam: um mínimo defeituoso (o estado de erro
    // desviado para o final) concorda com... nada; mas um AFD defeituoso que
    // concorda com o AFN em parte das cadeias mostra o par que falhou.
    Afd quebrado = min;
    quebrado.aceitacao[quebrado.inicial] = 1;  // passa a aceitar a cadeia vazia
    const ComparacaoDasTres d = compararTres(afn, afd, quebrado, "abc", 4);
    conferir(!d.igual() && d.divergenciasAfnAfd == 0 && d.divergenciasAfdMinimo > 0 &&
                 d.divergenciasAfnMinimo > 0,
             "mínimo adulterado: o par AFN×AFD fecha, os outros dois divergem");
    conferir(!d.exemplos.empty() && d.exemplos[0].empty(),
             "o primeiro contra-exemplo é a cadeia vazia");
}

void testarExplosao() {
    secao("explosão: o n-ésimo símbolo a partir do fim é 'a'  (a|b)*a(a|b){n-1}");
    for (std::size_t n = 1; n <= 10; ++n) {
        const std::string padrao = "(a|b)*a(a|b){" + std::to_string(n - 1) + "}";
        const Afn afn = afnDe(padrao);
        const Afd afd = determinizar(afn, "ab");
        const Afd min = minimizar(afd);
        // O AFD guarda 2^n conjuntos distintos mais o conjunto inicial isolado
        // (o único em que o estado de entrada do fecho ainda está ativo sozinho);
        // o mínimo tem 2^n: é preciso lembrar as n últimas letras.
        conferir(afd.quantidadeDeEstados() == (std::size_t{1} << n) + 1,
                 padrao + ": AFD com 2^n+1 estados, obtido " + std::to_string(afd.quantidadeDeEstados()));
        conferir(min.quantidadeDeEstados() == (std::size_t{1} << n),
                 padrao + ": mínimo com 2^n estados, obtido " + std::to_string(min.quantidadeDeEstados()));
        // a{0} vira ε (2 estados), por isso n = 1 sai da reta; de n = 2 em diante 6n+4.
        if (n >= 2) {
            conferir(afn.quantidadeDeEstados() == 6 * n + 4, padrao + ": AFN cresce linearmente (6n+4)");
        }
        if (n <= 8) {
            conferir(compararTres(afn, afd, min, "ab", 10).igual(), padrao + ": três máquinas iguais");
        }
    }
    // Conferência à mão para n = 2: o AFD lembra os dois últimos símbolos.
    const Afd m = minimizar(determinizar(afnDe("(a|b)*a(a|b)"), "ab"));
    conferir(m.aceita("ab") && m.aceita("aa") && m.aceita("bab") && !m.aceita("ba") &&
                 !m.aceita("bb") && !m.aceita("b") && !m.aceita("a"),
             "n=2: aceita sse o penúltimo símbolo é a");
}

void testarBateria(const std::string& caminho) {
    secao("bateria de casos nas três máquinas: " + caminho);
    const std::vector<CasoDeTeste> casos = lerCasos(caminho);
    conferir(!casos.empty(), "o arquivo de casos abriu e tem casos");
    std::size_t passaram = 0;
    for (const CasoDeTeste& caso : casos) {
        if (!caso.erroDeLeitura.empty()) {
            conferir(false, caso.origem + ": " + caso.erroDeLeitura);
            continue;
        }
        const Resultado r = analisarExpressao(caso.expressao);
        if (!r.ok) {
            conferir(false, caso.origem + ": padrão recusado");
            continue;
        }
        const Afn afn = construirThompson(r.arvore);
        // Alfabeto da comparação: o do padrão mais os símbolos da cadeia.
        std::string alfabeto = afn.alfabetoUsado() + caso.cadeia;
        const Afd afd = determinizar(afn, alfabeto);
        const Afd min = minimizar(afd);
        const bool a = afn.aceita(caso.cadeia), d = afd.aceita(caso.cadeia),
                   m = min.aceita(caso.cadeia);
        const bool ok = (a == caso.esperado) && (d == caso.esperado) && (m == caso.esperado) &&
                        min.quantidadeDeEstados() <= afd.quantidadeDeEstados();
        if (!ok) {
            conferir(false, caso.origem + ": padrão \"" + caso.expressao + "\" cadeia \"" +
                                caso.cadeia + "\": esperado " + (caso.esperado ? "ACEITA" : "RECUSA") +
                                ", AFN=" + std::to_string(a) + " AFD=" + std::to_string(d) +
                                " mín=" + std::to_string(m));
        } else {
            ++passaram;
        }
    }
    conferir(passaram == casos.size(), "bateria: " + std::to_string(passaram) + "/" +
                                           std::to_string(casos.size()));
    std::cout << "  " << passaram << " de " << casos.size() << " casos passaram nas três máquinas\n";
}

void testarMinimoRealmenteMinimo() {
    secao("o mínimo não tem dois estados equivalentes (conferência por cadeias)");
    // Em um AFD mínimo completo, todo par de estados é separado por alguma
    // cadeia curta; testa-se até comprimento 6 nos padrões pequenos.
    for (const char* p : {"(a|b)*c", "a|b", "(ab)*", "a*b*", "(a|b)*abb", "a{2,4}"}) {
        const Afn afn = afnDe(p);
        std::string alfa = afn.alfabetoUsado();
        const Afd min = minimizar(determinizar(afn, alfa));
        std::set<std::vector<char>> assinaturas;
        std::vector<std::string> cadeias{""};
        for (std::size_t i = 0; i < cadeias.size() && cadeias[i].size() < 6; ++i) {
            for (char c : alfa) cadeias.push_back(cadeias[i] + c);
        }
        // Executa cada cadeia a partir de cada estado.
        std::set<std::vector<char>> distintos;
        for (std::size_t e = 0; e < min.quantidadeDeEstados(); ++e) {
            std::vector<char> perfil;
            for (const std::string& w : cadeias) {
                int q = static_cast<int>(e);
                for (char c : w) q = min.delta[q][min.alfabeto.find(c)];
                perfil.push_back(min.aceitacao[q]);
            }
            distintos.insert(perfil);
        }
        conferir(distintos.size() == min.quantidadeDeEstados(),
                 std::string(p) + ": todos os estados do mínimo são distinguíveis");
    }
}

}  // namespace

int main(int argc, char* argv[]) {
    const std::string casos = (argc > 1) ? argv[1] : "casos_afn.txt";

    testarMoverEFecho();
    testarContagensConferidasAMao();
    testarMinimizacaoPorEtapas();
    testarComparacaoDasTres();
    testarExplosao();
    testarMinimoRealmenteMinimo();
    testarBateria(casos);

    std::cout << "\n" << (g_total - g_falhas) << "/" << g_total << " verificações passaram";
    if (g_falhas) std::cout << " — " << g_falhas << " FALHARAM";
    std::cout << '\n';
    return g_falhas == 0 ? 0 : 1;
}
