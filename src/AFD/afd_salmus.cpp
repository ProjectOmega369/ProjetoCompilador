// afd_salmus.cpp
//
// Implementação de Autômatos Finitos Determinísticos (AFD) para as
// classes léxicas da Linguagem Musical (Salmus), conforme documentado em
// especificacao/padrao_arvore.md ("1. Tabela de peças").
//
// A classe AFD é a mesma estrutura Q / Σ / δ / q0 / F do exemplo base
// (afd.cpp); o que muda são os autômatos concretos: em vez de um único
// AFD binário de exemplo, aqui construímos um AFD por classe léxica:
//
//   1) Identificador       -> [a-zA-Z][a-zA-Z0-9_]*
//   2) Numero inteiro      -> [0-9]+
//   3) Numero com sinal    -> [+-]?[0-9]+
//   4) Texto entre aspas   -> "[^"]*"
//   5) Palavra fixa        -> bpm|wait|amp|fx
//   6) Sinal               -> =|(|)
//
// O programa lê um arquivo texto (uma lexeme/token por linha) e informa,
// para cada linha, qual(is) classe(s) léxica(s) da Salmus a reconhecem.

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <stdexcept>

class AFD {
private:
    // Q: conjunto de estados
    std::unordered_set<std::string> estados;

    // Σ: alfabeto
    std::unordered_set<char> alfabeto;

    // δ: função de transição
    std::unordered_map<
        std::string,
        std::unordered_map<char, std::string>
    > transicoes;

    // q₀: estado inicial
    std::string estadoInicial;

    // F: conjunto de estados finais
    std::unordered_set<std::string> estadosFinais;

public:

    void adicionarEstado(const std::string& estado) {
        estados.insert(estado);
    }

    void adicionarSimbolo(char simbolo) {
        alfabeto.insert(simbolo);
    }

    void definirEstadoInicial(const std::string& estado) {
        if (!estados.contains(estado)) {
            throw std::invalid_argument(
                "O estado inicial deve pertencer a Q."
            );
        }
        estadoInicial = estado;
    }

    void adicionarEstadoFinal(const std::string& estado) {
        if (!estados.contains(estado)) {
            throw std::invalid_argument(
                "O estado final deve pertencer a Q."
            );
        }
        estadosFinais.insert(estado);
    }

    void adicionarTransicao(
        const std::string& estado,
        char simbolo,
        const std::string& proximoEstado
    ) {
        if (!estados.contains(estado)) {
            throw std::invalid_argument(
                "Estado de origem inexistente: " + estado
            );
        }
        if (!estados.contains(proximoEstado)) {
            throw std::invalid_argument(
                "Estado de destino inexistente: " + proximoEstado
            );
        }
        if (!alfabeto.contains(simbolo)) {
            throw std::invalid_argument(
                std::string("Simbolo inexistente no alfabeto: ") + simbolo
            );
        }
        transicoes[estado][simbolo] = proximoEstado;
    }

    bool aceita(const std::string& palavra) const {

        std::string estado = estadoInicial;

        for (char simbolo : palavra) {

            // Verifica se o símbolo pertence a Σ.
            if (!alfabeto.contains(simbolo)) {
                return false;
            }

            auto estadoIt = transicoes.find(estado);
            if (estadoIt == transicoes.end()) {
                return false;
            }

            auto simboloIt = estadoIt->second.find(simbolo);
            if (simboloIt == estadoIt->second.end()) {
                return false;
            }

            // q ← δ(q, símbolo)
            estado = simboloIt->second;
        }

        // w é aceita se q ∈ F.
        return estadosFinais.contains(estado);
    }
};


// =============================================================
// FÁBRICAS DOS AFDs — uma por classe léxica de especificacao/padrao_arvore.md
// =============================================================

