// 02_regex.cpp — implementação de 02_regex.h: leitura de um padrão e redução ao
// núcleo (concatenação, alternância, fecho; símbolo, ε, ∅).
//
// Gramática lida (docs/01_recorte.md, ampliada com os quantificadores {n},
// {n,}, {n,m} da tabela de reduções de docs/02_nucleo):
//
//   regex  := alt
//   alt    := concat ( "|" concat )*
//   concat := repeat+
//   repeat := atom ( "*" | "+" | "?" | "{" n "}" | "{" n "," "}" | "{" n "," m "}" )?
//   atom   := CHAR | "." | "\" META | "[" "^"? item+ "]" | "(" alt ")"
//   item   := SIMBOLO | SIMBOLO "-" SIMBOLO
//
// Metacaracteres que exigem barra para valer como símbolo:  \ | ( ) [ ] * + ? { } .
// (e ^ - dentro de classes).
//
// Invariante que a leitura mantém e que simplifica as duplicações: todo nó é
// criado DEPOIS dos seus filhos, então a subárvore de qualquer construção é o
// intervalo contíguo de nós criado enquanto ela era lida, com a raiz no último
// índice. Duplicar uma subárvore (x+, x{n}) é copiar esse intervalo deslocando
// os índices dos filhos — nunca compartilhar índice, que produziria um grafo.

#include "02_regex.h"

#include <algorithm>
#include <cstdio>
#include <sstream>

namespace linguagem_musical {

bool Arvore::vazia() const { return nos.empty() || raiz == kSemFilho; }

namespace {

constexpr std::size_t kMaxRepeticao = 1000;        // limite de n e m em {n,m}
constexpr std::size_t kMaxNos = 1000000;           // teto da árvore
constexpr std::size_t kMaxProfundidade = 500;      // aninhamento de ( )

// Σ de especificacao/alfabeto.md: 85 símbolos. As aspas tipográficas ‘ e “ da
// especificação são lidas como ' e " (o léxico trabalha com bytes ASCII), e ´
// entra como o byte 0xB4. Com isso |Σ| = 85, a conta de padrao_arvore.md.
const std::string& alfabeto() {
    static const std::string sigma = [] {
        std::string s;
        for (char c = 'a'; c <= 'z'; ++c) s += c;
        for (char c = 'A'; c <= 'Z'; ++c) s += c;
        for (char c = '0'; c <= '9'; ++c) s += c;
        s += "=-+'\"!#$%&*()[]{}\\`";
        s += static_cast<char>(0xB4);
        s += ";:,";
        return s;
    }();
    return sigma;
}

std::string mostrarSimbolo(char c) {
    const unsigned char u = static_cast<unsigned char>(c);
    if (c == '\n') return "\\n";
    if (c == '\t') return "\\t";
    if (u >= 0x20 && u < 0x7f) return std::string(1, c);
    char buf[8];
    std::snprintf(buf, sizeof buf, "\\x%02X", u);
    return buf;
}

bool ehMeta(char c) {
    return c != '\0' && std::string("\\|()[]*+?{}.^-").find(c) != std::string::npos;
}

class Leitor {
public:
    explicit Leitor(const std::string& texto) : texto_(texto) {}

    Resultado ler() {
        Resultado resultado;
        try {
            if (texto_.empty()) erro("expressão vazia", 0);
            const std::size_t raiz = lerAlternancia();
            if (!fim()) {
                erro("símbolo inesperado após o fim da expressão: " +
                         mostrarSimbolo(atual()),
                     pos_);
            }
            arv_.raiz = raiz;
            resultado.ok = true;
            resultado.arvore = std::move(arv_);
        } catch (const ErroDeSintaxe& e) {
            resultado.ok = false;
            resultado.erro = e;
        }
        return resultado;
    }

private:
    const std::string& texto_;
    std::size_t pos_ = 0;
    std::size_t profundidade_ = 0;
    Arvore arv_;

    bool fim() const { return pos_ >= texto_.size(); }
    char atual() const { return texto_[pos_]; }

    [[noreturn]] void erro(const std::string& mensagem, std::size_t posicao) const {
        throw ErroDeSintaxe{posicao, mensagem};
    }

    // --- construtores de nó ---------------------------------------------------

    std::size_t novo(const No& no) {
        if (arv_.nos.size() >= kMaxNos) {
            erro("padrão grande demais: a árvore passaria de " +
                     std::to_string(kMaxNos) + " nós",
                 pos_);
        }
        arv_.nos.push_back(no);
        return arv_.nos.size() - 1;
    }

