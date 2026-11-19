#include "05_interpretador.h"

#include <sstream>
#include <unordered_map>

namespace linguagem_musical {

namespace {

constexpr long kBpmMinimo = 20;
constexpr long kBpmMaximo = 300;

}  // namespace

SaidaInterpretacao interpretar(const std::vector<Comando>& comandos) {
    SaidaInterpretacao saida;

    // Tabela de símbolos: id da nota -> posição em saida.eventos.
    // É exatamente a "memória ilimitada de identificadores já vistos" que
    // padrao_arvore.md (seção 5) descreve como necessária e fora do
    // alcance de um autômato finito.
    std::unordered_map<std::string, std::size_t> notasDeclaradas;

    for (const Comando& cmd : comandos) {
        switch (cmd.tipo) {
            case TipoComando::DeclaracaoBpm: {
                if (cmd.valorInteiro < kBpmMinimo || cmd.valorInteiro > kBpmMaximo) {
                    throw ErroSemantico{
                        "bpm " + std::to_string(cmd.valorInteiro) +
                            " fora do intervalo aceito [20, 300]",
                        cmd.linha};
                }
                saida.bpm = cmd.valorInteiro;
                break;
            }

            case TipoComando::DeclaracaoTimbre: {
                saida.instrumento = cmd.valorTexto;
                break;
            }

            case TipoComando::DeclaracaoNota: {
                if (notasDeclaradas.count(cmd.idNota)) {
                    throw ErroSemantico{
                        "nota '" + cmd.idNota + "' já foi declarada antes",
                        cmd.linha};
                }
                Evento evento;
                evento.idNota = cmd.idNota;
                evento.nota = cmd.notaLetra;
                evento.oitava = cmd.oitava;
                evento.duracao = cmd.duracaoNome;
                notasDeclaradas[cmd.idNota] = saida.eventos.size();
                saida.eventos.push_back(std::move(evento));
                break;
            }

            case TipoComando::ChamadaEfeito: {
                auto it = notasDeclaradas.find(cmd.idAlvo);
                if (it == notasDeclaradas.end()) {
                    throw ErroSemantico{
                        "efeito '" + cmd.nomeEfeito + "' referencia '" + cmd.idAlvo +
                            "', que não foi declarado antes por '!'",
                        cmd.linha};
                }
                Evento& alvo = saida.eventos[it->second];
                alvo.efeitoNome = cmd.nomeEfeito;
                alvo.efeitoParametroTexto = cmd.parametroTexto;
                break;
            }

            case TipoComando::ChamadaAmplificar: {
                auto it = notasDeclaradas.find(cmd.idAlvo);
                if (it == notasDeclaradas.end()) {
                    throw ErroSemantico{
                        "amp referencia '" + cmd.idAlvo +
                            "', que não foi declarado antes por '!'",
                        cmd.linha};
                }
                saida.eventos[it->second].volumeTexto = cmd.deltaVolumeTexto;
                break;
            }

            case TipoComando::Espera: {
                std::ostringstream msg;
                msg << "espera: " << cmd.duracaoMs << "ms";
                if (saida.eventos.empty()) {
                    msg << " antes de qualquer evento";
                } else {
                    msg << " após o evento " << saida.eventos.size();
                }
                saida.mensagensEspera.push_back(msg.str());
                break;
            }
        }
    }

    return saida;
}

std::string formatarSaida(const SaidaInterpretacao& saida) {
    std::ostringstream out;

    if (saida.bpm) {
        out << "ritmo: " << *saida.bpm << " bpm\n";
    }
    if (saida.instrumento) {
        out << "instrumento: " << *saida.instrumento << "\n";
    }
    out << "\n";

    for (std::size_t i = 0; i < saida.eventos.size(); ++i) {
        const Evento& evento = saida.eventos[i];
        out << "evento " << (i + 1) << " -> nota: " << evento.nota
            << ", oitava: " << evento.oitava
            << ", duracao: " << evento.duracao;
        if (evento.volumeTexto) {
            out << ", volume: " << *evento.volumeTexto << "db";
        }
        if (evento.efeitoNome) {
            out << ", efeito: " << *evento.efeitoNome << "("
                << *evento.efeitoParametroTexto << ")";
        }
        out << "\n";
    }

    for (const std::string& linha : saida.mensagensEspera) {
        out << linha << "\n";
    }

    return out.str();
}

}  // namespace linguagem_musical
