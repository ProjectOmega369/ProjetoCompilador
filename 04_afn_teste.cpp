// 04_afn_teste.cpp — testes de 04_afn (construção de Thompson + simulação).
//
// Uso:  ./04_afn_teste [casos_afn.txt]
// Sai com código 0 se tudo passou, 1 caso contrário.
//
// Todo veredito aqui foi determinado à mão, olhando a árvore ou a linguagem do
// padrão — nenhum foi copiado da saída do programa.

#include <cstdio>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "04_afn.h"

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

// Monta árvores do núcleo à mão, sem passar pela leitura.
struct Montador {
    Arvore a;

    std::size_t no(TipoDeNo tipo, char simbolo, std::size_t e, std::size_t d) {
        No n;
        n.tipo = tipo;
        n.simbolo = simbolo;
        n.esquerda = e;
        n.direita = d;
        a.nos.push_back(n);
        return a.nos.size() - 1;
    }
    std::size_t s(char c) { return no(TipoDeNo::Simbolo, c, kSemFilho, kSemFilho); }
    std::size_t eps() { return no(TipoDeNo::CadeiaVazia, '\0', kSemFilho, kSemFilho); }
    std::size_t vazia() { return no(TipoDeNo::LinguagemVazia, '\0', kSemFilho, kSemFilho); }
    std::size_t cat(std::size_t e, std::size_t d) { return no(TipoDeNo::Concatenacao, '\0', e, d); }
    std::size_t alt(std::size_t e, std::size_t d) { return no(TipoDeNo::Alternancia, '\0', e, d); }
    std::size_t fecho(std::size_t f) { return no(TipoDeNo::Fecho, '\0', f, kSemFilho); }
    Arvore pronto(std::size_t raiz) {
        a.raiz = raiz;
        return a;
    }
};

struct Contagem {
    std::size_t simbolos = 0, vazias = 0, linguagensVazias = 0, concats = 0, alts = 0, fechos = 0;
};

Contagem contar(const Arvore& t) {
    Contagem c;
    for (const No& n : t.nos) {
        switch (n.tipo) {
            case TipoDeNo::Simbolo: ++c.simbolos; break;
            case TipoDeNo::CadeiaVazia: ++c.vazias; break;
            case TipoDeNo::LinguagemVazia: ++c.linguagensVazias; break;
            case TipoDeNo::Concatenacao: ++c.concats; break;
            case TipoDeNo::Alternancia: ++c.alts; break;
            case TipoDeNo::Fecho: ++c.fechos; break;
        }
    }
    return c;
}

// Confere o veredito da máquina para uma cadeia.
void aceita(const Afn& m, const std::string& cadeia, bool esperado, const std::string& quem) {
    conferir(m.aceita(cadeia) == esperado,
             quem + " com \"" + cadeia + "\": esperado " + (esperado ? "ACEITA" : "RECUSA"));
}

bool lanca(const Arvore& t) {
    try {
        construirThompson(t);
    } catch (const std::invalid_argument&) {
        return true;
    } catch (...) {
        return false;
    }
    return false;
}

// ---------------------------------------------------------------------------

