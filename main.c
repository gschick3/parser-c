#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

#define MAX_LINE_LENGTH 255
#define MAX_PROG_LENGTH 2048

struct Program {
    char buff[MAX_PROG_LENGTH];
    int len;
    int nextChar;
};

enum TokenType {END, DIGITS, SYMBOL, IDENTIFIER};

struct Token {
    enum TokenType type;
    char buff[MAX_LINE_LENGTH];
    int len;
};

void parse(char[100]);

void move_char(struct Token *dest, struct Program *src);
struct Token next_token(struct Program*);

bool is_token(struct Program*, const char*);
bool is_digits(struct Program*);
bool is_numsign(struct Program*);
bool is_id(struct Program*);
bool is_boolop(struct Program*);
bool is_num(struct Program*);
bool is_etail(struct Program*);
bool is_expr(struct Program*);
bool is_boolean(struct Program*);
bool is_stmt(struct Program*);
bool is_linetail(struct Program*);
bool is_label(struct Program*);
bool is_line(struct Program*);
bool is_linelist(struct Program*);
bool is_program(struct Program*);

int main() {
    char filename[100];
    for (int i = 1; i <= 5; i++) {
        snprintf(filename, 100, "../test_files/test%d.txt", i);
        printf("test%d.txt: ", i);
        parse(filename);
    }

    return 0;
}

void parse(char filename[100]) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("File does not exist: %s\n", filename);
        return;
    }

    char c;
    struct Program program = {"", 0, 0};
    while((c = fgetc(fp)) != EOF && program.len < MAX_PROG_LENGTH)
        program.buff[program.len++] = c;
    fclose(fp);

    if (is_program(&program))
        printf("Success.\n");
    else
        printf("Error on token: '%s'\n", next_token(&program).buff);
}

void move_char(struct Token *dest, struct Program *src) {
    dest->buff[dest->len++] = src->buff[src->nextChar++];
}

struct Token next_token(struct Program *p) {
    struct Token t = {END, "", 0};

    while (p->nextChar <= p->len && isspace(p->buff[p->nextChar]))
        p->nextChar++;

    if (p->nextChar > p->len)
        return t;

    if (isalpha(p->buff[p->nextChar])) {
        t.type = IDENTIFIER;
        while (p->nextChar <= p->len && isalnum(p->buff[p->nextChar]))
            move_char(&t, p);
    } else if (isdigit(p->buff[p->nextChar])) {
        t.type = DIGITS;
        while (p->nextChar <= p->len && isdigit(p->buff[p->nextChar]))
            move_char(&t, p);
    } else if (p->buff[p->nextChar] == '(' || p->buff[p->nextChar] == ')') {
        t.type = SYMBOL;
        move_char(&t, p);
    } else {
        t.type = SYMBOL;
        while (p->nextChar <= p->len && !isalnum(p->buff[p->nextChar]) && !isspace(p->buff[p->nextChar]))
            move_char(&t, p);
    }
    return t;
}

bool is_token(struct Program *p, const char* token_name) {
    const struct Token next = next_token(p);
    if (strncmp(next.buff, token_name, next.len + 1)) { // strncmp returns 0 if match, 1 or -1 otherwise
        p->nextChar -= next.len;
        return false;
    }

    return true;
}

bool is_digits(struct Program *p) {
    const struct Token next = next_token(p); // This will remove tested tokens
    // if successful, no need to do anything
    if (next.type != DIGITS) {
        // otherwise, change state to failure and add tokens back
        p->nextChar -= next.len;
        return false;
    }

    return true;
}

bool is_numsign(struct Program *p) {
    // numsign can be empty, so always return true
    return is_token(p, "+") || is_token(p, "-") || true;
}

bool is_id(struct Program *p) {
    const struct Token next = next_token(p);
    const char reserved[10][10] = {"if", "while", "read", "write", "goto", "gosub", "return", "break", "end", "endwhile"};
    for (int i = 0; i < 10; i++) {
        if (!strncmp(next.buff, reserved[i], next.len + 1)) {
            p->nextChar -= next.len;
            return false;
        }
    }
    if (next.type != IDENTIFIER) {
        p->nextChar -= next.len;
        return false;
    }

    return true;
}

bool is_boolop(struct Program *p) {
    return is_token(p, ">")
        || is_token(p, "<")
        || is_token(p, ">=")
        || is_token(p, "<=")
        || is_token(p, "<>")
        || is_token(p, "=");
}

bool is_num(struct Program *p) {
    // numsign will always evaluate to true
    return is_numsign(p) && is_digits(p);
}

bool is_etail(struct Program *p) {
    return ((is_token(p, "+") || is_token(p, "-")
            || is_token(p, "*") || is_token(p, "/"))
            && is_expr(p)) || true;
}

bool is_expr(struct Program *p) {
    if (is_id(p) || is_num(p))
        return is_etail(p);
    return is_token(p, "(") && is_expr(p) && is_token(p, ")");
}

bool is_boolean(struct Program *p) {
    if (is_token(p, "true") || is_token(p, "false"))
        return true;
    return is_expr(p) && is_boolop(p) && is_expr(p);
}

bool is_stmt(struct Program *p) {
    const int orig_pos = p->nextChar;
    if (is_id(p)) {
        if (is_token(p, "=") && is_expr(p))
            return true;
        p->nextChar = orig_pos; // we have to reset this bc id can be in the same pos for label as for stmt and we won't know
        return false;
    }
    if (is_token(p, "if"))
        return is_token(p, "(") && is_boolean(p) && is_token(p, ")") && is_stmt(p);
    if (is_token(p, "while"))
        return is_token(p, "(") && is_boolean(p) && is_token(p, ")") && is_linelist(p) && is_token(p, "endwhile");
    if (is_token(p, "read") || is_token(p, "goto") || is_token(p, "gosub"))
        return is_id(p);
    if (is_token(p, "write"))
        return is_expr(p);
    return is_token(p, "return") || is_token(p, "break") || is_token(p, "end");
}

bool is_linetail(struct Program *p) {
    return (is_token(p, ";") && is_stmt(p)) || true;
}

bool is_label(struct Program *p) {
    const int orig_pos = p->nextChar;
    if (!(is_id(p) && is_token(p, ":")))
        p->nextChar = orig_pos; // we have to reset this bc id can be in the same pos for label as for stmt and we won't know
    return true;
}

bool is_line(struct Program *p) {
    return is_label(p) && is_stmt(p) && is_linetail(p);
}

bool is_linelist(struct Program *p) {
    return (is_line(p) && is_linelist(p)) || true;
}

bool is_program(struct Program *p) {
    return is_linelist(p) && is_token(p, "$$");
}