// 1) Identificador: [a-zA-Z][a-zA-Z0-9_]*
AFD construirIdentificador() {
    AFD afd;

    afd.adicionarEstado("q0");
    afd.adicionarEstado("q1"); // final

    for (char c = 'a'; c <= 'z'; ++c) afd.adicionarSimbolo(c);
    for (char c = 'A'; c <= 'Z'; ++c) afd.adicionarSimbolo(c);
    for (char c = '0'; c <= '9'; ++c) afd.adicionarSimbolo(c);
    afd.adicionarSimbolo('_');

    afd.definirEstadoInicial("q0");
    afd.adicionarEstadoFinal("q1");

    // q0 --letra--> q1  (começa por letra)
    for (char c = 'a'; c <= 'z'; ++c) afd.adicionarTransicao("q0", c, "q1");
    for (char c = 'A'; c <= 'Z'; ++c) afd.adicionarTransicao("q0", c, "q1");

    // q1 --letra|digito|'_'--> q1  (continua com letra, dígito ou '_')
    for (char c = 'a'; c <= 'z'; ++c) afd.adicionarTransicao("q1", c, "q1");
    for (char c = 'A'; c <= 'Z'; ++c) afd.adicionarTransicao("q1", c, "q1");
    for (char c = '0'; c <= '9'; ++c) afd.adicionarTransicao("q1", c, "q1");
    afd.adicionarTransicao("q1", '_', "q1");

    return afd;
}

// 2) Numero inteiro: [0-9]+
AFD construirInteiro() {
    AFD afd;

    afd.adicionarEstado("q0");
    afd.adicionarEstado("q1"); // final

    for (char c = '0'; c <= '9'; ++c) afd.adicionarSimbolo(c);

    afd.definirEstadoInicial("q0");
    afd.adicionarEstadoFinal("q1");

    for (char c = '0'; c <= '9'; ++c) {
        afd.adicionarTransicao("q0", c, "q1"); // primeiro dígito (obrigatório)
        afd.adicionarTransicao("q1", c, "q1"); // fecho: mais dígitos
    }

    return afd;
}

// 3) Numero com sinal: [+-]?[0-9]+
AFD construirNumeroComSinal() {
    AFD afd;

    afd.adicionarEstado("q0"); // inicial
    afd.adicionarEstado("q1"); // sinal já lido, ainda não é final
    afd.adicionarEstado("q2"); // ao menos um dígito lido -> final

    afd.adicionarSimbolo('+');
    afd.adicionarSimbolo('-');
    for (char c = '0'; c <= '9'; ++c) afd.adicionarSimbolo(c);

    afd.definirEstadoInicial("q0");
    afd.adicionarEstadoFinal("q2");

    afd.adicionarTransicao("q0", '+', "q1");
    afd.adicionarTransicao("q0", '-', "q1");

    for (char c = '0'; c <= '9'; ++c) {
        afd.adicionarTransicao("q0", c, "q2"); // sem sinal, direto ao dígito
        afd.adicionarTransicao("q1", c, "q2"); // sinal + primeiro dígito
        afd.adicionarTransicao("q2", c, "q2"); // fecho: mais dígitos
    }

    return afd;
}

// 4) Texto entre aspas: "[^"]*"
AFD construirTextoEntreAspas() {
    AFD afd;

    afd.adicionarEstado("q0"); // inicial, aguardando '"' de abertura
    afd.adicionarEstado("q1"); // dentro da string
    afd.adicionarEstado("q2"); // final, '"' de fechamento já lido

    // Σ da classe: aspas + todo o intervalo ASCII imprimível (aproximação
    // do alfabeto Σ de especificacao/alfabeto.md, que também é finito).
    afd.adicionarSimbolo('"');
    for (char c = 32; c < 127; ++c) {
        if (c != '"') afd.adicionarSimbolo(c);
    }

    afd.definirEstadoInicial("q0");
    afd.adicionarEstadoFinal("q2");

    afd.adicionarTransicao("q0", '"', "q1");

    for (char c = 32; c < 127; ++c) {
        if (c != '"') afd.adicionarTransicao("q1", c, "q1"); // fecho: qualquer char != '"'
    }

    afd.adicionarTransicao("q1", '"', "q2");

    return afd;
}

