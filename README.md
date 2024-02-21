# parser-c
Simple pseudo-parser written in C. My original class project parser written in Racket is [here](https://github.com/gschick3/parser-racket).

## Language Grammar

This language is a simplified version of ALGOL

```
program -> linelist $$ 
linelist -> line linelist | epsilon 
line ->  label stmt linetail 
label -> id: | epsilon 
linetail -> ;stmt+ | epsilon 
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
num -> numsign digit digit*
numsign -> + | - | epsilon 
digit -> [0-9]
```

