# Do padrão à árvore, e da árvore de volta ao padrão

## Linguagem Musical (Salmus)

Murillo Pinheiro de Oliveira  
Gustavo Marques Martins  
Lucas Spila Sereno   
Kaike Renan de Almeida   
Diego Bernardo Artero   
Fernando Caffer Mattar

## 1\. Tabela de peças

| Peça | Trecho real (exemplo.icxc) | Padrão com açúcar | Nós |
| :---- | :---- | :---- | :---- |
| Identificador | instrumento | \[a-z\]\[a-z0-9\_\]\* | 126 |
| Número inteiro | 120 | \[0-9\]+ | 40 |
| Número com sinal | \+3 | \[+-\]?\[0-9\]+ | 46 |
| Texto entre aspas | "piano" | "\[^"\]\*" | 172 |
| Palavra fixa | wait | bpm|wait|amp|fx | 23 |
| Sinal | \= | \=|(|) | 5 |

### 

### Derivações (padrão no núcleo, forma linear e contagem)

**Identificador** 

\[a-z\]\[a-z0-9\_\]\* Mesmo cálculo do exemplo do enunciado para a peça "nome": L \= alt de 26 letras gera 51 nós.  
M \= alt de 26 letras \+ 10 dígitos \+ \_ (37 símbolos) gerando 73 nós.   
Núcleo: concat(L, fecho(M)).  
   
Forma linear:   
concat(  
 alt(  
  alt(  
   ...alt('a','b')...,'z')) ,   
fecho(  
 alt(  
  alt(...('a','b')...,'z','0'...'9'),'\_'))).   
Nós: 51 \+ (73 \+ 1 fecho) \+ 1 concat \= 126 nós.

**Número inteiro** 

\[0-9\]+ D \= alt dos 10 dígitos \=   
alt(  
 alt(  
  alt(  
   alt(  
    alt(  
     alt(  
      alt(  
       alt(  
        alt('0','1'),'2'),'3'),'4'),'5'),'6'),'7'),'8'),'9') 

Gerando 19 nós.   
Núcleo: concat(D, fecho(D)).   
Forma linear: concat( D , fecho( D ) ).   
Nós: 19 \+ (19 \+ 1 fecho) \+ 1 concat \= **40** nós.

**Número com sinal**

 \[+-\]?\[0-9\]+   
Classe de sinal S \= alt('+','-') \= 3 nós.  
Opcional: x? → alt(x, ε) → 3 \+ 1 (ε) \+ 1 (alt) \= 5 nós.   
Núcleo: concat(alt(S, ε), concat(D, fecho(D))).   
Forma linear:   
concat(   
 alt(   
  alt('+','-') , ε) ,   
concat(D , fecho(D))).   
Nós: 5 \+ 40 (bloco de dígitos, calculado acima) \+ 1 concat \= 46\.

**Texto entre aspas**

"\[^"\]\*" Σ tem 85 símbolos (alfabeto[.](http://alfabeto.md)md);  
A classe "não-aspas" tem 85 − 1 \= 84 símbolos → 2·84−1 \= 167 nós.  
Núcleo: concat(concat('"', fecho(N)), '"'), onde N \= classe não-aspas.  
Forma linear: concat( concat( '"' , fecho( N ) ) , '"' ).   
Nós: 1 (abre aspas) \+ (167 \+ 1 fecho) \+ 1 concat \+ 1 (fecha aspas) \+ 1 concat \= **172**.

**Palavra fixa**  
 bpm|wait|amp|fx Cada palavra é uma cadeia de literais: bpm=5 nós, wait=7, amp=5, fx=3.   
Soma \= 20 cadeias de literais.   
Núcleo:  
alt(  
 alt(  
  alt(bpm, wait), amp), fx).   
