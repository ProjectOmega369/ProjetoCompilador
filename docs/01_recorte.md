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
**Decisão** - expressões regulares aceitas com concatenação, alternância ("|"), fecho Kleene (*), fecho positivo (+),  opcional (?),      
classe de caracteres ( [...]), classe negativa (^) ,coringa ('.') e agrupamento por parênteses.
Definição de classes de objetos, definição de variáveis (de tipos primitivos, e próprios do sistema - como por exemplo de amostras de instrumentos ou efeitos sonoros), criação de     


**Núcleo Mínimo** - concatenação, alternância e fecho. todas as outras quatro classes podem ser reduzidas ao núcleo
um exemplo de fecho positivo: "a+" após a leitura vira "aa*", uma classe de caracteres '[cdf]' vira uma alternância '(a | b | c)';


**Descartado** - 

## Que forma tem a descrição escrita pelo usuário.

**Decisão** - Um programa é uma sequência de declarações contendo 'Variables', 'classes', ''.

**Núcleo Mínimo** - 

**Descartado** - 

## O que o sistema produz (Nivel 1).
**Decisão** - 

**Núcleo Mínimo** -

**Descartado** - 

--- Gramática Hospedeira (Linguagem que o usuário escreve) ---
* andExpr := cmpExpr("and" cmpExpr)*;
* cmpExpr := ("!="|"=>"|"<="|"=="|"<"| ">");
* Primary := ID | NUMBER | STRING | "value" "("ID") | ("expr")";

--- Mini Linguagem Regular (Alvo dos Automatos) ---
regex  := alt ;
alt    := concat ( "|" concat )* ;
concat := repeat+ ;
repeat := atom ( "*" | "+" | "?" )? ;
atom   := CHAR | "." | "[" classe "]" | "(" alt ")" ;