    std::size_t simbolo(char c) {
        No no;
        no.tipo = TipoDeNo::Simbolo;
        no.simbolo = c;
        return novo(no);
    }

    std::size_t cadeiaVazia() {
        No no;
        no.tipo = TipoDeNo::CadeiaVazia;
        return novo(no);
    }

    std::size_t linguagemVazia() {
        No no;
        no.tipo = TipoDeNo::LinguagemVazia;
        return novo(no);
    }

    std::size_t binario(TipoDeNo tipo, std::size_t esquerda, std::size_t direita) {
        No no;
        no.tipo = tipo;
        no.esquerda = esquerda;
        no.direita = direita;
        return novo(no);
    }

    std::size_t concat(std::size_t e, std::size_t d) {
        return binario(TipoDeNo::Concatenacao, e, d);
    }
    std::size_t alt(std::size_t e, std::size_t d) {
        return binario(TipoDeNo::Alternancia, e, d);
    }
    std::size_t fecho(std::size_t filho) {
        return binario(TipoDeNo::Fecho, filho, kSemFilho);
    }

    // Alternância (dobra à esquerda) de uma lista de símbolos; ∅ se a lista é vazia.
    std::size_t alternanciaDe(const std::vector<char>& simbolos) {
        if (simbolos.empty()) return linguagemVazia();
        std::size_t raiz = simbolo(simbolos[0]);
        for (std::size_t i = 1; i < simbolos.size(); ++i) {
            const std::size_t folha = simbolo(simbolos[i]);
            raiz = alt(raiz, folha);
        }
        return raiz;
    }

    // Copia o intervalo [inicio, fim) de nós para o fim da árvore, deslocando os
    // índices dos filhos, e devolve a raiz da cópia (o último nó do intervalo).
    std::size_t clonar(std::size_t inicio, std::size_t fimIntervalo) {
        const std::size_t deslocamento = arv_.nos.size() - inicio;
        if (arv_.nos.size() + (fimIntervalo - inicio) > kMaxNos) {
            erro("padrão grande demais: a árvore passaria de " +
                     std::to_string(kMaxNos) + " nós",
                 pos_);
        }
        for (std::size_t i = inicio; i < fimIntervalo; ++i) {
            No copia = arv_.nos[i];  // cópia antes do push_back: sem referência solta
            if (copia.esquerda != kSemFilho) copia.esquerda += deslocamento;
            if (copia.direita != kSemFilho) copia.direita += deslocamento;
            arv_.nos.push_back(copia);
        }
        return arv_.nos.size() - 1;
    }

    // --- gramática ------------------------------------------------------------

    std::size_t lerAlternancia() {
        std::size_t raiz = lerConcatenacao();
        while (!fim() && atual() == '|') {
            ++pos_;
            const std::size_t direita = lerConcatenacao();
            raiz = alt(raiz, direita);
        }
        return raiz;
    }

    std::size_t lerConcatenacao() {
        std::size_t raiz = lerRepeticao(true);
        while (!fim() && atual() != '|' && atual() != ')') {
            const std::size_t direita = lerRepeticao(false);
            raiz = concat(raiz, direita);
        }
        return raiz;
    }

    std::size_t lerRepeticao(bool primeiroDaAlternativa) {
        const std::size_t inicio = arv_.nos.size();
        const std::size_t raiz = lerAtomo(primeiroDaAlternativa);
        const std::size_t fimAtomo = arv_.nos.size();
        if (fim()) return raiz;

        switch (atual()) {
            case '*':
                ++pos_;
                return fecho(raiz);
            case '+': {  // x+  →  concat(x, fecho(x)), com x duplicado
                ++pos_;
                const std::size_t copia = clonar(inicio, fimAtomo);
                const std::size_t f = fecho(copia);
                return concat(raiz, f);
            }
            case '?': {  // x?  →  alt(x, ε)
                ++pos_;
                const std::size_t vazia = cadeiaVazia();
                return alt(raiz, vazia);
            }
            case '{':
                return lerLimites(inicio, fimAtomo, raiz);
            default:
                return raiz;
        }
    }

    std::size_t lerInteiro() {
        if (fim() || atual() < '0' || atual() > '9') {
            erro("número esperado no quantificador { }", pos_);
        }
        const std::size_t posNumero = pos_;
        std::size_t valor = 0;
        while (!fim() && atual() >= '0' && atual() <= '9') {
            valor = valor * 10 + static_cast<std::size_t>(atual() - '0');
            if (valor > kMaxRepeticao) {
                erro("contagem de repetição acima do limite de " +
                         std::to_string(kMaxRepeticao),
                     posNumero);
            }
            ++pos_;
        }
        return valor;
    }

