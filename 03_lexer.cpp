#include "03_lexer.h"

#include <cctype>
#include <unordered_map>

namespace linguagem_musical {

namespace {

// Palavras reservadas — especificacao/classes_lexicas.md.
// "true"/"false" (Booleano) também são reservadas, mesmo que o núcleo
// mínimo do programa de exemplo não as utilize.
const std::unordered_map<std::string, TipoToken> kPalavrasReservadas = {
    {"bpm", TipoToken::Bpm},
    {"wait", TipoToken::Wait},
    {"amp", TipoToken::Amp},
    {"fx", TipoToken::Fx},
};

bool ehInicioDeIdentificador(char c) {
    return std::isalpha(static_cast<unsigned char>(c)) || c == '_';
}

bool ehContinuacaoDeIdentificador(char c) {
    return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
}

// Cursor sobre o texto-fonte, com contagem de linha/coluna para diagnóstico.
class Cursor {
public:
    explicit Cursor(const std::string& fonte) : fonte_(fonte) {}

    bool fimDeArquivo() const { return pos_ >= fonte_.size(); }

    char atual() const { return fimDeArquivo() ? '\0' : fonte_[pos_]; }

    char proximo() const {
        return (pos_ + 1 < fonte_.size()) ? fonte_[pos_ + 1] : '\0';
    }

    void avancar() {
        if (fimDeArquivo()) return;
        if (fonte_[pos_] == '\n') {
            ++linha_;
            coluna_ = 1;
        } else {
            ++coluna_;
        }
        ++pos_;
    }

    std::size_t linha() const { return linha_; }
    std::size_t coluna() const { return coluna_; }

private:
    const std::string& fonte_;
    std::size_t pos_ = 0;
    std::size_t linha_ = 1;
    std::size_t coluna_ = 1;
};

void pularEspacosEComentarios(Cursor& c) {
    for (;;) {
        while (!c.fimDeArquivo() &&
               std::isspace(static_cast<unsigned char>(c.atual()))) {
            c.avancar();
        }
        if (!c.fimDeArquivo() && c.atual() == '/' && c.proximo() == '/') {
            while (!c.fimDeArquivo() && c.atual() != '\n') {
                c.avancar();
            }
            continue;  // pode haver mais espaços/comentários depois
        }
        break;
    }
}

Token lerTexto(Cursor& c) {
    std::size_t linha = c.linha();
    std::size_t coluna = c.coluna();
    c.avancar();  // consome a aspa de abertura
    std::string conteudo;
    while (!c.fimDeArquivo() && c.atual() != '"') {
        conteudo.push_back(c.atual());
        c.avancar();
    }
    if (c.fimDeArquivo()) {
        throw ErroLexico{"string não fechada — esperado '\"'", linha, coluna};
    }
    c.avancar();  // consome a aspa de fechamento
    return Token{TipoToken::Texto, conteudo, linha, coluna};
}

Token lerNumero(Cursor& c) {
    std::size_t linha = c.linha();
    std::size_t coluna = c.coluna();
    std::string lexema;
    if (c.atual() == '+' || c.atual() == '-') {
        lexema.push_back(c.atual());
        c.avancar();
    }
    while (!c.fimDeArquivo() &&
           std::isdigit(static_cast<unsigned char>(c.atual()))) {
        lexema.push_back(c.atual());
        c.avancar();
    }
    if (!c.fimDeArquivo() && c.atual() == '.' &&
        std::isdigit(static_cast<unsigned char>(c.proximo()))) {
        lexema.push_back('.');
        c.avancar();
        while (!c.fimDeArquivo() &&
               std::isdigit(static_cast<unsigned char>(c.atual()))) {
            lexema.push_back(c.atual());
            c.avancar();
        }
    }
    return Token{TipoToken::Numero, lexema, linha, coluna};
}

Token lerIdentificadorOuPalavraReservada(Cursor& c) {
    std::size_t linha = c.linha();
    std::size_t coluna = c.coluna();
    std::string lexema;
    while (!c.fimDeArquivo() && ehContinuacaoDeIdentificador(c.atual())) {
        lexema.push_back(c.atual());
        c.avancar();
    }
    auto it = kPalavrasReservadas.find(lexema);
    TipoToken tipo =
        (it != kPalavrasReservadas.end()) ? it->second : TipoToken::Identificador;
    return Token{tipo, lexema, linha, coluna};
}

}  // namespace

std::vector<Token> tokenizar(const std::string& fonte) {
    std::vector<Token> tokens;
    Cursor c(fonte);

    for (;;) {
        pularEspacosEComentarios(c);
        if (c.fimDeArquivo()) {
            tokens.push_back(Token{TipoToken::FimDeArquivo, "", c.linha(), c.coluna()});
            break;
        }

        std::size_t linha = c.linha();
        std::size_t coluna = c.coluna();
        char ch = c.atual();

        if (ch == '@') {
            c.avancar();
            tokens.push_back(Token{TipoToken::Timbre, "@", linha, coluna});
        } else if (ch == '!') {
            c.avancar();
            tokens.push_back(Token{TipoToken::NotaTok, "!", linha, coluna});
        } else if (ch == '=') {
            c.avancar();
            tokens.push_back(Token{TipoToken::Igual, "=", linha, coluna});
        } else if (ch == '(') {
            c.avancar();
            tokens.push_back(Token{TipoToken::AbreParen, "(", linha, coluna});
        } else if (ch == ')') {
            c.avancar();
            tokens.push_back(Token{TipoToken::FechaParen, ")", linha, coluna});
        } else if (ch == ',') {
            c.avancar();
            tokens.push_back(Token{TipoToken::Virgula, ",", linha, coluna});
        } else if (ch == '"') {
            tokens.push_back(lerTexto(c));
        } else if (std::isdigit(static_cast<unsigned char>(ch)) ||
                   ((ch == '+' || ch == '-') &&
                    std::isdigit(static_cast<unsigned char>(c.proximo())))) {
            tokens.push_back(lerNumero(c));
        } else if (ehInicioDeIdentificador(ch)) {
            tokens.push_back(lerIdentificadorOuPalavraReservada(c));
        } else {
            throw ErroLexico{
                std::string("símbolo fora do alfabeto da linguagem: '") + ch + "'",
                linha, coluna};
        }
    }

    return tokens;
}

std::string nomeToken(TipoToken tipo) {
    switch (tipo) {
        case TipoToken::Bpm: return "'bpm'";
        case TipoToken::Timbre: return "'@'";
        case TipoToken::NotaTok: return "'!'";
        case TipoToken::Fx: return "'fx'";
        case TipoToken::Amp: return "'amp'";
        case TipoToken::Wait: return "'wait'";
        case TipoToken::Identificador: return "identificador";
        case TipoToken::Numero: return "número";
        case TipoToken::Texto: return "texto entre aspas";
        case TipoToken::Igual: return "'='";
        case TipoToken::AbreParen: return "'('";
        case TipoToken::FechaParen: return "')'";
        case TipoToken::Virgula: return "','";
        case TipoToken::FimDeArquivo: return "fim de arquivo";
    }
    return "token desconhecido";
}

}  // namespace linguagem_musical