void testarCasosDaConstrucao() {
    secao("construção: um caso por construtor do núcleo");

    {  // símbolo: 2 estados, 1 transição
        Montador m;
        const Afn afn = construirThompson(m.pronto(m.s('a')));
        conferir(afn.quantidadeDeEstados() == 2, "símbolo: 2 estados");
        conferir(afn.quantidadeDeTransicoes() == 1, "símbolo: 1 transição");
        conferir(afn.quantidadeDeTransicoesVazias() == 0, "símbolo: nenhuma transição vazia");
        aceita(afn, "a", true, "'a'");
        aceita(afn, "", false, "'a'");
        aceita(afn, "b", false, "'a'");
        aceita(afn, "aa", false, "'a'");
    }
    {  // ε
        Montador m;
        const Afn afn = construirThompson(m.pronto(m.eps()));
        conferir(afn.quantidadeDeEstados() == 2 && afn.quantidadeDeTransicoes() == 1 &&
                     afn.quantidadeDeTransicoesVazias() == 1,
                 "ε: 2 estados, 1 transição vazia");
        aceita(afn, "", true, "ε");
        aceita(afn, "a", false, "ε");
        // Um '\0' na ENTRADA é um símbolo como outro qualquer, não a transição
        // vazia: aqui, tratá-lo como ε faria a máquina aceitar um símbolo.
        aceita(afn, std::string(1, '\0'), false, "ε com entrada \"\\0\"");
    }
    {  // ∅
        Montador m;
        const Afn afn = construirThompson(m.pronto(m.vazia()));
        conferir(afn.quantidadeDeEstados() == 2 && afn.quantidadeDeTransicoes() == 0,
                 "∅: 2 estados e nenhuma transição");
        aceita(afn, "", false, "∅");
        aceita(afn, "a", false, "∅");
    }
    {  // concatenação: não cria estado novo, só uma transição vazia
        Montador m;
        const Afn afn = construirThompson(m.pronto(m.cat(m.s('a'), m.s('b'))));
        conferir(afn.quantidadeDeEstados() == 4, "concat('a','b'): 4 estados");
        conferir(afn.quantidadeDeTransicoes() == 3 && afn.quantidadeDeTransicoesVazias() == 1,
                 "concat('a','b'): 3 transições, 1 vazia");
        aceita(afn, "ab", true, "concat('a','b')");
        aceita(afn, "a", false, "concat('a','b')");
        aceita(afn, "b", false, "concat('a','b')");
        aceita(afn, "ba", false, "concat('a','b')");
        aceita(afn, "", false, "concat('a','b')");
    }
    {  // alternância: entrada e saída novas, 4 transições vazias
        Montador m;
        const Afn afn = construirThompson(m.pronto(m.alt(m.s('a'), m.s('b'))));
        conferir(afn.quantidadeDeEstados() == 6, "alt('a','b'): 6 estados");
        conferir(afn.quantidadeDeTransicoes() == 6 && afn.quantidadeDeTransicoesVazias() == 4,
                 "alt('a','b'): 6 transições, 4 vazias");
        aceita(afn, "a", true, "alt('a','b')");
        aceita(afn, "b", true, "alt('a','b')");
        aceita(afn, "", false, "alt('a','b')");
        aceita(afn, "ab", false, "alt('a','b')");
        aceita(afn, "c", false, "alt('a','b')");
    }
    {  // fecho: entrada e saída novas, 4 transições vazias (pular e voltar)
        Montador m;
        const Afn afn = construirThompson(m.pronto(m.fecho(m.s('a'))));
        conferir(afn.quantidadeDeEstados() == 4, "fecho('a'): 4 estados");
        conferir(afn.quantidadeDeTransicoes() == 5 && afn.quantidadeDeTransicoesVazias() == 4,
                 "fecho('a'): 5 transições, 4 vazias");
        for (const char* w : {"", "a", "aa", "aaaa"}) aceita(afn, w, true, "fecho('a')");
        for (const char* w : {"b", "ab", "ba"}) aceita(afn, w, false, "fecho('a')");
    }
}

void testarLinguagemVaziaEmComposicao() {
    secao("∅ e ε dentro de operadores");
    {
        Montador m;
        const Afn afn = construirThompson(m.pronto(m.cat(m.vazia(), m.s('a'))));
        aceita(afn, "a", false, "concat(∅,'a')");
        aceita(afn, "", false, "concat(∅,'a')");
    }
    {
        Montador m;
        const Afn afn = construirThompson(m.pronto(m.cat(m.s('a'), m.vazia())));
        aceita(afn, "a", false, "concat('a',∅)");
        aceita(afn, "", false, "concat('a',∅)");
    }
    {
        Montador m;
        const Afn afn = construirThompson(m.pronto(m.alt(m.vazia(), m.s('a'))));
        aceita(afn, "a", true, "alt(∅,'a')");
        aceita(afn, "", false, "alt(∅,'a')");
    }
    {
        Montador m;
        const Afn afn = construirThompson(m.pronto(m.fecho(m.vazia())));
        aceita(afn, "", true, "fecho(∅)");
        aceita(afn, "a", false, "fecho(∅)");
    }
    {  // a? = alt('a', ε): só aceita "" por causa do fecho vazio do conjunto inicial
        Montador m;
        const Afn afn = construirThompson(m.pronto(m.alt(m.s('a'), m.eps())));
        aceita(afn, "", true, "alt('a',ε)");
        aceita(afn, "a", true, "alt('a',ε)");
        aceita(afn, "aa", false, "alt('a',ε)");
    }
}

