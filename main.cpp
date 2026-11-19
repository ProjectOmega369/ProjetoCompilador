// main.cpp — Ponto de entrada do interpretador da Linguagem Musical (Salmus).
//
// Faz exatamente os dois primeiros passos pedidos pela tarefa:
//
//   1. Interpretador escrito na linguagem hospedeira C++.
//   2. Leitura do arquivo com a função de leitura padrão da linguagem
//      hospedeira: em C++ o equivalente idiomático de open()/fopen() é
//      std::ifstream (é a própria biblioteca padrão de C++ para E/S de
//      arquivos — fopen() também funcionaria, já que C++ inclui a
//      biblioteca padrão de C, mas ifstream é a forma recomendada em C++).
//
// Uso:
//   ./interpretador caminho/para/programa.icxc
//   ./interpretador                                  (usa docs/exemplo.icxc)

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "03_lexer.h"
#include "04_parser.h"
#include "05_interpretador.h"

namespace {

// Abre o arquivo com ifstream e devolve todo o seu conteúdo como string.
// Lança std::runtime_error se o arquivo não puder ser aberto.
std::string lerArquivo(const std::string& caminho) {
    std::ifstream arquivo(caminho);
    if (!arquivo.is_open()) {
        throw std::runtime_error("não foi possível abrir o arquivo: " + caminho);
    }
    std::ostringstream conteudo;
    conteudo << arquivo.rdbuf();
    return conteudo.str();
}

}  // namespace

int main(int argc, char** argv) {
    const std::string caminho =
        (argc > 1) ? argv[1] : "docs/exemplo.icxc";

    std::string fonte;
    try {
        fonte = lerArquivo(caminho);
    } catch (const std::runtime_error& erro) {
        std::cerr << "Erro de leitura: " << erro.what() << "\n";
        return 1;
    }

    using namespace linguagem_musical;

    try {
        std::vector<Token> tokens = tokenizar(fonte);
        std::vector<Comando> comandos = analisar(tokens);
        SaidaInterpretacao saida = interpretar(comandos);
        std::cout << formatarSaida(saida);
    } catch (const ErroLexico& erro) {
        std::cerr << "Erro léxico (linha " << erro.linha << ", coluna "
                   << erro.coluna << "): " << erro.mensagem << "\n";
        return 1;
    } catch (const ErroSintatico& erro) {
        std::cerr << "Erro sintático (linha " << erro.linha << "): "
                   << erro.mensagem << "\n";
        return 1;
    } catch (const ErroSemantico& erro) {
        std::cerr << "Erro semântico (linha " << erro.linha << "): "
                   << erro.mensagem << "\n";
        return 1;
    }

    return 0;
}