// 5) Palavra fixa: bpm|wait|amp|fx
AFD construirPalavraFixa() {
    AFD afd;

    // Um estado por prefixo já lido de cada palavra (uma "trie" determinística).
    const std::vector<std::string> estadosList = {
        "q0",
        "b", "bp", "bpm",
        "w", "wa", "wai", "wait",
        "a", "am", "amp",
        "f", "fx"
    };
    for (const auto& e : estadosList) afd.adicionarEstado(e);

    for (char c : std::string("bpmwaitfx")) afd.adicionarSimbolo(c);

    afd.definirEstadoInicial("q0");
    afd.adicionarEstadoFinal("bpm");
    afd.adicionarEstadoFinal("wait");
    afd.adicionarEstadoFinal("amp");
    afd.adicionarEstadoFinal("fx");

    afd.adicionarTransicao("q0", 'b', "b");
    afd.adicionarTransicao("b",  'p', "bp");
    afd.adicionarTransicao("bp", 'm', "bpm");

    afd.adicionarTransicao("q0", 'w', "w");
    afd.adicionarTransicao("w",   'a', "wa");
    afd.adicionarTransicao("wa",  'i', "wai");
    afd.adicionarTransicao("wai", 't', "wait");

    afd.adicionarTransicao("q0", 'a', "a");
    afd.adicionarTransicao("a",  'm', "am");
    afd.adicionarTransicao("am", 'p', "amp");

    afd.adicionarTransicao("q0", 'f', "f");
    afd.adicionarTransicao("f",  'x', "fx");

    return afd;
}

// 6) Sinal: =|(|)
AFD construirSinal() {
    AFD afd;

    afd.adicionarEstado("q0");
    afd.adicionarEstado("q1"); // final

    afd.adicionarSimbolo('=');
    afd.adicionarSimbolo('(');
    afd.adicionarSimbolo(')');

    afd.definirEstadoInicial("q0");
    afd.adicionarEstadoFinal("q1");

    afd.adicionarTransicao("q0", '=', "q1");
    afd.adicionarTransicao("q0", '(', "q1");
    afd.adicionarTransicao("q0", ')', "q1");

    return afd;
}


// =============================================================
// LEITURA DO ARQUIVO — uma lexeme (token candidato) por linha
// =============================================================

std::vector<std::string> lerLexemes(const std::string& nomeArquivo) {

    std::ifstream arquivo(nomeArquivo);

    if (!arquivo.is_open()) {
        throw std::runtime_error(
            "Nao foi possivel abrir o arquivo: " + nomeArquivo
        );
    }

    std::vector<std::string> lexemes;
    std::string linha;

    while (std::getline(arquivo, linha)) {

        // Remove '\r' de arquivos salvos com quebra de linha estilo Windows.
        if (!linha.empty() && linha.back() == '\r') {
            linha.pop_back();
        }

        if (!linha.empty()) {
            lexemes.push_back(linha);
        }
    }

    return lexemes;
}


int main(int argc, char* argv[]) {

    try {

        // ==================================================
        // CONSTRUÇÃO DOS AFDs (um por classe léxica da Salmus)
        // ==================================================

        AFD afdIdentificador     = construirIdentificador();
        AFD afdInteiro           = construirInteiro();
        AFD afdNumeroComSinal    = construirNumeroComSinal();
        AFD afdTextoEntreAspas   = construirTextoEntreAspas();
        AFD afdPalavraFixa       = construirPalavraFixa();
        AFD afdSinal             = construirSinal();

        // ==================================================
        // LEITURA DO ARQUIVO (uma lexeme por linha)
        // ==================================================

        const std::string nomeArquivo = (argc > 1) ? argv[1] : "fonte.lin";

        std::vector<std::string> lexemes = lerLexemes(nomeArquivo);

        // ==================================================
        // CLASSIFICAÇÃO LÉXICA
        // ==================================================

        for (const std::string& lexeme : lexemes) {

            // Prioridade de palavra reservada sobre identificador genérico,
            // como em qualquer analisador léxico (maximal munch + reserved
            // word check).
            std::string classe;

            if (afdPalavraFixa.aceita(lexeme)) {
                classe = "Palavra fixa";
            } else if (afdIdentificador.aceita(lexeme)) {
                classe = "Identificador";
            } else if (afdInteiro.aceita(lexeme)) {
                classe = "Numero inteiro";
            } else if (afdNumeroComSinal.aceita(lexeme)) {
                classe = "Numero com sinal";
            } else if (afdTextoEntreAspas.aceita(lexeme)) {
                classe = "Texto entre aspas";
            } else if (afdSinal.aceita(lexeme)) {
                classe = "Sinal";
            } else {
                classe = "NAO RECONHECIDO";
            }

            std::cout << '"' << lexeme << "\" -> " << classe << '\n';
        }

    }
    catch (const std::exception& erro) {

        std::cerr << "Erro: " << erro.what() << '\n';
        return 1;
    }

    return 0;
}