void testarCiclosDeTransicaoVazia() {
    secao("ciclos de transições vazias (a simulação precisa terminar)");
    {
        Montador m;
        const Afn afn = construirThompson(m.pronto(m.fecho(m.eps())));
        aceita(afn, "", true, "fecho(ε)");
        aceita(afn, "a", false, "fecho(ε)");
    }
    {
        Montador m;
        const Afn afn = construirThompson(m.pronto(m.fecho(m.fecho(m.s('a')))));
        for (const char* w : {"", "a", "aaa"}) aceita(afn, w, true, "fecho(fecho('a'))");
        aceita(afn, "b", false, "fecho(fecho('a'))");
    }
    {
        Montador m;
        const Afn afn = construirThompson(m.pronto(m.fecho(m.alt(m.eps(), m.s('a')))));
        for (const char* w : {"", "a", "aa"}) aceita(afn, w, true, "fecho(alt(ε,'a'))");
        aceita(afn, "b", false, "fecho(alt(ε,'a'))");
    }
}

void testarSubarvoreCompartilhada() {
    secao("subárvore compartilhada (o índice repetido não pode virar um grafo)");
    // concat(x, x) com o MESMO índice nos dois filhos: a máquina precisa ter
    // duas cópias de x, não um estado visitado duas vezes.
    Montador m;
    const std::size_t x = m.s('a');
    const Afn afn = construirThompson(m.pronto(m.cat(x, x)));
    conferir(afn.quantidadeDeEstados() == 4, "concat(x,x): 4 estados (duas cópias de x)");
    aceita(afn, "aa", true, "concat(x,x)");
    aceita(afn, "a", false, "concat(x,x)");
    aceita(afn, "aaa", false, "concat(x,x)");

    Montador m2;
    const std::size_t y = m2.cat(m2.s('a'), m2.s('b'));
    const Afn afn2 = construirThompson(m2.pronto(m2.alt(y, m2.fecho(y))));
    aceita(afn2, "ab", true, "alt(y,fecho(y))");
    aceita(afn2, "abab", true, "alt(y,fecho(y))");
    aceita(afn2, "", true, "alt(y,fecho(y))");
    aceita(afn2, "aba", false, "alt(y,fecho(y))");
}

void testarArvoresMalformadas() {
    secao("árvores malformadas: recusa explícita, nunca máquina parcial");
    conferir(lanca(Arvore{}), "árvore vazia");
    {
        Montador m;
        m.s('a');
        Arvore t = m.a;
        t.raiz = 5;
        conferir(lanca(t), "raiz fora da árvore");
    }
    {
        Montador m;
        const std::size_t a = m.s('a');
        conferir(lanca(m.pronto(m.no(TipoDeNo::Alternancia, '\0', a, kSemFilho))),
                 "alternância sem filho direito");
    }
    {
        Montador m;
        m.s('a');
        conferir(lanca(m.pronto(m.no(TipoDeNo::Fecho, '\0', 99, kSemFilho))),
                 "fecho com filho fora da árvore");
    }
    {  // n0 = concat(n1, n1); n1 = fecho(n0): ciclo
        Arvore t;
        No n0;
        n0.tipo = TipoDeNo::Concatenacao;
        n0.esquerda = 1;
        n0.direita = 1;
        No n1;
        n1.tipo = TipoDeNo::Fecho;
        n1.esquerda = 0;
        t.nos = {n0, n1};
        t.raiz = 0;
        conferir(lanca(t), "ciclo entre dois nós");
    }
    {
        Arvore t;
        No n0;
        n0.tipo = TipoDeNo::Concatenacao;
        n0.esquerda = 0;
        n0.direita = 0;
        t.nos = {n0};
        t.raiz = 0;
        conferir(lanca(t), "nó que é filho de si mesmo");
    }
    {
        Montador m;
        conferir(lanca(m.pronto(m.s('\0'))), "símbolo '\\0' colide com a transição vazia");
    }
    {
        Montador m;
        m.s('a');
        Arvore t = m.a;
        t.nos[0].tipo = static_cast<TipoDeNo>(99);
        t.raiz = 0;
        conferir(lanca(t), "tipo de nó desconhecido");
    }
}