Forma linear:   
alt(  
 alt(  
  alt(   
   concat(  
    concat('b','p'),'m'),   
concat(  
 concat(  
  concat('w','a'),'i'),'t') ),   
concat(  
 concat('a','m'),'p')),   
concat('f','x') ).  
Nós: 20 \+ 3 alternâncias \= 23 nós.

**Sinal**

\=|(|) Três símbolos literais, cada um é uma folha.  
Núcleo: alt(alt('=', '('), ')').  
Forma linear: alt( alt( '=' , '(' ) , ')' ). Nós: 3 folhas \+ 2 alt \= 5 nós.

## 2\. Cobertura mínima

Fecho aparece em duas linhas: **Identificador** (fecho(M), sobre a classe de continuação) e **Número inteiro** (fecho(D)). Opcional aparece em **Número com sinal** (alt(S, ε), cobrindo o sinal \+/- que pode ou não anteceder o valor, como em amp(nota1, \+3)).

## 3\. Um par que converge

\[0-9\]+ e \[0-9\]\[0-9\]\* descrevem o mesmo conjunto (uma ocorrência obrigatória seguida de zero ou mais). Ambos reduzem à mesma forma linear:

concat(D , fecho(D))  \= 40 nós nas duas escritas.

## 4\. Um par que não converge

(a|b)\* e (a\*b\*)\* denotam a mesma linguagem — todas as cadeias sobre {a, b} — mas suas árvores diferem.

- (a|b)\* → fecho(alt('a','b')) → 3 (alt) \+ 1 (fecho) \= **4 nós**.  
- (a\*b\*)\* → fecho(concat(fecho('a'), fecho('b'))) → 2 (fecho 'a') \+ 2 (fecho 'b') \+ 1 (concat) \+ 1 (fecho externo) \= **6 nós**.

Prova de que denotam o mesmo conjunto (por exibição de cadeia): a cadeia "ba" não é gerada por um único bloco a\*b\* (que exige todos os a antes de todos os b), mas é gerada por (a\*b\*)\* tomando dois blocos em sequência: bloco 1 \= ""a\* "b"b\* (ou seja, a\*=ε, b\*="b") e bloco 2 \= "a"a\* ""b\* (a\*="a", b\*=ε), concatenados: "b" \+ "a" \= "ba".   
Da mesma forma, qualquer intercalação de as e bs se monta concatenando blocos de um símbolo só que é exatamente o que (a|b)\* também gera.

## 5\. Dois requisitos, um de cada lado

O BPM de uma composição deve estar entre 20 e 300 (bpm ritmo \= 120 em exemplo.icxc). Parece exigir uma comparação numérica (aritmética), mas como o intervalo é finito e conhecido, é regular:

\[2-9\]\[0-9\]|1\[0-9\]\[0-9\]|2\[0-9\]\[0-9\]|300

(20–99, depois 100–199, depois 200–299, depois o caso exato 300.)

Um identificador de nota ou efeito referenciado numa chamada precisa ter sido declarado antes por um \! ou fx. Para checar isso a máquina precisaria lembrar todos os identificadores já declarados no programa, e esse conjunto não tem limite, a cada \! nomeN(...) novo que o programa declarou o teto foi expandido. Esse requisito fica fora da classe das expressões regulares e fora deste módulo.

## 6\. Quatro recusas com posição

Posições contadas a partir de 0 (0-indexed), sobre o texto do padrão que define a classe léxica não sobre um programa .icxc.

| Malformação | Padrão de entrada | Mensagem | Posição |
| :---- | :---- | :---- | :---- |
| Grupo que não fecha | (bpm|wait | grupo não fechado — esperado ) | 9 (fim do texto) |
| Repetição sem operando | \*\[0-9\] | operador de repetição sem operando: \* não pode ser o primeiro símbolo | 0 |
| Classe sem colchete final | \[a-z | classe de caracteres não fechada — esperado \] | 4 (fim do texto) |
| Símbolo sobrando depois do fim | \[0-9\]+) | símbolo inesperado após o fim da expressão: ) | 6 |

