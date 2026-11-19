# ProjetoCompilador
este repositório será utilizado para armazenar o conteúdo, criações ou atividades desenvolvidas na matéria de compiladores e linguagem formal do 6 - termo de Ciências da Computação da UNIMAR.

## Linguagem Musical (Salmus) — do padrão ao AFD mínimo

## Rodar os testes (Partes 1 e 2)

```
make test
```

No Windows com MinGW (`make` costuma não existir, e o `libstdc++` do Git no PATH
pode ser carregado no lugar do do compilador e dar segfault):

```
mingw32-make test LDFLAGS=-static
```

O comando compila e roda `04_afn_teste` e `05_afd_teste` sobre `src/AFN/casos_afn.txt`;
sai com código 0 se tudo passou. Resultado atual: 212/212 e 90/90 verificações.

Demonstração (imprime estados antes/depois da minimização e a tabela de explosão):

```
make demo                                  # (a|b)*c e a tabela de explosão
./05_demo "(a|b)*c" abc 4                  # padrão, alfabeto, comprimento máximo
```

## Onde está cada parte

| Parte | Arquivos |
|---|---|
| Leitura do padrão → árvore do núcleo | `src/AFN/02_regex.h`, `src/AFN/02_regex.cpp` |
| Construção de Thompson (6 casos: símbolo, ε, ∅, concatenação, alternância, fecho; `+ ? [] . {n,m}` já chegam reduzidos ao núcleo) | `src/AFN/04_afn.h`, `src/AFN/04_afn.cpp` (`construirThompson`) |
| Fecho vazio, movimento por símbolo, simulação do AFN | `src/AFN/04_afn.cpp` (`Afn::fechoVazio`, `Afn::mover`, `Afn::simular`) |
| Determinização (subconjuntos; guarda o conjunto do AFN de cada estado em `Afd::origem`) | `src/AFN/05_afd.h`, `src/AFN/05_afd.cpp` (`determinizar`) |
| Minimização (completar, inacessíveis, partição inicial, refinamento até o ponto fixo) | `src/AFN/05_afd.cpp` (`completar`, `removerInacessiveis`, `minimizar`) |
| Comparação das três máquinas (os três pares) | `src/AFN/05_afd.cpp` (`compararTres`) |
| Testes da Parte 1 | `src/AFN/04_afn_teste.cpp`, `src/AFN/04_afn_fuzz.cpp` (diferencial contra `std::regex`) |
| Testes da Parte 2 | `src/AFN/05_afd_teste.cpp` |
| Bateria de casos conferidos à mão (padrão, cadeia, resposta) | `src/AFN/casos_afn.txt` (250 casos; usada pelas duas partes, nas três máquinas) |
| Programa que imprime as contagens | `src/AFN/05_demo.cpp` |

O conjunto vazio **é** um estado do AFD (o estado de erro), então a determinização
já sai completa. `minimizar` ainda completa a entrada, para aceitar AFDs
incompletos de outra origem.

## Conferência com (a|b)*c, alfabeto {a, b, c}

- AFN de Thompson: 10 estados, 12 transições, 9 vazias; aceita `ababac`, recusa `abab`.
- AFD: **5 estados** antes da minimização (um deles é o conjunto vazio).
- AFD mínimo completo: **3 estados** depois (início, aceitação, erro).
- Cadeias até comprimento 4: 121; as três máquinas aceitam as mesmas 15; nenhuma
  divergência nos pares AFN×AFD, AFD×mínimo e AFN×mínimo.

## Crescimento de estados na determinização

Padrão: `(a|b)*a(a|b){n-1}` sobre {a,b} — "o n-ésimo símbolo a partir do fim é a".
Para decidir é preciso lembrar as n últimas letras, então são 2^n estados no mínimo.
Contagens impressas por `./05_demo`:

|  n | AFN | AFD  | mínimo | 2^n  |
|---:|----:|-----:|-------:|-----:|
|  1 |  12 |    3 |      2 |    2 |
|  2 |  16 |    5 |      4 |    4 |
|  3 |  22 |    9 |      8 |    8 |
|  4 |  28 |   17 |     16 |   16 |
|  5 |  34 |   33 |     32 |   32 |
|  6 |  40 |   65 |     64 |   64 |
|  7 |  46 |  129 |    128 |  128 |
|  8 |  52 |  257 |    256 |  256 |
|  9 |  58 |  513 |    512 |  512 |
| 10 |  64 | 1025 |   1024 | 1024 |
| 11 |  70 | 2049 |   2048 | 2048 |
| 12 |  76 | 4097 |   4096 | 4096 |

O AFN cresce linearmente (6n+4 estados para n ≥ 2; n = 1 é menor porque `{0}` vira ε),
o AFD cresce como 2^n + 1 e o mínimo como 2^n: a minimização não desfaz a explosão,
que é intrínseca à linguagem. O "+1" é o conjunto inicial, que contém a entrada do
fecho de Thompson e nunca reaparece. O AFN reconhece a mesma linguagem com
poucos estados porque "adivinha" onde está o n-ésimo símbolo do fim.