void testarTracoDaSimulacao() {
    secao("traço da simulação (numeração conferida à mão para alt('a','b'))");
    // Estados: 0-1 = 'a', 2-3 = 'b', 4 = entrada, 5 = saída.
    // ε: 4→0, 4→2, 1→5, 3→5.
    Montador m;
    const Afn afn = construirThompson(m.pronto(m.alt(m.s('a'), m.s('b'))));
    conferir(afn.inicial() == 4 && afn.aceitacao() == 5, "entrada 4, saída 5");

    Simulacao s = afn.simular("a");
    conferir(s.aceitou && s.passos.size() == 2, "\"a\": aceita em 2 passos");
    conferir(s.passos[0].ativos == std::vector<Estado>({0, 2, 4}), "\"a\": início {0,2,4}");
    conferir(s.passos[1].simboloLido == 'a' && s.passos[1].ativos == std::vector<Estado>({1, 5}),
             "\"a\": depois de 'a' {1,5}");
    conferir(s.maiorConjuntoAtivo == 3, "\"a\": maior conjunto ativo = 3");

    s = afn.simular("b");
    conferir(s.aceitou && s.passos[1].ativos == std::vector<Estado>({3, 5}),
             "\"b\": depois de 'b' {3,5}");

    s = afn.simular("c");
    conferir(!s.aceitou && s.passos.size() == 2 && s.passos[1].ativos.empty(),
             "\"c\": conjunto vazio no passo 1");

    s = afn.simular("ab");
    conferir(!s.aceitou && s.passos.size() == 3 && s.passos[2].ativos.empty(),
             "\"ab\": vazio depois do 'b'");

    conferir(afn.fechoVazio({4}) == std::vector<Estado>({0, 2, 4}), "fechoVazio({4}) = {0,2,4}");
    conferir(afn.fechoVazio({1}) == std::vector<Estado>({1, 5}), "fechoVazio({1}) = {1,5}");
    bool lancou = false;
    try {
        afn.fechoVazio({99});
    } catch (const std::out_of_range&) {
        lancou = true;
    }
    conferir(lancou, "fechoVazio de estado inexistente lança out_of_range");

    // Um '\0' na ENTRADA não pode casar com a transição vazia.
    conferir(!afn.aceita(std::string(1, '\0')), "'\\0' na entrada não é ε");

    std::cout << "  exemplo do traço para \"ab\":\n";
    const Simulacao demo = afn.simular("ab");
    std::cout << afn.formatarSimulacao("ab", demo);
}

Afn afnDe(const std::string& padrao) {
    const Resultado r = analisarExpressao(padrao);
    if (!r.ok) throw std::runtime_error("padrão de teste inválido: " + padrao);
    return construirThompson(r.arvore);
}

void testarNumerosDaTarefa2() {
    secao("simulação: conjunto inicial e maior conjunto ativo (Tarefa 2)");
    {
        const Afn m = afnDe("a|b");
        conferir(m.quantidadeDeEstados() == 6, "a|b: máquina de 6 estados");
        conferir(m.simular("").passos[0].ativos.size() == 3,
                 "a|b: conjunto ativo antes do primeiro símbolo tem 3 estados");
    }
    {
        const Afn m = afnDe("(a|b)*c");
        conferir(m.quantidadeDeEstados() == 10, "(a|b)*c: máquina de 10 estados");
        conferir(m.simular("").passos[0].ativos.size() == 6,
                 "(a|b)*c: conjunto ativo inicial tem 6 estados");
        const Simulacao s = m.simular("abbac");
        conferir(s.aceitou && s.maiorConjuntoAtivo == 7,
                 "(a|b)*c sobre abbac: aceita, com 7 estados ativos ao mesmo tempo");
    }
    // O defeito do fecho inicial esquecido só aparece em padrão que COMEÇA por
    // alternância ou fecho: aí o conjunto inicial tem mais de um estado.
    for (const char* p : {"a|b", "a*", "a?", "[ab]", "(a|b)*c", "[+-]?[0-9]+", "(ab)*"}) {
        const Afn m = afnDe(p);
        conferir(m.simular("").passos[0].ativos.size() > 1,
                 std::string("conjunto inicial de ") + p + " tem mais de um estado");
    }
    // Já um padrão que começa por símbolo tem conjunto inicial de 1 estado: é por
    // isso que testar só com "ab" ou "a.c" nunca revelaria o fecho inicial
    // esquecido.
    for (const char* p : {"ab", "abc", "a+", "a.c"}) {
        conferir(afnDe(p).simular("").passos[0].ativos.size() == 1,
                 std::string("conjunto inicial de ") + p + " tem exatamente 1 estado");
    }
    // Sem o fecho vazio inicial, a? não aceitaria "" — conferido pela bateria,
    // e aqui direto pelo conjunto: o estado de aceitação já está no inicial.
    {
        const Afn m = afnDe("a?");
        const std::vector<Estado> ini = m.simular("").passos[0].ativos;
        bool tem = false;
        for (Estado e : ini) tem = tem || e == m.aceitacao();
        conferir(tem && m.aceita(""), "a?: o estado de aceitação está no conjunto inicial");
    }
}

