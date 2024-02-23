# parser-c
Simple parser written in C. It validates the source code and creates a parse tree from it. My original class project pseudo-parser written in Racket is [here](https://github.com/gschick3/parser-racket).

## Language Grammar

```
program -> linelist $$ 
linelist -> line linelist | epsilon 
line ->  label stmt linetail 
label -> id : | epsilon 
linetail -> ; stmt linetail | epsilon 
stmt -> id = expr 
	| if (boolean) stmt 
	| while (boolean) linelist endwhile
	| read id
	| write expr
	| goto id
	| gosub id
	| return
	| break
	| end
boolean -> true | false | expr bool-op expr 
bool-op -> < | > | >= | <= | <> | =
expr -> id etail | num etail | (expr) 
etail -> + expr | - expr | * expr | / expr | epsilon
id -> [a-zA-Z][a-zA-Z0-9]*
num -> numsign digits
numsign -> + | - | epsilon 
digits -> [0-9]*
```

