#include "04_parser.h"

namespace linguagem_musical {

namespace {

class Analisador {
public:
    explicit Analisador(const std::vector<Token>& tokens) : tokens_(tokens) {}

    std::vector<Comando> analisarPrograma() {
        std::vector<Comando> comandos;
        while (atual().tipo != TipoToken::FimDeArquivo) {
            comandos.push_back(analisarComando());
        }
        return comandos;
    }

private:
    const std::vector<Token>& tokens_;
    std::size_t pos_ = 0;

    const Token& atual() const { return tokens_[pos_]; }

    const Token& consumir(TipoToken esperado) {
        if (atual().tipo != esperado) {
            throw ErroSintatico{
                "esperado " + nomeToken(esperado) + ", encontrado " +
                    nomeToken(atual().tipo) +
                    (atual().lexema.empty() ? "" : " ('" + atual().lexema + "')"),
                atual().linha};
        }
        const Token& tok = atual();
        ++pos_;
        return tok;
    }

    static long paraInteiro(const Token& tok) {
        return std::stol(tok.lexema);
    }

    Comando analisarComando() {
        switch (atual().tipo) {
            case TipoToken::Bpm: return analisarBpm();
            case TipoToken::Timbre: return analisarTimbre();
            case TipoToken::NotaTok: return analisarNota();
            case TipoToken::Fx: return analisarEfeito();
            case TipoToken::Amp: return analisarAmplificar();
            case TipoToken::Wait: return analisarEspera();
            default:
                throw ErroSintatico{
                    "token inesperado no início de um comando: " +
                        nomeToken(atual().tipo),
                    atual().linha};
        }
    }

    // bpm ID "=" NUMERO
    Comando analisarBpm() {
        Comando cmd;
        cmd.tipo = TipoComando::DeclaracaoBpm;
        cmd.linha = atual().linha;
        consumir(TipoToken::Bpm);
        cmd.nomeVariavel = consumir(TipoToken::Identificador).lexema;
        consumir(TipoToken::Igual);
        cmd.valorInteiro = paraInteiro(consumir(TipoToken::Numero));
        return cmd;
    }

    // "@" ID "=" TEXTO
    Comando analisarTimbre() {
        Comando cmd;
        cmd.tipo = TipoComando::DeclaracaoTimbre;
        cmd.linha = atual().linha;
        consumir(TipoToken::Timbre);
        cmd.nomeVariavel = consumir(TipoToken::Identificador).lexema;
        consumir(TipoToken::Igual);
        cmd.valorTexto = consumir(TipoToken::Texto).lexema;
        return cmd;
    }

    // "!" ID "(" ID "," NUMERO "," ID ")"
    Comando analisarNota() {
        Comando cmd;
        cmd.tipo = TipoComando::DeclaracaoNota;
        cmd.linha = atual().linha;
        consumir(TipoToken::NotaTok);
        cmd.idNota = consumir(TipoToken::Identificador).lexema;
        consumir(TipoToken::AbreParen);
        cmd.notaLetra = consumir(TipoToken::Identificador).lexema;
        consumir(TipoToken::Virgula);
        cmd.oitava = paraInteiro(consumir(TipoToken::Numero));
        consumir(TipoToken::Virgula);
        cmd.duracaoNome = consumir(TipoToken::Identificador).lexema;
        consumir(TipoToken::FechaParen);
        return cmd;
    }

    // "fx" ID "(" ID "," NUMERO ")"
    Comando analisarEfeito() {
        Comando cmd;
        cmd.tipo = TipoComando::ChamadaEfeito;
        cmd.linha = atual().linha;
        consumir(TipoToken::Fx);
        cmd.nomeEfeito = consumir(TipoToken::Identificador).lexema;
        consumir(TipoToken::AbreParen);
        cmd.idAlvo = consumir(TipoToken::Identificador).lexema;
        consumir(TipoToken::Virgula);
        cmd.parametroTexto = consumir(TipoToken::Numero).lexema;
        consumir(TipoToken::FechaParen);
        return cmd;
    }

    // "amp" "(" ID "," NUMERO ")"
    Comando analisarAmplificar() {
        Comando cmd;
        cmd.tipo = TipoComando::ChamadaAmplificar;
        cmd.linha = atual().linha;
        consumir(TipoToken::Amp);
        consumir(TipoToken::AbreParen);
        cmd.idAlvo = consumir(TipoToken::Identificador).lexema;
        consumir(TipoToken::Virgula);
        cmd.deltaVolumeTexto = consumir(TipoToken::Numero).lexema;
        consumir(TipoToken::FechaParen);
        return cmd;
    }

    // "wait" NUMERO
    Comando analisarEspera() {
        Comando cmd;
        cmd.tipo = TipoComando::Espera;
        cmd.linha = atual().linha;
        consumir(TipoToken::Wait);
        cmd.duracaoMs = paraInteiro(consumir(TipoToken::Numero));
        return cmd;
    }
};

}  // namespace

std::vector<Comando> analisar(const std::vector<Token>& tokens) {
    Analisador analisador(tokens);
    return analisador.analisarPrograma();
}

}  // namespace linguagem_musical