    // x{n}, x{n,}, x{n,m}. O átomo original [inicio, fimAtomo) é aproveitado
    // como primeira cópia; as demais são clones.
    std::size_t lerLimites(std::size_t inicio, std::size_t fimAtomo, std::size_t raiz) {
        const std::size_t posAbre = pos_;
        ++pos_;  // '{'
        const std::size_t n = lerInteiro();
        std::size_t m = n;
        bool aberto = false;
        if (!fim() && atual() == ',') {
            ++pos_;
            if (!fim() && atual() >= '0' && atual() <= '9') {
                m = lerInteiro();
            } else {
                aberto = true;
            }
        }
        if (fim() || atual() != '}') erro("quantificador não fechado — esperado }", pos_);
        ++pos_;
        if (!aberto && m < n) {
            erro("limites invertidos em {n,m}: m menor que n", posAbre);
        }

        // x{0} e x{0,0}: só a cadeia vazia. Descarta os nós do átomo, para a
        // árvore não ficar com nós órfãos.
        if (!aberto && m == 0) {
            arv_.nos.resize(inicio);
            return cadeiaVazia();
        }

        bool originalUsado = false;
        auto copia = [&]() -> std::size_t {
            if (!originalUsado) {
                originalUsado = true;
                return raiz;
            }
            return clonar(inicio, fimAtomo);
        };

        std::size_t acumulado = kSemFilho;
        auto acrescentar = [&](std::size_t peca) {
            acumulado = (acumulado == kSemFilho) ? peca : concat(acumulado, peca);
        };

        for (std::size_t i = 0; i < n; ++i) acrescentar(copia());
        if (aberto) {
            const std::size_t c = copia();
            acrescentar(fecho(c));
        } else {
            for (std::size_t i = n; i < m; ++i) {
                const std::size_t c = copia();
                const std::size_t vazia = cadeiaVazia();
                acrescentar(alt(c, vazia));
            }
        }
        return acumulado;
    }

    std::size_t lerAtomo(bool primeiroDaAlternativa) {
        if (fim()) {
            erro("fim inesperado da expressão: esperado símbolo, classe, grupo ou coringa",
                 pos_);
        }
        const char c = atual();
        switch (c) {
            case '(':
                return lerGrupo();
            case '[':
                return lerClasse();
            case '.': {
                ++pos_;
                const std::string& sigma = alfabeto();
                return alternanciaDe(std::vector<char>(sigma.begin(), sigma.end()));
            }
            case '\\':
                return simbolo(lerEscape());
            case '*':
            case '+':
            case '?':
            case '{':
                if (primeiroDaAlternativa) {
                    erro(std::string("operador de repetição sem operando: ") + c +
                             " não pode ser o primeiro símbolo",
                         pos_);
                }
                erro(std::string("operador de repetição sem operando: ") + c +
                         " não tem o que repetir (a repetição anterior já foi aplicada)",
                     pos_);
            case '|':
            case ')':
                erro(std::string("alternativa vazia: esperado um símbolo antes de ") + c,
                     pos_);
            case ']':
            case '}':
                erro(std::string("símbolo reservado sem par: ") + c + " (use \\" + c +
                         " para o literal)",
                     pos_);
            default:
                if (c == '\0') erro("símbolo nulo não é permitido num padrão", pos_);
                ++pos_;
                return simbolo(c);
        }
    }

    std::size_t lerGrupo() {
        if (++profundidade_ > kMaxProfundidade) {
            erro("agrupamento aninhado demais (limite " +
                     std::to_string(kMaxProfundidade) + ")",
                 pos_);
        }
        ++pos_;  // '('
        const std::size_t raiz = lerAlternancia();
        if (fim() || atual() != ')') erro("grupo não fechado — esperado )", pos_);
        ++pos_;
        --profundidade_;
        return raiz;  // (x) → x: o agrupamento não vira nó
    }

    // Lê "\" META e devolve o símbolo literal.
    char lerEscape() {
        const std::size_t posBarra = pos_;
        ++pos_;
        if (fim()) erro("escape incompleto: \\ no fim da expressão", posBarra);
        const char c = atual();
        if (!ehMeta(c)) erro(std::string("escape desconhecido: \\") + c, posBarra);
        ++pos_;
        return c;
    }

    char lerSimboloDeClasse() {
        if (atual() == '\\') return lerEscape();
        const char c = atual();
        if (c == '\0') erro("símbolo nulo não é permitido num padrão", pos_);
        ++pos_;
        return c;
    }

