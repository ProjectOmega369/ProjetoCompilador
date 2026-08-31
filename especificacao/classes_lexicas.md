Classes Lexicais da Linguagem

Apenas as formas que o analisador reconhece caractere a caractere (tokens). Para
palavras reservadas (Ritmo, Timbre, Espera, Nota, Amplificar, Efeito), a classe léxica
é só o token em si — o que cada uma *faz* na linguagem está descrito na seção
"Construções da linguagem", abaixo, e não aqui.

* Identificador — sequência de letras, dígitos e `_`, começando por letra.

* Inteiro (int) — sequência de dígitos; define variáveis numéricas do tipo inteiro.

* String (str) — sequência de caracteres entre aspas; define variáveis do tipo texto.

* Float (f) — dígitos com ponto decimal; define variáveis numéricas com casa decimal.

* Booleano (bool) — palavra reservada `true` ou `false`.

* Ritmo (bpm) — palavra reservada `bpm`; introduz a unidade de medida de velocidade da música.

* Timbre (@) — símbolo reservado `@`; introduz a origem do som (instrumento) a ser usado.

* Espera (wait) — palavra reservada `wait`; introduz a unidade de tempo a ser esperado.

* Nota (!) — símbolo reservado `!`; introduz a declaração de uma nota musical.

* Amplificar (amp) — palavra reservada `amp`; introduz uma chamada de amplificação.

* Efeito (fx) — palavra reservada `fx`; introduz a declaração de um efeito.

## Construções da linguagem (nível sintático/semântico, não léxico)

Estas não são classes léxicas — são o que o parser monta a partir dos tokens acima.

* **Nota** — objeto composto por 3 parâmetros (nota, oitava, duração), introduzido pelo
  token `!`.

* **Amplificar** — função que altera a altura do som em decibéis, invocada pelo token
  `amp`.

* **Efeito** — objeto responsável por adicionar um efeito a uma música, introduzido pelo
  token `fx`.
