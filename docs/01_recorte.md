# Recorte da linguagem 
Registro das 3 decisões pedidos pela 1 tarefa do projeto, registrando o núcleo mínimo (padrões considerados essenciais ou minimamente necessários)
e padrões ou decisões que foram descartadas e o motivo do descarte. 

## Domínio em que a linguagem irá atuar
**Decisão** - Musica, mais especificamente sobre a manipulação de sons, efeitos sonoros e instrumentos musicais. o intuito seria criar uma linguagem que poderia integrar a criação de musicas
ao aplicar amostras (samples) de instrumentos e/ou efeitos sonoros, assim como reproduzi-la. 

**Núcleo Mínimo** - os aspectos mais importantes e essenciais da linguagem seria a implementação da manipulação e reprodução, outras funcionalidades importantes mas não essenciais
seria a representação gráfica de partituras ou das notas da musica, importar musicas já criadas e então converte-las em um objeto manipulável pela linguagem.

**Descartado** - formas de converter musica em outros tipos de estilo ou quantidade de bits, por exemplo: converter uma musica de uma banda, como "Red Hot Chili Peppers - Californication" em 8 bits, esse estilo ou 
formato tem esse termo "8 bits" por usar os efeitos sonoros usando em jogos antigos feitos nesse formato. Outra funcionalidade descartada seria dá pessoa importar suas proprias amostras de sons de instrumento invés de amostras pré-prontas.

## Que classe de padrões que o sistema aceita.
**Decisão** - Expressões regulares aceitas com concatenação, alternância ("|"), fecho Kleene (*), fecho positivo (+), opcional (?), classe de caracteres ([...]), classe negativa (^) e coringa ('.'), além de agrupamento por parênteses.
Definição de classes de objetos, definição de variáveis (de tipos primitivos, e próprios do sistema — como por exemplo de amostras de instrumentos ou efeitos sonoros), e criação de funções que descrevem uma sequência musical (notas, blocos repetidos, aplicação de efeitos).    


**Núcleo Mínimo** - Concatenação, alternância e fecho. Todas as outras quatro classes podem ser reduzidas ao núcleo:
um exemplo de fecho positivo: "a+" após a leitura vira "aa*"; uma classe de caracteres '[cdf]' vira uma alternância '(c | d | f)'.


**Descartado** - Retrovisor (backreference) e grupo de captura: saem da classe das linguagens regulares, e um padrão que os usasse não poderia ser compilado para autômato finito. Lookahead / lookbehind: não são necessários para o modelo de expressões regulares adotado, e algumas formas não podem ser representadas pelo núcleo sem mecanismos adicionais.

## Que forma tem a descrição escrita pelo usuário.

**Decisão** - Um programa é uma sequência de declarações: declarações de variáveis (tipos primitivos — int, str, f, bool — e tipos próprios do sistema — Nota, Efeito, Timbre), seguida de comandos que descrevem a composição musical (notas com oitava e duração, aplicação de efeito, definição de BPM e instrumento).

**Núcleo Mínimo** - A declaração de variáveis e a sequência ordenada de comandos musicais (nota com oitava e duração, aplicação de efeito, definição de BPM) são obrigatórias.

**Descartado** - Estruturas de controle condicional aninhadas e importação de módulos externos ficam fora do nível mínimo

## O que o sistema produz (Nivel 1).
**Decisão** - A saída é uma representação estruturada da composição: uma lista ordenada de eventos musicais (instrumento, nota, oitava, duração, efeitos aplicados, BPM), pronta para ser interpretada por um player externo ou impressa como partitura textual. Síntese de áudio real (gerar um .wav) fica fora do nível 1.

**Núcleo Mínimo** - A lista ordenada de eventos, cada um com nota, oitava e duração, é obrigatória; efeito e timbre são anexados ao evento quando presentes no programa de entrada.

**Descartado** - Geração de arquivo de áudio e renderização gráfica de partitura ficam fora do nível 1.

--- Gramática Hospedeira (Linguagem que o usuário escreve) ---
* program := declaration*;
* declaration := type ID "=" value;
* value := ("INTEGER" | "ID" | "STRING"| "BOOLEAN" | "FLOAT" | "NOTE" | "EFFECT" | "TIMBRE");
* note         := "Nota" "(" NUMBER "," NUMBER ")" ;
*effect       := "Efeito" "(" STRING ")" ;
*timbre       := "Timbre" "(" STRING ")" ;
* andExpr := cmpExpr("and" cmpExpr)*;
* cmpExpr := ("!="|"=>"|"<="|"=="|"<"| ">");
* type := ("int" | "str" | " bool" | "float"| "nota" | "efeito" | "timbre");
* Primary := ID | NUMBER | STRING | "value" "("ID") | ("expr")";

--- Mini Linguagem Regular (Alvo dos Automatos) ---
regex  := alt ;
alt    := concat ( "|" concat )* ;
concat := repeat+ ;
repeat := atom ( "*" | "+" | "?" )? ;
atom   := CHAR | "." | "[" classe "]" | "(" alt ")" ;

