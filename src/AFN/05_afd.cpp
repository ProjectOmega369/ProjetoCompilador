// 05_afd.cpp — implementação de 05_afd.h.

#include "05_afd.h"

#include <algorithm>
#include <map>
#include <sstream>
#include <stdexcept>

namespace linguagem_musical {

namespace {

std::string semRepeticao(const std::string& s) {
    std::string r;
    for (char c : s) {
        if (r.find(c) == std::string::npos) r += c;
    }
    return r;
}

}  // namespace

bool Afd::completo() const {
    for (const auto& linha : delta) {
        for (int d : linha) {
            if (d == kSemTransicao) return false;
        }
    }
    return true;
}

bool Afd::aceita(const std::string& cadeia) const {
    if (delta.empty()) return false;
    int e = inicial;
    for (char c : cadeia) {
        const std::size_t k = alfabeto.find(c);
        if (k == std::string::npos) return false;
        e = delta[e][k];
        if (e == kSemTransicao) return false;
    }
    return aceitacao[e] != 0;
}

// -----------------------------------------------------------------------------
// Construção de subconjuntos
// -----------------------------------------------------------------------------

Afd determinizar(const Afn& afn, const std::string& alfabetoPedido) {
    if (alfabetoPedido.find('\0') != std::string::npos) {
        throw std::invalid_argument("o alfabeto não pode conter '\\0' (rótulo de ε)");
    }
    Afd afd;
    afd.alfabeto = semRepeticao(alfabetoPedido);
    if (afn.quantidadeDeEstados() == 0) return afd;

    std::map<std::vector<Estado>, int> indice;  // conjunto do AFN -> estado do AFD
    auto obter = [&](const std::vector<Estado>& conjunto) {
        auto it = indice.find(conjunto);
        if (it != indice.end()) return it->second;
        const int novo = static_cast<int>(afd.delta.size());
        indice.emplace(conjunto, novo);
        afd.delta.emplace_back(afd.alfabeto.size(), kSemTransicao);
        afd.origem.push_back(conjunto);
        afd.aceitacao.push_back(
            std::binary_search(conjunto.begin(), conjunto.end(), afn.aceitacao()) ? 1 : 0);
        return novo;
    };

    afd.inicial = obter(afn.fechoVazio({afn.inicial()}));
    // Fila de trabalho: os estados são numerados na ordem em que aparecem.
    for (std::size_t atual = 0; atual < afd.delta.size(); ++atual) {
        for (std::size_t k = 0; k < afd.alfabeto.size(); ++k) {
            // Cópia: obter() pode realocar afd.origem.
            const std::vector<Estado> origemAtual = afd.origem[atual];
            const std::vector<Estado> alvo =
                afn.fechoVazio(afn.mover(origemAtual, afd.alfabeto[k]));
            // alvo vazio = o estado de erro, que é um estado como os outros.
            afd.delta[atual][k] = obter(alvo);
        }
    }
    return afd;
}

// -----------------------------------------------------------------------------
// Minimização
// -----------------------------------------------------------------------------

Afd completar(const Afd& afd) {
    if (afd.delta.empty() || afd.completo()) return afd;
    Afd r = afd;
    const int erro = static_cast<int>(r.delta.size());
    r.delta.emplace_back(r.alfabeto.size(), erro);
    r.aceitacao.push_back(0);
    if (!r.origem.empty()) r.origem.emplace_back();
    for (auto& linha : r.delta) {
        for (int& d : linha) {
            if (d == kSemTransicao) d = erro;
        }
    }
    return r;
}

Afd removerInacessiveis(const Afd& afd) {
    if (afd.delta.empty()) return afd;
    std::vector<int> novo(afd.delta.size(), -1);
    std::vector<int> ordem{afd.inicial};
    novo[afd.inicial] = 0;
    for (std::size_t i = 0; i < ordem.size(); ++i) {
        for (int d : afd.delta[ordem[i]]) {
            if (d != kSemTransicao && novo[d] < 0) {
                novo[d] = static_cast<int>(ordem.size());
                ordem.push_back(d);
            }
        }
    }
    Afd r;
    r.alfabeto = afd.alfabeto;
    r.inicial = 0;
    for (int velho : ordem) {
        std::vector<int> linha = afd.delta[velho];
        for (int& d : linha) {
            if (d != kSemTransicao) d = novo[d];
        }
        r.delta.push_back(std::move(linha));
        r.aceitacao.push_back(afd.aceitacao[velho]);
        if (!afd.origem.empty()) r.origem.push_back(afd.origem[velho]);
    }
    return r;
}

Afd minimizar(const Afd& entrada, RelatorioMinimizacao* relatorio) {
    RelatorioMinimizacao rel;
    rel.estadosAntes = entrada.quantidadeDeEstados();
    if (entrada.delta.empty()) {
        if (relatorio) *relatorio = rel;
        return entrada;
    }

    const Afd completo = completar(entrada);
    rel.depoisDeCompletar = completo.quantidadeDeEstados();
    const Afd a = removerInacessiveis(completo);
    rel.depoisDeInacessiveis = a.quantidadeDeEstados();
    const std::size_t n = a.quantidadeDeEstados();
    const std::size_t m = a.alfabeto.size();

    // Partição inicial: aceitação x não aceitação (só as classes não vazias).
    std::vector<int> bloco(n);
    std::size_t blocos;
    {
        bool temAceita = false, temResto = false;
        for (std::size_t e = 0; e < n; ++e) (a.aceitacao[e] ? temAceita : temResto) = true;
        const int idAceita = temResto ? 1 : 0;
        for (std::size_t e = 0; e < n; ++e) bloco[e] = a.aceitacao[e] ? idAceita : 0;
        blocos = (temAceita ? 1 : 0) + (temResto ? 1 : 0);
    }
    rel.particaoInicial = blocos;

    // Refinamento: dois estados continuam juntos se estão no mesmo bloco e seus
    // sucessores, símbolo a símbolo, caem nos mesmos blocos. Repete até nenhum
    // bloco se partir (ponto fixo).
    for (;;) {
        ++rel.rodadas;
        std::map<std::vector<int>, int> assinaturas;
        std::vector<int> novoBloco(n);
        for (std::size_t e = 0; e < n; ++e) {
            std::vector<int> chave{bloco[e]};
            for (std::size_t k = 0; k < m; ++k) chave.push_back(bloco[a.delta[e][k]]);
            auto it = assinaturas.emplace(chave, static_cast<int>(assinaturas.size())).first;
            novoBloco[e] = it->second;
        }
        const std::size_t novos = assinaturas.size();
        bloco = std::move(novoBloco);
        if (novos == blocos) break;
        blocos = novos;
    }

    // Monta o quociente, numerando os blocos pela ordem do estado inicial.
    std::vector<int> numero(blocos, -1);
    std::vector<int> representante;
    // Os estados de `a` já estão em ordem de alcance (BFS), então percorrê-los
    // em ordem numera os blocos de forma determinística.
    for (std::size_t e = 0; e < n; ++e) {
        if (numero[bloco[e]] < 0) {
            numero[bloco[e]] = static_cast<int>(representante.size());
            representante.push_back(static_cast<int>(e));
        }
    }
    Afd r;
    r.alfabeto = a.alfabeto;
    r.inicial = numero[bloco[a.inicial]];
    for (int rep : representante) {
        std::vector<int> linha(m);
        for (std::size_t k = 0; k < m; ++k) linha[k] = numero[bloco[a.delta[rep][k]]];
        r.delta.push_back(std::move(linha));
        r.aceitacao.push_back(a.aceitacao[rep]);
    }
    rel.estadosDepois = r.quantidadeDeEstados();
    if (relatorio) *relatorio = rel;
    return r;
}

std::string formatarAfd(const Afd& afd) {
    std::ostringstream s;
    s << "estados: " << afd.quantidadeDeEstados() << ", inicial: q" << afd.inicial
      << ", alfabeto: \"" << afd.alfabeto << "\"\n";
    for (std::size_t e = 0; e < afd.delta.size(); ++e) {
        s << "  q" << e << (afd.aceitacao[e] ? " [aceita]" : "         ");
        for (std::size_t k = 0; k < afd.alfabeto.size(); ++k) {
            s << "  " << afd.alfabeto[k] << "->";
            if (afd.delta[e][k] == kSemTransicao) s << "-";
            else s << "q" << afd.delta[e][k];
        }
        if (e < afd.origem.size()) {
            s << "   = {";
            for (std::size_t i = 0; i < afd.origem[e].size(); ++i) {
                s << (i ? "," : "") << afd.origem[e][i];
            }
            s << "}" << (afd.origem[e].empty() ? " (erro)" : "");
        }
        s << '\n';
    }
    return s.str();
}

ComparacaoDasTres compararTres(const Afn& afn, const Afd& afd, const Afd& minimo,
                               const std::string& alfabeto,
                               std::size_t comprimentoMaximo) {
    ComparacaoDasTres c;
    const std::string sigma = semRepeticao(alfabeto);
    std::vector<std::string> nivel{""};
    for (std::size_t len = 0; len <= comprimentoMaximo; ++len) {
        for (const std::string& w : nivel) {
            ++c.total;
            const bool n = afn.aceita(w), d = afd.aceita(w), m = minimo.aceita(w);
            c.aceitasAfn += n;
            c.aceitasAfd += d;
            c.aceitasMinimo += m;
            c.divergenciasAfnAfd += (n != d);
            c.divergenciasAfdMinimo += (d != m);
            c.divergenciasAfnMinimo += (n != m);
            if ((n != d || d != m || n != m) && c.exemplos.size() < 10) {
                c.exemplos.push_back(w);
            }
        }
        if (len == comprimentoMaximo) break;
        std::vector<std::string> prox;
        prox.reserve(nivel.size() * sigma.size());
        for (const std::string& w : nivel) {
            for (char x : sigma) prox.push_back(w + x);
        }
        nivel = std::move(prox);
    }
    return c;
}

}  // namespace linguagem_musical