void testarLeituraDeCasosMalformados() {
    secao("bateria: linhas malformadas contam como falha, não somem");
    const std::string caminho = "casos_malformados_tmp.txt";
    {
        std::ofstream f(caminho);
        f << "# comentário ignorado\n"
             "\n"
             "a\ta\taceita\n"
             "a\t\trecusa\n"
             "a\tb\trejeita\n"   // sinônimo aceito
             "linha sem tabulações\n"
             "a\ta\ttalvez\n"
             "(a\ta\taceita\n";   // padrão recusado pela leitura
    }
    const std::vector<CasoDeTeste> casos = lerCasos(caminho);
    conferir(casos.size() == 6, "6 casos lidos (comentário e linha em branco ignorados)");
    const ResultadoDaBateria r = rodarBateria(casos);
    std::remove(caminho.c_str());
    conferir(r.total == 6 && r.passaram == 3 && r.falhas.size() == 3,
             "3 passam e 3 falham (linha sem TABs, veredito inválido, padrão recusado); "
             "obtido " + std::to_string(r.passaram) + " de " + std::to_string(r.total));
    conferir(lerCasos("arquivo_que_nao_existe.txt").empty(), "arquivo inexistente: lista vazia");
}

void testarMaquinaSemEstados() {
    secao("máquina sem estados");
    const Afn vazio;
    conferir(!vazio.aceita(""), "não aceita a cadeia vazia");
    conferir(!vazio.aceita("a"), "não aceita nada");
    conferir(vazio.simular("a").passos.empty(), "sem passos");
}

void testarEscala() {
    secao("padrões grandes (sem recursão sobre a altura da árvore)");
    {  // 100000 símbolos concatenados, árvore inclinada à esquerda
        const std::size_t n = 100000;
        Montador m;
        std::size_t acumulado = m.s('a');
        for (std::size_t i = 1; i < n; ++i) {
            const std::size_t folha = m.s('a');
            acumulado = m.cat(acumulado, folha);
        }
        const Afn afn = construirThompson(m.pronto(acumulado));
        conferir(afn.quantidadeDeEstados() == 2 * n, "cadeia de 100000 símbolos: 2n estados");
        conferir(afn.aceita(std::string(n, 'a')), "aceita a cadeia de 100000 'a'");
        conferir(!afn.aceita(std::string(n - 1, 'a')), "recusa 99999 'a'");
        conferir(!afn.aceita(std::string(n + 1, 'a')), "recusa 100001 'a'");
    }
    {  // fecho(fecho(...fecho('a'))) com 50000 níveis
        const std::size_t n = 50000;
        Montador m;
        std::size_t t = m.s('a');
        for (std::size_t i = 0; i < n; ++i) t = m.fecho(t);
        const Afn afn = construirThompson(m.pronto(t));
        conferir(afn.quantidadeDeEstados() == 2 + 2 * n, "fechos aninhados: 2+2n estados");
        conferir(afn.aceita("") && afn.aceita("aaa") && !afn.aceita("b"),
                 "fechos aninhados aceitam a* e só isso");
    }
}

// ---------------------------------------------------------------------------