    std::size_t lerClasse() {
        const std::size_t posAbre = pos_;
        ++pos_;  // '['
        bool negada = false;
        if (!fim() && atual() == '^') {
            negada = true;
            ++pos_;
        }

        std::vector<char> conjunto;
        std::vector<bool> presente(256, false);
        auto adicionar = [&](unsigned char u) {
            if (!presente[u]) {
                presente[u] = true;
                conjunto.push_back(static_cast<char>(u));
            }
        };

        while (true) {
            if (fim()) erro("classe de caracteres não fechada — esperado ]", pos_);
            if (atual() == ']') break;
            const std::size_t posItem = pos_;
            const char primeiro = lerSimboloDeClasse();
            // "a-z" é intervalo; "-" antes do ] (ou no começo) é literal.
            if (!fim() && atual() == '-' && pos_ + 1 < texto_.size() &&
                texto_[pos_ + 1] != ']') {
                ++pos_;
                const char ultimo = lerSimboloDeClasse();
                const unsigned char a = static_cast<unsigned char>(primeiro);
                const unsigned char b = static_cast<unsigned char>(ultimo);
                if (b < a) {
                    erro("intervalo invertido: " + mostrarSimbolo(primeiro) + "-" +
                             mostrarSimbolo(ultimo),
                         posItem);
                }
                for (unsigned u = a; u <= b; ++u) adicionar(static_cast<unsigned char>(u));
            } else {
                adicionar(static_cast<unsigned char>(primeiro));
            }
        }
        if (conjunto.empty()) erro("classe de caracteres vazia", posAbre);
        ++pos_;  // ']'

        if (!negada) return alternanciaDe(conjunto);

        // [^...]: alternância sobre Σ menos os símbolos da classe.
        std::vector<char> complemento;
        for (char c : alfabeto()) {
            if (!presente[static_cast<unsigned char>(c)]) complemento.push_back(c);
        }
        return alternanciaDe(complemento);
    }
};

}  // namespace

Resultado analisarExpressao(const std::string& expressao) {
    return Leitor(expressao).ler();
}

std::string formatarArvore(const Arvore& arvore) {
    if (arvore.vazia()) return "";

    struct Item {
        bool texto;
        std::string conteudo;
        std::size_t no;
    };
    std::string saida;
    std::vector<Item> pilha;
    pilha.push_back({false, "", arvore.raiz});

    // Guarda contra árvore malformada (ciclo): uma árvore legítima gasta no
    // máximo ~4 itens por nó.
    std::size_t passos = 0;
    const std::size_t limite = 64 * (arvore.nos.size() + 1);

    while (!pilha.empty()) {
        if (++passos > limite) {
            saida += "…";
            break;
        }
        Item item = std::move(pilha.back());
        pilha.pop_back();
        if (item.texto) {
            saida += item.conteudo;
            continue;
        }
        if (item.no >= arvore.nos.size()) {
            saida += "<nó inválido>";
            continue;
        }
        const No& no = arvore.nos[item.no];
        switch (no.tipo) {
            case TipoDeNo::Simbolo: {
                std::string s;
                if (no.simbolo == '\'') s = "\\'";
                else if (no.simbolo == '\\') s = "\\\\";
                else s = mostrarSimbolo(no.simbolo);
                saida += "'" + s + "'";
                break;
            }
            case TipoDeNo::CadeiaVazia:
                saida += "ε";
                break;
            case TipoDeNo::LinguagemVazia:
                saida += "∅";
                break;
            case TipoDeNo::Concatenacao:
            case TipoDeNo::Alternancia:
                saida += (no.tipo == TipoDeNo::Concatenacao) ? "concat(" : "alt(";
                pilha.push_back({true, ")", 0});
                pilha.push_back({false, "", no.direita});
                pilha.push_back({true, ", ", 0});
                pilha.push_back({false, "", no.esquerda});
                break;
            case TipoDeNo::Fecho:
                saida += "fecho(";
                pilha.push_back({true, ")", 0});
                pilha.push_back({false, "", no.esquerda});
                break;
        }
    }
    return saida;
}

std::string formatarErro(const std::string& expressao, const ErroDeSintaxe& erro) {
    std::ostringstream saida;
    saida << "padrão: " << expressao << '\n';
    saida << "        " << std::string(erro.posicao, ' ') << "^\n";
    saida << "erro na posição " << erro.posicao << ": " << erro.mensagem;
    return saida.str();
}

std::size_t tamanho(const Arvore& arvore) { return arvore.nos.size(); }

}  // namespace linguagem_musical
