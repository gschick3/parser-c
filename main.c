#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 255
#define MAX_PROG_LENGTH 2048

struct Program {
    char buff[MAX_PROG_LENGTH];
    int len;
    int next_char;
};

enum TokenType {END, NUMBER, SYMBOL, IDENTIFIER};

struct Token {
    enum TokenType type;
    char buff[MAX_LINE_LENGTH];
    int len;
};

enum ProdType {PROGRAM, LINELIST, LINE, LINETAIL, STMT, EXPR, ETAIL, BOOLEAN, LABEL, NUM, BOOL_OP, NUMSIGN, DIGITS, ID, TOKEN};
const char prod_strings[][10] = {"PROGRAM", "LINELIST", "LINE", "LINETAIL", "STMT", "EXPR", "ETAIL",
    "BOOLEAN", "LABEL", "NUM", "BOOL_OP", "NUMSIGN", "DIGITS", "ID", "TOKEN"};

struct TreeNode {
    enum ProdType type;
    char data[MAX_LINE_LENGTH];
    int data_len;
    struct TreeNode* children[6];
    int num_children;
};

int parse(char[100]);
void print_tree(const struct TreeNode*, int);
void destroy_tree(struct TreeNode*);

void move_char(struct Token *dest, struct Program *src);
struct Token next_token(struct Program*);
struct TreeNode* token_to_tree(const struct Token*, enum ProdType);

bool is_token(struct Program*, struct TreeNode*, const char*);
bool is_digits(struct Program*, struct TreeNode*);
bool is_numsign(struct Program*, struct TreeNode*);
bool is_id(struct Program*, struct TreeNode*);
bool is_boolop(struct Program*, struct TreeNode*);
bool is_num(struct Program*, struct TreeNode*);
bool is_etail(struct Program*, struct TreeNode*);
bool is_expr(struct Program*, struct TreeNode*);
bool is_boolean(struct Program*, struct TreeNode*);
bool is_stmt(struct Program*, struct TreeNode*);
bool is_linetail(struct Program*, struct TreeNode*);
bool is_label(struct Program*, struct TreeNode*);
bool is_line(struct Program*, struct TreeNode*);
bool is_linelist(struct Program*, struct TreeNode*);
bool is_program(struct Program*, struct TreeNode*);

int main() {
    const int num_files = 5;
    for (int i = 1; i <= num_files; i++) {
        char filename[100];
        snprintf(filename, 100, "../test_files/test%d.txt", i);
        printf("test%d.txt: ", i);
        parse(filename);
    }

    return 0;
}

int parse(char filename[100]) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("File does not exist: %s\n", filename);
        return 1;
    }

    // Read file character by character
    char c;
    struct Program program = {"", 0, 0};
    while((c = fgetc(fp)) != EOF && program.len < MAX_PROG_LENGTH)
        program.buff[program.len++] = c;
    fclose(fp);

    // program_tree will capture the parse tree as it is built
    struct TreeNode *program_tree = malloc(sizeof(struct TreeNode));
    if (is_program(&program, program_tree)) {
        printf("Success.\n");
        print_tree(program_tree, 0);
    }
    else
        printf("Error on token: '%s'\n", next_token(&program).buff);

    destroy_tree(program_tree);
    return 0;
}

void print_tree(const struct TreeNode* tree_node, const int level) {
    for (int i = 0; i < level; i++)
        printf("-   "); // indent based on level of parse tree
    printf("%s %.*s\n", prod_strings[tree_node->type], tree_node->data_len, tree_node->data);
    for (int i = 0; i < tree_node->num_children; i++)
        print_tree(tree_node->children[i], level+1);
}

void destroy_tree(struct TreeNode* tree_node) {
    for (int i = 0; i < tree_node->num_children; i++)
        destroy_tree(tree_node->children[i]);

    free(tree_node);
}

// Move a single character from the source code buffer to the token buffer
void move_char(struct Token *dest, struct Program *src) {
    dest->buff[dest->len++] = src->buff[src->next_char++];
}

// Scan for the next token in the source code stream
struct Token next_token(struct Program *p) {
    struct Token t = {END, "", 0};

    // Skip whitespace
    while (p->next_char <= p->len && isspace(p->buff[p->next_char]))
        p->next_char++;

    if (p->next_char > p->len)
        return t;