void testarLeituraDosPadroes() {
    secao("leitura: contagens e formas de docs/ (padrao_arvore.md, 02_nucleo)");

    struct Contagem1 {
        const char* padrao;
        std::size_t nos;
    };
    const std::vector<Contagem1> tabela = {
        {"[a-z][a-z0-9_]*", 126},  // Identificador
        {"[0-9]+", 40},            // Número inteiro
        {"[+-]?[0-9]+", 46},       // Número com sinal
        {"\"[^\"]*\"", 172},       // Texto entre aspas
        {"bpm|wait|amp|fx", 23},   // Palavra fixa
        {"=|\\(|\\)", 5},          // Sinal
        {"(a|b)*", 4},
        {"(a*b*)*", 6},
        {"[0-9][0-9]*", 40},       // converge com [0-9]+
    };
    for (const auto& linha : tabela) {
        const Resultado r = analisarExpressao(linha.padrao);
        conferir(r.ok, std::string("lê ") + linha.padrao);
        if (r.ok) {
            conferir(tamanho(r.arvore) == linha.nos,
                     std::string("nós de ") + linha.padrao + ": esperado " +
                         std::to_string(linha.nos) + ", obtido " +
                         std::to_string(tamanho(r.arvore)));
        }
    }
    conferir(formatarArvore(analisarExpressao("[0-9]+").arvore) ==
                 formatarArvore(analisarExpressao("[0-9][0-9]*").arvore),
             "[0-9]+ e [0-9][0-9]* reduzem à mesma forma linear");
    conferir(tamanho(analisarExpressao(".").arvore) == 169, ". = alternância sobre os 85 símbolos de Σ");
    conferir(tamanho(analisarExpressao("[^a]").arvore) == 167, "[^a] = 84 símbolos de Σ");

    struct Forma {
        const char* padrao;
        const char* esperado;
    };
    const std::vector<Forma> formas = {
        {"a", "'a'"},
        {"ab", "concat('a', 'b')"},
        {"a|b|c", "alt(alt('a', 'b'), 'c')"},
        {"a|bc", "alt('a', concat('b', 'c'))"},
        {"ab*", "concat('a', fecho('b'))"},
        {"(ab)*", "fecho(concat('a', 'b'))"},
        {"a+", "concat('a', fecho('a'))"},
        {"a?", "alt('a', ε)"},
        {"[abc]", "alt(alt('a', 'b'), 'c')"},
        {"[a-c]", "alt(alt('a', 'b'), 'c')"},
        {"[aab]", "alt('a', 'b')"},
        {"(ab)", "concat('a', 'b')"},
        {"((a))", "'a'"},
        {"\\.", "'.'"},
        {"\\\\", "'\\\\'"},
        {"a{2}", "concat('a', 'a')"},
        {"a{2,}", "concat(concat('a', 'a'), fecho('a'))"},
        {"a{1,3}", "concat(concat('a', alt('a', ε)), alt('a', ε))"},
        {"a{0,2}", "concat(alt('a', ε), alt('a', ε))"},
        {"a{0,}", "fecho('a')"},
        {"a{0}", "ε"},
        {"(ab)+", "concat(concat('a', 'b'), fecho(concat('a', 'b')))"},
    };
    for (const auto& f : formas) {
        const Resultado r = analisarExpressao(f.padrao);
        conferir(r.ok && formatarArvore(r.arvore) == f.esperado,
                 std::string("forma de ") + f.padrao + ": esperado " + f.esperado +
                     ", obtido " + (r.ok ? formatarArvore(r.arvore) : formatarErro(f.padrao, r.erro)));
    }
    conferir(tamanho(analisarExpressao("a{0}").arvore) == 1, "a{0} não deixa nós órfãos");

    secao("leitura: as quatro recusas com posição (padrao_arvore.md, seção 6)");
    struct Recusa {
        const char* padrao;
        const char* mensagem;
        std::size_t posicao;
    };
    const std::vector<Recusa> recusas = {
        {"(bpm|wait", "grupo não fechado — esperado )", 9},
        {"*[0-9]", "operador de repetição sem operando: * não pode ser o primeiro símbolo", 0},
        {"[a-z", "classe de caracteres não fechada — esperado ]", 4},
        {"[0-9]+)", "símbolo inesperado após o fim da expressão: )", 6},
    };
    for (const auto& r : recusas) {
        const Resultado leitura = analisarExpressao(r.padrao);
        conferir(!leitura.ok && leitura.erro.mensagem == r.mensagem && leitura.erro.posicao == r.posicao,
                 std::string("recusa de ") + r.padrao);
    }
    std::cout << "  exemplo de formatarErro:\n"
              << formatarErro("[0-9]+)", analisarExpressao("[0-9]+)").erro) << '\n';

    secao("leitura: outras recusas (posição conferida à mão)");
    struct Posicao {
        const char* padrao;
        std::size_t posicao;
    };
    const std::vector<Posicao> outras = {
        {"", 0},        {"a**", 2},      {"()", 1},        {"a|", 2},      {"|a", 0},
        {"[]", 0},      {"[^]", 0},      {"[z-a]", 1},     {"\\q", 0},     {"a\\", 1},
        {"a{", 2},      {"a{2", 3},      {"a{,3}", 2},     {"a{3,2}", 1},  {"a{1001}", 2},
        {"]", 0},       {"a}", 1},       {"(a", 2},        {"a)b", 1},     {"+a", 0},
    };
    for (const auto& o : outras) {
        const Resultado r = analisarExpressao(o.padrao);
        conferir(!r.ok && r.erro.posicao == o.posicao,
                 std::string("recusa de \"") + o.padrao + "\" na posição " + std::to_string(o.posicao) +
                     (r.ok ? " (foi aceito)" : " (obtido " + std::to_string(r.erro.posicao) + ": " + r.erro.mensagem + ")"));
    }
}

