# Recorte da linguagem 
Registro das 3 decisões pedidos pela 1 tarefa do projeto, registrando o núcleo mínimo (padrões considerados essenciais ou minimamente necessários)
e padrões ou decisões que foram descartadas e o motivo do descarte. 
Domínio em que a linguagem irá atuar: Musica.

## Que classe de padrões que o sistema aceita.
**Decisão** - expressões regulares aceitas com concatenação, alternância ("|"), fecho kleene (*), fecho positivo (+),  opcional (?),      
classe de caracteres ( [...]), classe negativa (^) ,coringa ('.') e agrupamento por parênteses.

**Núcleo Mínimo** - concatenação, alternância e fecho. todas as outras quatro classes podem ser reduzidas ao nucleo
um exemplo de fecho positivo: "a+" após a leitura vira "aa*", uma classe de caracteres '[cdf]' vira uma alternancia '(a | b | c)';


**Descartado** - 

## Que forma tem a descrição escrita pelo usuário.

**Decisão** - Um programa é uma sequência de declarações 'variables', seguinda de um.

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

