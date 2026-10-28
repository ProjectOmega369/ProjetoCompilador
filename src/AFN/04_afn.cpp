// 04_afn.cpp — implementação de 04_afn.h.

#include "04_afn.h"

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace linguagem_musical {

namespace {

// Mostra um símbolo de forma legível: imprimíveis como são, o resto em \xHH.
std::string mostrarSimbolo(char c) {
    const unsigned char u = static_cast<unsigned char>(c);
    if (c == '\n') return "\\n";
    if (c == '\t') return "\\t";
    if (c == '\r') return "\\r";
    if (u >= 0x20 && u < 0x7f) return std::string(1, c);
    char buf[8];
    std::snprintf(buf, sizeof buf, "\\x%02X", u);
    return buf;
}

std::string mostrarCadeia(const std::string& cadeia) {
    std::string saida;
    for (char c : cadeia) saida += mostrarSimbolo(c);
    return saida;
}

std::string mostrarConjunto(const std::vector<Estado>& conjunto) {
    std::string saida = "{";
    for (std::size_t i = 0; i < conjunto.size(); ++i) {
        if (i) saida += ", ";
        saida += std::to_string(conjunto[i]);
    }
    return saida + "}";
}

}  // namespace

// -----------------------------------------------------------------------------
// Afn
// -----------------------------------------------------------------------------

Estado Afn::novoEstado() {
    transicoes_.emplace_back();
    return transicoes_.size() - 1;
}

void Afn::adicionarTransicao(Estado origem, char simbolo, Estado destino) {
    if (origem >= transicoes_.size() || destino >= transicoes_.size()) {
        throw std::out_of_range("transição entre estados inexistentes");
    }
    transicoes_[origem].push_back({simbolo, destino});
}

void Afn::definirInicial(Estado estado) {
    if (estado >= transicoes_.size()) {
        throw std::out_of_range("estado inicial inexistente");
    }
    inicial_ = estado;
}

void Afn::definirAceitacao(Estado estado) {
    if (estado >= transicoes_.size()) {
        throw std::out_of_range("estado de aceitação inexistente");
    }
    aceitacao_ = estado;
}

std::size_t Afn::quantidadeDeEstados() const { return transicoes_.size(); }

std::size_t Afn::quantidadeDeTransicoes() const {
    std::size_t total = 0;
    for (const auto& saidas : transicoes_) total += saidas.size();
    return total;
}

std::size_t Afn::quantidadeDeTransicoesVazias() const {
    std::size_t total = 0;
    for (const auto& saidas : transicoes_) {
        for (const TransicaoAfn& t : saidas) {
            if (t.simbolo == kEpsilon) ++total;
        }
    }
    return total;
}

void Afn::expandirFecho(std::vector<Estado>& conjunto,
                        std::vector<std::size_t>& marca,
                        std::size_t geracao) const {
    std::vector<Estado> pilha;
    std::vector<Estado> unicos;
    for (Estado e : conjunto) {
        if (marca[e] != geracao) {
            marca[e] = geracao;
            unicos.push_back(e);
            pilha.push_back(e);
        }
    }
    while (!pilha.empty()) {
        const Estado e = pilha.back();
        pilha.pop_back();
        for (const TransicaoAfn& t : transicoes_[e]) {
            if (t.simbolo == kEpsilon && marca[t.destino] != geracao) {
                marca[t.destino] = geracao;
                unicos.push_back(t.destino);
                pilha.push_back(t.destino);
            }
        }
    }
    std::sort(unicos.begin(), unicos.end());
    conjunto = std::move(unicos);
}

std::vector<Estado> Afn::fechoVazio(const std::vector<Estado>& conjunto) const {
    for (Estado e : conjunto) {
        if (e >= transicoes_.size()) {
            throw std::out_of_range("fecho vazio de estado inexistente");
        }
    }
    std::vector<std::size_t> marca(transicoes_.size(), 0);
    std::vector<Estado> resultado = conjunto;
    expandirFecho(resultado, marca, 1);
    return resultado;
}

// Simulação por conjunto de estados ativos (Tarefa 2).
//
// Representação do conjunto: um vetor de estados (é o que se percorre) mais um
// vetor denso de marcas indexado pelo estado, `marca`, em vez de um conjunto
// ordenado. `marca[e] == geracao` responde "e já entrou neste passo?" com uma
// consulta, sem busca e sem alocar um nó por estado inserido. Em vez de zerar
// as marcas a cada símbolo, cada passo usa um número de geração novo: nada é
// zerado, e o custo por símbolo acompanha o conjunto ativo (mais a ordenação
// dele, que mantém o traço canônico), não o tamanho da máquina. O fecho vazio
// da simulação reaproveita essas mesmas marcas.
//
// Dois pontos em que a simulação costuma errar, e que os testes cobrem:
//  - o conjunto INICIAL é o fecho vazio do estado inicial, não o estado sozinho;
//    sem isso, todo padrão que começa por alternância ou fecho (a|b, a*, a?,
//    [ab]) falha, e os padrões que começam por símbolo não mostram o defeito;
//  - o fecho vazio marca os visitados: com um ciclo só de transições vazias,
//    como em (a*)* ou (a?)*, a travessia sem marcas não termina.
//
// Simular não é determinizar: os conjuntos são calculados e descartados a cada
// símbolo; nenhum deles vira estado de coisa alguma.
bool Afn::executar(const std::string& cadeia, Simulacao* traco) const {
    if (traco) *traco = Simulacao{};
    // Máquina sem estados não aceita nada (nem a cadeia vazia).
    if (transicoes_.empty()) return false;

    std::vector<std::size_t> marca(transicoes_.size(), 0);
    std::size_t geracao = 0;

    // O fecho vazio vale também para o conjunto INICIAL.
    std::vector<Estado> ativos{inicial_};
    expandirFecho(ativos, marca, ++geracao);

    auto registrar = [&](char lido) {
        if (!traco) return;
        traco->passos.push_back({lido, ativos});
        traco->maiorConjuntoAtivo =
            std::max(traco->maiorConjuntoAtivo, ativos.size());
    };
    registrar('\0');

    for (char c : cadeia) {
        std::vector<Estado> proximos;
        for (Estado e : ativos) {
            for (const TransicaoAfn& t : transicoes_[e]) {
                // A transição vazia nunca consome símbolo, nem um '\0' da
                // entrada: o rótulo kEpsilon é pulado antes da comparação.
                if (t.simbolo != kEpsilon && t.simbolo == c) {
                    proximos.push_back(t.destino);
                }
            }
        }
        ativos = std::move(proximos);
        expandirFecho(ativos, marca, ++geracao);
        registrar(c);
        if (ativos.empty()) break;  // sem estado ativo nada reabre a disputa
    }

    const bool aceitou =
        std::binary_search(ativos.begin(), ativos.end(), aceitacao_);
    if (traco) traco->aceitou = aceitou;
    return aceitou;
}

bool Afn::aceita(const std::string& cadeia) const {
    return executar(cadeia, nullptr);
}

Simulacao Afn::simular(const std::string& cadeia) const {
    Simulacao simulacao;
    executar(cadeia, &simulacao);
    return simulacao;
}

std::string Afn::formatarSimulacao(const std::string& cadeia,
                                   const Simulacao& simulacao) const {
    std::ostringstream saida;
    saida << "cadeia \"" << mostrarCadeia(cadeia) << "\" (" << cadeia.size()
          << (cadeia.size() == 1 ? " símbolo)\n" : " símbolos)\n");
    for (std::size_t i = 0; i < simulacao.passos.size(); ++i) {
        const PassoDaSimulacao& passo = simulacao.passos[i];
        saida << "  passo " << i << "  ";
        if (i == 0) {
            saida << "início     ";
        } else {
            // "lê '" tem 4 colunas ('ê' ocupa 2 bytes, 1 coluna); o resto é ASCII.
            const std::string simbolo = mostrarSimbolo(passo.simboloLido);
            const std::size_t colunas = 5 + simbolo.size();
            saida << "lê '" << simbolo << "'"
                  << std::string(colunas < 11 ? 11 - colunas : 0, ' ');
        }
        saida << ' ' << mostrarConjunto(passo.ativos) << '\n';
    }
    if (!simulacao.passos.empty() && simulacao.passos.back().ativos.empty()) {
        saida << "  conjunto vazio: nenhuma continuação possível\n";
    }
    saida << "veredito: " << (simulacao.aceitou ? "ACEITA" : "RECUSA")
          << "  (maior conjunto ativo: " << simulacao.maiorConjuntoAtivo << ")\n";
    return saida.str();
}

// -----------------------------------------------------------------------------
// Construção de Thompson
// -----------------------------------------------------------------------------

namespace {

// Os seis casos da construção. Cada um recebe a máquina em construção e os
// blocos dos filhos, cria os estados novos de que precisa e devolve o bloco.

Bloco blocoSimbolo(Afn& afn, char simbolo) {
    const Estado entrada = afn.novoEstado();
    const Estado saida = afn.novoEstado();
    afn.adicionarTransicao(entrada, simbolo, saida);
    return {entrada, saida};
}

Bloco blocoCadeiaVazia(Afn& afn) {
    const Estado entrada = afn.novoEstado();
    const Estado saida = afn.novoEstado();
    afn.adicionarTransicao(entrada, kEpsilon, saida);
    return {entrada, saida};
}

// ∅: entrada e saída sem caminho entre elas. Nenhuma cadeia chega à saída.
Bloco blocoLinguagemVazia(Afn& afn) {
    const Estado entrada = afn.novoEstado();
    const Estado saida = afn.novoEstado();
    return {entrada, saida};
}

Bloco blocoConcatenacao(Afn& afn, Bloco a, Bloco b) {
    afn.adicionarTransicao(a.saida, kEpsilon, b.entrada);
    return {a.entrada, b.saida};
}

Bloco blocoAlternancia(Afn& afn, Bloco a, Bloco b) {
    const Estado entrada = afn.novoEstado();
    const Estado saida = afn.novoEstado();
    afn.adicionarTransicao(entrada, kEpsilon, a.entrada);
    afn.adicionarTransicao(entrada, kEpsilon, b.entrada);
    afn.adicionarTransicao(a.saida, kEpsilon, saida);
    afn.adicionarTransicao(b.saida, kEpsilon, saida);
    return {entrada, saida};
}

Bloco blocoFecho(Afn& afn, Bloco a) {
    const Estado entrada = afn.novoEstado();
    const Estado saida = afn.novoEstado();
    afn.adicionarTransicao(entrada, kEpsilon, a.entrada);  // uma ou mais voltas
    afn.adicionarTransicao(entrada, kEpsilon, saida);      // zero voltas
    afn.adicionarTransicao(a.saida, kEpsilon, a.entrada);  // mais uma volta
    afn.adicionarTransicao(a.saida, kEpsilon, saida);      // terminou
    return {entrada, saida};
}

std::string rotuloDoNo(std::size_t indice) { return "nó " + std::to_string(indice); }

}  // namespace

Afn construirThompson(const Arvore& arvore) {
    const std::size_t n = arvore.nos.size();
    if (arvore.raiz == kSemFilho || arvore.raiz >= n) {
        throw std::invalid_argument("árvore sem raiz válida");
    }

    auto exigirFilho = [&](std::size_t pai, std::size_t filho, const char* lado) {
        if (filho == kSemFilho || filho >= n) {
            throw std::invalid_argument(rotuloDoNo(pai) + ": filho " + lado +
                                        " ausente ou fora da árvore");
        }
    };

    Afn afn;
    std::vector<Bloco> blocos;           // pilha de blocos já construídos
    std::vector<char> emAndamento(n, 0); // nós cujos filhos ainda estão em obra

    // Percurso em pós-ordem com pilha própria: visita os filhos (esquerdo
    // primeiro) e, ao voltar ao pai, combina os blocos que eles deixaram.
    struct Item {
        std::size_t no;
        bool filhosProntos;
    };
    std::vector<Item> pilha{{arvore.raiz, false}};

    while (!pilha.empty()) {
        const Item item = pilha.back();
        pilha.pop_back();
        const No& no = arvore.nos[item.no];

        if (item.filhosProntos) {
            emAndamento[item.no] = 0;
            switch (no.tipo) {
                case TipoDeNo::Concatenacao: {
                    const Bloco b = blocos.back(); blocos.pop_back();
                    const Bloco a = blocos.back(); blocos.pop_back();
                    blocos.push_back(blocoConcatenacao(afn, a, b));
                    break;
                }
                case TipoDeNo::Alternancia: {
                    const Bloco b = blocos.back(); blocos.pop_back();
                    const Bloco a = blocos.back(); blocos.pop_back();
                    blocos.push_back(blocoAlternancia(afn, a, b));
                    break;
                }
                case TipoDeNo::Fecho: {
                    const Bloco a = blocos.back(); blocos.pop_back();
                    blocos.push_back(blocoFecho(afn, a));
                    break;
                }
                default:  // folhas nunca ficam "à espera dos filhos"
                    throw std::logic_error("estado interno inconsistente");
            }
            continue;
        }

        switch (no.tipo) {
            case TipoDeNo::Simbolo:
                if (no.simbolo == kEpsilon) {
                    throw std::invalid_argument(
                        rotuloDoNo(item.no) +
                        ": símbolo '\\0' colide com o rótulo da transição vazia");
                }
                blocos.push_back(blocoSimbolo(afn, no.simbolo));
                break;
            case TipoDeNo::CadeiaVazia:
                blocos.push_back(blocoCadeiaVazia(afn));
                break;
            case TipoDeNo::LinguagemVazia:
                blocos.push_back(blocoLinguagemVazia(afn));
                break;
            case TipoDeNo::Concatenacao:
            case TipoDeNo::Alternancia:
                exigirFilho(item.no, no.esquerda, "esquerdo");
                exigirFilho(item.no, no.direita, "direito");
                if (emAndamento[item.no]) {
                    throw std::invalid_argument(rotuloDoNo(item.no) +
                                                ": ciclo na árvore");
                }
                emAndamento[item.no] = 1;
                pilha.push_back({item.no, true});
                pilha.push_back({no.direita, false});
                pilha.push_back({no.esquerda, false});
                break;
            case TipoDeNo::Fecho:
                exigirFilho(item.no, no.esquerda, "esquerdo");
                if (emAndamento[item.no]) {
                    throw std::invalid_argument(rotuloDoNo(item.no) +
                                                ": ciclo na árvore");
                }
                emAndamento[item.no] = 1;
                pilha.push_back({item.no, true});
                pilha.push_back({no.esquerda, false});
                break;
            default:
                throw std::invalid_argument(rotuloDoNo(item.no) +
                                            ": tipo de nó desconhecido");
        }
    }

    if (blocos.size() != 1) {
        throw std::logic_error("a construção deveria deixar exatamente um bloco");
    }
    afn.definirInicial(blocos.front().entrada);
    afn.definirAceitacao(blocos.front().saida);
    return afn;
}

// -----------------------------------------------------------------------------
// Bateria de casos
// -----------------------------------------------------------------------------

std::vector<CasoDeTeste> lerCasos(const std::string& caminho) {
    std::ifstream arquivo(caminho);
    std::vector<CasoDeTeste> casos;
    if (!arquivo.is_open()) return casos;

    std::string linha;
    std::size_t numero = 0;
    while (std::getline(arquivo, linha)) {
        ++numero;
        if (!linha.empty() && linha.back() == '\r') linha.pop_back();
        if (linha.empty() || linha.front() == '#') continue;

        CasoDeTeste caso;
        caso.origem = caminho + ":" + std::to_string(numero);

        const std::size_t t1 = linha.find('\t');
        const std::size_t t2 =
            (t1 == std::string::npos) ? std::string::npos : linha.find('\t', t1 + 1);
        if (t2 == std::string::npos) {
            caso.erroDeLeitura =
                "linha malformada: esperado padrão<TAB>cadeia<TAB>aceita|recusa";
            casos.push_back(caso);
            continue;
        }

        caso.expressao = linha.substr(0, t1);
        caso.cadeia = linha.substr(t1 + 1, t2 - t1 - 1);
        const std::string veredito = linha.substr(t2 + 1);
        if (veredito == "aceita") {
            caso.esperado = true;
        } else if (veredito == "recusa" || veredito == "rejeita") {
            caso.esperado = false;
        } else {
            caso.erroDeLeitura = "veredito inválido \"" + veredito +
                                 "\": esperado aceita ou recusa";
        }
        casos.push_back(caso);
    }
    return casos;
}

ResultadoDaBateria rodarBateria(const std::vector<CasoDeTeste>& casos) {
    ResultadoDaBateria resultado;
    for (const CasoDeTeste& caso : casos) {
        ++resultado.total;
        const std::string cabecalho = caso.origem + ": padrão \"" +
                                      mostrarCadeia(caso.expressao) +
                                      "\", cadeia \"" +
                                      mostrarCadeia(caso.cadeia) + "\": ";

        if (!caso.erroDeLeitura.empty()) {
            resultado.falhas.push_back(caso.origem + ": " + caso.erroDeLeitura);
            continue;
        }

        const Resultado leitura = analisarExpressao(caso.expressao);
        if (!leitura.ok) {
            resultado.falhas.push_back(cabecalho + "padrão recusado pela leitura — " +
                                       leitura.erro.mensagem + " (posição " +
                                       std::to_string(leitura.erro.posicao) + ")");
            continue;
        }

        bool obtido = false;
        try {
            obtido = construirThompson(leitura.arvore).aceita(caso.cadeia);
        } catch (const std::exception& e) {
            resultado.falhas.push_back(cabecalho + "construção falhou — " + e.what());
            continue;
        }

        if (obtido != caso.esperado) {
            resultado.falhas.push_back(
                cabecalho + "esperado " + (caso.esperado ? "ACEITA" : "RECUSA") +
                ", obtido " + (obtido ? "ACEITA" : "RECUSA"));
            continue;
        }
        ++resultado.passaram;
    }
    return resultado;
}

}  // namespace linguagem_musical