void testarInvariantesDaConstrucao() {
    secao("invariantes da construção sobre padrões lidos");
    // Cada folha = 2 estados; cada alt/fecho = +2 estados; concat = +0.
    //   transições      = folhas(símbolo, ε) + concats + 4·(alts + fechos)
    //   transições vazias = ε + concats + 4·(alts + fechos)
    const std::vector<std::string> padroes = {
        "[a-z][a-z0-9_]*", "[0-9]+", "[+-]?[0-9]+", "\"[^\"]*\"", "bpm|wait|amp|fx", "=|\\(|\\)",
        "[2-9][0-9]|1[0-9][0-9]|2[0-9][0-9]|300", "(a*b*)*", "a{2,5}(b|c)?",
    };
    for (const std::string& p : padroes) {
        const Resultado r = analisarExpressao(p);
        if (!r.ok) {
            conferir(false, "lê " + p);
            continue;
        }
        const Contagem c = contar(r.arvore);
        const Afn afn = construirThompson(r.arvore);
        const std::size_t folhas = c.simbolos + c.vazias + c.linguagensVazias;
        const std::size_t operadoresComEstados = c.alts + c.fechos;
        conferir(afn.quantidadeDeEstados() == 2 * folhas + 2 * operadoresComEstados,
                 "estados de " + p);
        conferir(afn.quantidadeDeTransicoes() ==
                     c.simbolos + c.vazias + c.concats + 4 * operadoresComEstados,
                 "transições de " + p);
        conferir(afn.quantidadeDeTransicoesVazias() ==
                     c.vazias + c.concats + 4 * operadoresComEstados,
                 "transições vazias de " + p);
    }
}

void testarBateria(const std::string& caminho) {
    secao("bateria de casos: " + caminho);
    const std::vector<CasoDeTeste> casos = lerCasos(caminho);
    conferir(!casos.empty(), "o arquivo de casos abriu e tem casos");
    const ResultadoDaBateria r = rodarBateria(casos);
    for (const std::string& falha : r.falhas) std::cout << "  FALHOU: " << falha << '\n';
    conferir(r.falhas.empty() && r.passaram == r.total,
             "bateria: " + std::to_string(r.passaram) + "/" + std::to_string(r.total));
    std::cout << "  " << r.passaram << " de " << r.total << " casos passaram\n";
}

}  // namespace

int main(int argc, char* argv[]) {
    const std::string casos = (argc > 1) ? argv[1] : "casos_afn.txt";

    testarCasosDaConstrucao();
    testarLinguagemVaziaEmComposicao();
    testarCiclosDeTransicaoVazia();
    testarSubarvoreCompartilhada();
    testarArvoresMalformadas();
    testarTracoDaSimulacao();
    testarNumerosDaTarefa2();
    testarLeituraDeCasosMalformados();
    testarMaquinaSemEstados();
    testarEscala();
    testarLeituraDosPadroes();
    testarInvariantesDaConstrucao();
    testarBateria(casos);

    std::cout << "\n" << (g_total - g_falhas) << "/" << g_total << " verificações passaram";
    if (g_falhas) std::cout << " — " << g_falhas << " FALHARAM";
    std::cout << '\n';
    return g_falhas == 0 ? 0 : 1;
}