    if (isalpha(p->buff[p->next_char])) {
        t.type = IDENTIFIER;
        while (p->next_char <= p->len && isalnum(p->buff[p->next_char]))
            move_char(&t, p);
    } else if (isdigit(p->buff[p->next_char])) {
        t.type = NUMBER;
        while (p->next_char <= p->len && isdigit(p->buff[p->next_char]))
            move_char(&t, p);
    } else if (p->buff[p->next_char] == '(' || p->buff[p->next_char] == ')') {
        t.type = SYMBOL;
        move_char(&t, p);
    } else {
        t.type = SYMBOL;
        while (p->next_char <= p->len && !isalnum(p->buff[p->next_char]) && !isspace(p->buff[p->next_char]))
            move_char(&t, p);
    }
    return t;
}

// Create an emtpy tree node
struct TreeNode* new_tree_node(const enum ProdType type) {
    struct TreeNode *new_node = malloc(sizeof(struct TreeNode));
    new_node->num_children = 0;
    new_node->data_len = 0;
    new_node->type = type;
    return new_node;
}

// Move all data from terminal into new tree node data
struct TreeNode* token_to_tree(const struct Token* token, const enum ProdType type) {
    struct TreeNode *new_node = new_tree_node(type);
    strncpy(new_node->data, token->buff, token->len);
    new_node->data_len = token->len;

    return new_node;
}

bool is_token(struct Program *p, struct TreeNode *t, const char* token_name) {
    const struct Token next = next_token(p);
    if (!strncmp(next.buff, token_name, next.len + 1)) {
        // strncmp returns 0 if match, 1 or -1 otherwise
        t->children[t->num_children++] = token_to_tree(&next, TOKEN);
        return true;
    }

    p->next_char -= next.len;
    return false;
}

bool is_digits(struct Program *p, struct TreeNode *t) {
    const struct Token next = next_token(p); // This will remove tested tokens
    // if successful, no need to do anything
    if (next.type == NUMBER) {
        t->children[t->num_children++] = token_to_tree(&next, DIGITS);
        return true;
    }

    // otherwise, change state to failure and add tokens back
    p->next_char -= next.len;
    return false;
}

bool is_numsign(struct Program *p, struct TreeNode *t) {
    struct TreeNode *new_node = new_tree_node(NUMSIGN);
    if (is_token(p, new_node, "+") || is_token(p, new_node, "-"))
        t->children[t->num_children++] = new_node;
    else destroy_tree(new_node);

    return true; // numsign can be empty, so always return true
}

bool is_id(struct Program *p, struct TreeNode *t) {
    const struct Token next = next_token(p);

    // Make sure token is not reserved word
    const char reserved[10][10] = {"if", "while", "read", "write", "goto", "gosub", "return", "break", "end", "endwhile"};
    for (int i = 0; i < 10; i++) {
        if (!strncmp(next.buff, reserved[i], next.len + 1)) {
            p->next_char -= next.len;
            return false;
        }
    }

    if (next.type == IDENTIFIER) {
        t->children[t->num_children++] = token_to_tree(&next, ID);
        return true;
    }

    p->next_char -= next.len;
    return false;
}

bool is_boolop(struct Program *p, struct TreeNode *t) {
    struct TreeNode *new_node = new_tree_node(BOOL_OP);

    if (is_token(p, new_node, ">")
        || is_token(p, new_node, "<")
        || is_token(p, new_node, ">=")
        || is_token(p, new_node, "<=")
        || is_token(p, new_node, "<>")
        || is_token(p, new_node, "=")) {
        t->children[t->num_children++] = new_node;
        return true;
    }

    destroy_tree(new_node);
    return false;
}

bool is_num(struct Program *p, struct TreeNode *t) {
    struct TreeNode *new_node = new_tree_node(NUM);

    if (is_numsign(p, new_node) && is_digits(p, new_node)) {
        t->children[t->num_children++] = new_node;
        return true;
    }

    destroy_tree(new_node);
    return false;
}

bool is_etail(struct Program *p, struct TreeNode *t) {
    struct TreeNode *new_node = new_tree_node(ETAIL);

    if ((is_token(p, new_node, "+") || is_token(p, new_node, "-")
        || is_token(p, new_node, "*") || is_token(p, new_node, "/"))
        && is_expr(p, new_node))
        t->children[t->num_children++] = new_node;
    else destroy_tree(new_node);

    return true; // etail will always evaluate to true
}

bool is_expr(struct Program *p, struct TreeNode *t) {
    struct TreeNode *new_node = new_tree_node(EXPR);
    if ((is_id(p, new_node) || is_num(p, new_node)) && is_etail(p, new_node)) {
        t->children[t->num_children++] = new_node;
        return true;
    }
    new_node->num_children = 0; // reset children if above fails

    if (is_token(p, new_node, "(")) {
        if (is_expr(p, new_node) && is_token(p, new_node, ")")) {
            t->children[t->num_children++] = new_node;
            return true;
        }
        destroy_tree(new_node);
        return false;
    }

    destroy_tree(new_node);
    return false;
}

bool is_boolean(struct Program *p, struct TreeNode *t) {
    struct TreeNode *new_node = new_tree_node(BOOLEAN);
    if (is_token(p, new_node, "true") || is_token(p, new_node, "false")) {
        t->children[t->num_children++] = new_node;
        return true;
    }
    new_node->num_children = 0;
    if (is_expr(p, new_node)) {
        if (is_boolop(p, new_node) && is_expr(p, new_node)) {
            t->children[t->num_children++] = new_node;
            return true;
        }
        destroy_tree(new_node);
        return false;
    }

    destroy_tree(new_node);
    return false;
}

bool is_stmt(struct Program *p, struct TreeNode *t) {
    struct TreeNode *new_node = new_tree_node(STMT);
    const int orig_pos = p->next_char;
    if (is_id(p, new_node)) {
        if (is_token(p, new_node, "=") && is_expr(p, new_node)) {
            t->children[t->num_children++] = new_node;
            return true;
        }
        p->next_char = orig_pos; // we have to reset this bc id can be in the same pos for label as for stmt and we won't know
        destroy_tree(new_node);
        return false;
    }

    new_node->num_children = 0;
    if (is_token(p, new_node, "if")) {
        if (is_token(p, new_node, "(") && is_boolean(p, new_node)
        && is_token(p, new_node, ")") && is_stmt(p, new_node)) {
            t->children[t->num_children++] = new_node;
            return true;
        }
        destroy_tree(new_node);
        return false;
    }

    new_node->num_children = 0;
    if (is_token(p, new_node, "while")) {
        if (is_token(p, new_node, "(") && is_boolean(p, new_node) && is_token(p, new_node, ")")
            && is_linelist(p, new_node) && is_token(p, new_node, "endwhile")) {
            t->children[t->num_children++] = new_node;
            return true;
        }
        destroy_tree(new_node);
        return false;
    }

    new_node->num_children = 0;
    if (is_token(p, new_node, "read") || is_token(p, new_node, "goto") || is_token(p, new_node, "gosub")) {
        if (is_id(p, new_node)) {
            t->children[t->num_children++] = new_node;
            return true;
        }
        destroy_tree(new_node);
        return false;
    }

    new_node->num_children = 0;
    if (is_token(p, new_node, "write")) {
        if (is_expr(p, new_node)) {
            t->children[t->num_children++] = new_node;
            return true;
        }
        destroy_tree(new_node);
        return false;
    }

    new_node->num_children = 0;
    if (is_token(p, new_node, "return") || is_token(p, new_node, "break") || is_token(p, new_node, "end")) {
        t->children[t->num_children++] = new_node;
        return true;
    }

    destroy_tree(new_node);
    return false;
}

bool is_linetail(struct Program *p, struct TreeNode *t) {
    struct TreeNode *new_node = new_tree_node(LINETAIL);
    if (is_token(p, new_node, ";") && is_stmt(p, new_node))
        t->children[t->num_children++] = new_node;
    else destroy_tree(new_node);

    return true;
}

bool is_label(struct Program *p, struct TreeNode *t) {
    struct TreeNode *new_node = new_tree_node(LABEL);
    const int orig_pos = p->next_char;
    if (is_id(p, new_node) && is_token(p, new_node, ":"))
        t->children[t->num_children++] = new_node;
    else {
        p->next_char = orig_pos; // we have to reset this bc id can be in the same pos for label as for stmt and we won't know
        destroy_tree(new_node);
    }
    return true;
}

bool is_line(struct Program *p, struct TreeNode *t) {
    struct TreeNode *new_node = new_tree_node(LINE);
    if (is_label(p, new_node) && is_stmt(p, new_node) && is_linetail(p, new_node)) {
        t->children[t->num_children++] = new_node;
        return true;
    }

    destroy_tree(new_node);
    return false;
}

bool is_linelist(struct Program *p, struct TreeNode *t) {
    struct TreeNode *new_node = new_tree_node(LINELIST);
    if (is_line(p, new_node) && is_linelist(p, new_node))
        t->children[t->num_children++] = new_node;
    else destroy_tree(new_node);

    return true;
}

bool is_program(struct Program *p, struct TreeNode *t) {
    if (t == NULL) return false;
    t->type = PROGRAM;
    t->num_children = 0;
    t->data_len = 0;
    return is_linelist(p, t) && is_token(p, t, "$$");
}