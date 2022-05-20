#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SAFEALLOC(var,Type) if((var=(Type*)malloc(sizeof(Type)))==NULL)err("not enough memory");

enum{
    ID,BREAK,CHAR,DOUBLE,ELSE,FOR,IF,INT,RETURN,STRUCT,VOID,WHILE,
    CT_INT,CT_REAL,CT_CHAR,CT_STRING,
    COMMA,SEMICOLON,LPAR,RPAR,LBRACKET,RBRACKET,LACC,RACC,END,
    ADD,SUB,MUL,DIV,DOT,AND,OR,NOT,ASSIGN,EQUAL,NOTEQ,LESS,LESSEQ,GREATER,GREATEREQ
};

const char *enumID[] = {
        "ID","BREAK","CHAR","DOUBLE","ELSE","FOR","IF","INT","RETURN","STRUCT","VOID","WHILE",
        "CT_INT","CT_REAL","CT_CHAR","CT_STRING",
        "COMMA","SEMICOLON","LPAR","RPAR","LBRACKET","RBRACKET","LACC","RACC","END",
        "ADD","SUB","MUL","DIV","DOT","AND","OR","NOT","ASSIGN","EQUAL","NOTEQ","LESS","LESSEQ","GREATER","GREATEREQ"
};

int line = 1,flag = 1;

typedef struct _Token{
    int code; // codul (numele)
    union{
        char *text; // folosit pentru ID, CT_STRING (alocat dinamic)
        int i; // folosit pentru CT_INT, CT_CHAR
        double r; // folosit pentru CT_REAL
    };
    int line; // linia din fisierul de intrare
    struct _Token *next; // inlantuire la urmatorul AL
}Token;

Token *lastToken = NULL,*tokens = NULL;

void err(const char *fmt,...)
{
    va_list va;
    va_start(va,fmt);
    fprintf(stderr,"error: ");
    vfprintf(stderr,fmt,va);
    fputc('\n',stderr);
    va_end(va);
    exit(-1);
}

void tkerr(const Token *tk,const char *fmt,...)
{
    va_list va;
    va_start(va,fmt);
    fprintf(stderr,"error in line %d: ",tk->line);
    vfprintf(stderr,fmt,va);
    fputc('\n',stderr);
    va_end(va);
    exit(-1);
}

Token *addTk(int code)
{
    Token *tk;
    SAFEALLOC(tk,Token)
    tk->code=code;
    tk->line=line;
    tk->next=NULL;
    if(lastToken){
        lastToken->next=tk;
    }else{
        tokens=tk;
    }
    lastToken=tk;
    return tk;
}

char *createString(const char *pStartCh,const char *pCrtCh){
    char *string;
    int size;
    size = pCrtCh - pStartCh;
    size++;
    string = malloc(size * sizeof (char));
    strncpy(string, pStartCh, (size-1));
    string[size] = '\0';
    return string;
}
char *pCrtCh;
int getNextToken() {
    int state = 0, nCh;
    char ch;
    const char *pStartCh;
    Token *tk;
    while(1) { // bucla infinita
        ch = *pCrtCh;
        switch (state)
        {
            case 0:
            {
                if(ch == '_'|| isalpha(ch)){
                    pStartCh=pCrtCh; // memoreaza inceputul ID-ului
                    pCrtCh++;
                    state = 1;
                }else if(isdigit(ch)){
                    pStartCh=pCrtCh;
                    pCrtCh++;
                    state = 3;
                }else if(ch == '\''){
                    pCrtCh++;
                    state = 11;
                }else if(ch == '\"'){
                    pStartCh = pCrtCh;
                    pCrtCh++;
                    state = 14;
                }else if(ch == ','){
                    pCrtCh++;
                    state = 16;
                }else if(ch == ';'){
                    pCrtCh++;
                    state = 17;
                }else if(ch == '('){
                    pCrtCh++;
                    state = 18;
                }else if(ch == ')'){
                    pCrtCh++;
                    state = 19;
                }else if(ch == '['){
                    pCrtCh++;
                    state = 20;
                }else if(ch == ']'){
                    pCrtCh++;
                    state = 21;
                }else if(ch == '{'){
                    pCrtCh++;
                    state = 22;
                }else if(ch == '}'){
                    pCrtCh++;
                    state = 23;
                }else if(ch == '\0'){
                    pCrtCh++;
                    state = 24;
                }else if(ch == '+'){
                    pCrtCh++;
                    state = 25;
                }else if(ch == '-'){
                    pCrtCh++;
                    state = 26;
                }else if(ch == '*'){
                    pCrtCh++;
                    state = 27;
                }else if(ch == '.'){
                    pCrtCh++;
                    state = 28;
                }else if(ch == '&'){
                    pCrtCh++;
                    state = 29;
                }else if(ch == '|'){
                    pCrtCh++;
                    state = 31;
                }else if(ch == '!'){
                    pCrtCh++;
                    state = 33;
                }else if(ch == '='){
                    pCrtCh++;
                    state = 36;
                }else if(ch == '<'){
                    pCrtCh++;
                    state = 39;
                }else if(ch == '>'){
                    pCrtCh++;
                    state = 42;
                }else if(ch == '/'){
                    pCrtCh++;
                    state = 45;
                }else if( ch == '\r' || ch == '\t' || ch == ' '){
                    pCrtCh++;
                }else if(ch == '\n'){
                    line++;
                    pCrtCh++;
                }
                break;
            }
            case 1:
            {
                if(ch == '_' || isalnum(ch)){
                    pCrtCh++;
                }else{
                    state = 2;
                }
                break;
            }
            case 2:
            {
                nCh=pCrtCh-pStartCh;
                //BREAK,CHAR,DOUBLE,ELSE,FOR,IF,INT,RETURN,STRUCT,VOID,WHILE
                if(nCh==5&&!memcmp(pStartCh,"break",5))tk=addTk(BREAK);
                else if(nCh==4&&!memcmp(pStartCh,"char",4))tk=addTk(CHAR);
                else if(nCh==6&&!memcmp(pStartCh,"double",6))tk=addTk(DOUBLE);
                else if(nCh==4&&!memcmp(pStartCh,"else",4))tk=addTk(ELSE);
                else if(nCh==3&&!memcmp(pStartCh,"for",3))tk=addTk(FOR);
                else if(nCh==2&&!memcmp(pStartCh,"if",2))tk=addTk(IF);
                else if(nCh==3&&!memcmp(pStartCh,"int",3))tk=addTk(INT);
                else if(nCh==6&&!memcmp(pStartCh,"return",6))tk=addTk(RETURN);
                else if(nCh==6&&!memcmp(pStartCh,"struct",6))tk=addTk(STRUCT);
                else if(nCh==4&&!memcmp(pStartCh,"void",4))tk=addTk(VOID);
                else if(nCh==5&&!memcmp(pStartCh,"while",5))tk=addTk(WHILE);
                else{
                    tk=addTk(ID);
                    tk->text=createString(pStartCh,pCrtCh);
                }
                return tk->code;
            }
            case 3:{
                if(isdigit(ch)){
                    pCrtCh++;

                    state = 3;
                }else if(ch == '.'){
                    pCrtCh++;
                    state = 5;
                }else if(ch == 'e' || ch == 'E'){
                    pCrtCh++;
                    state = 7;
                }else{
                    state = 4;
                }
                break;
            }
            case 4:{
                tk = addTk(CT_INT);
                char *value = createString(pStartCh,pCrtCh);
                tk -> i = atoi(value);
                free(value);
                return tk->code;
            }
            case 5:{
                if(isdigit(ch)){
                    pCrtCh++;
                    state = 6;
                }
                break;
            }
            case 6:{
                if(isdigit(ch)){
                    pCrtCh++;
                }else if(ch == 'e' || ch == 'E'){
                    pCrtCh++;
                    state = 7;
                }else{
                    state = 10;
                }
                break;
            }
            case 7:{
                if(ch == '+' || ch == '-'){
                    pCrtCh++;
                    state = 8;
                }else{
                    state = 8;
                }
                break;
            }
            case 8:{
                if(isdigit(ch)){
                    pCrtCh++;
                    state = 9;
                }
                break;
            }
            case 9:{
                if(isdigit(ch)){
                    pCrtCh++;
                }else{
                    state = 10;
                }
                break;
            }
            case 10:{
                tk = addTk(CT_REAL);
                char *value = createString(pStartCh,pCrtCh);
                tk -> r = atof(value);
                free(value);
                return tk->code;
            }
            case 11:{
                if(ch != '\''){
                    pStartCh = pCrtCh;
                    pCrtCh++;
                    state = 12;
                }
                break;
            }
            case 12:{
                if(ch == '\''){
                    pCrtCh++;
                    state = 13;
                }
                break;
            }
            case 13:{
                tk = addTk(CT_CHAR);
                unsigned char c = *pStartCh;
                tk -> i = c;
                return tk->code;
            }
            case 14:{
                if(ch != '\"'){
                    pCrtCh++;
                }else{
                    state = 15;
                }
                break;
            }
            case 15:{
                tk = addTk(CT_STRING);
                tk->text=createString(pStartCh,pCrtCh + 1);
                pCrtCh++;
                return tk->code;
            }
            case 16:{
                tk = addTk(COMMA);
                return tk->code;
            }
            case 17:{
                tk = addTk(SEMICOLON);
                return tk->code;
            }
            case 18:{
                tk = addTk(LPAR);
                return tk->code;
            }
            case 19:{
                tk = addTk(RPAR);
                return tk->code;
            }
            case 20:{
                tk = addTk(LBRACKET);
                return tk->code;
            }
            case 21:{
                tk = addTk(RBRACKET);
                return tk->code;
            }
            case 22:{
                tk = addTk(LACC);
                return tk->code;
            }
            case 23:{
                tk = addTk(RACC);
                return tk->code;
            }
            case 24:{
                tk = addTk(END);
                flag = 0;
                return tk->code;
            }
            case 25:{
                tk = addTk(ADD);
                return tk->code;
            }
            case 26:{
                tk = addTk(SUB);
                return tk->code;
            }
            case 27:{
                tk = addTk(MUL);
                return tk->code;
            }
            case 28:{
                tk = addTk(DOT);
                return tk->code;
            }
            case 29:{
                if(ch == '&'){
                    pCrtCh++;
                    state = 30;
                }
                break;
            }
            case 30:{
                tk = addTk(AND);
                return tk->code;
            }
            case 31:{
                if(ch == '|'){
                    pCrtCh++;
                    state = 32;
                }
                break;
            }
            case 32:{
                tk = addTk(OR);
                return tk->code;
            }
            case 33:{
                if(ch == '='){
                    pCrtCh++;
                    state = 35;
                }else{
                    state = 34;
                }
                break;
            }
            case 34:{
                tk = addTk(NOT);
                return tk->code;
            }
            case 35:{
                tk = addTk(NOTEQ);
                return tk->code;
            }
            case 36:{
                if(ch == '='){
                    pCrtCh++;
                    state = 38;
                }else{
                    state = 37;
                }
                break;
            }
            case 37:{
                tk = addTk(ASSIGN);
                return tk->code;
            }
            case 38:{
                tk = addTk(EQUAL);
                return tk->code;
            }
            case 39:{
                if(ch == '='){
                    pCrtCh++;
                    state = 41;
                }else{
                    state = 40;
                }
                break;
            }
            case 40:{
                tk = addTk(LESS);
                return tk->code;
            }
            case 41:{
                tk = addTk(LESSEQ);
                return tk->code;
            }
            case 42:{
                if(ch == '='){
                    pCrtCh++;
                    state = 44;
                }else{
                    state = 43;
                }
                break;
            }
            case 43:{
                tk = addTk(GREATER);
                return tk->code;
            }
            case 44:{
                tk = addTk(GREATEREQ);
                return tk->code;
            }
            case 45:{
                if(ch == '/'){
                    pCrtCh++;
                    state = 47;
                }else{
                    state = 46;
                }
                break;
            }
            case 46:{
                tk = addTk(DIV);
                return tk->code;
            }
            case 47:{
                if(ch != '\n' ){    //|| ch != '\t' || ch != '\r'){
                    pCrtCh++;
                }else{
                    state = 0;
                }
                break;
            }
        }
    }
}
int linie = 0;
void showAtoms(Token *tk){
    if(linie == tk -> line){
        printf( " %s", enumID[tk->code]);
    }else{
        linie = tk->line;
        printf("\n%d: %s", tk->line, enumID[tk->code]);
    }
    switch (tk->code) {
        case CT_STRING:{
            printf(":%s", tk->text);
            break;
        }
        case CT_INT:{
            printf(":%d", tk->i);
            break;
        }
        case CT_REAL:{
            printf(":%lf", tk->r);
            break;
        }
        case CT_CHAR:{
            printf(":%c", tk->i);
            break;
        }
        case ID:{
            printf(":%s", tk->text);
            break;
        }
    }
    if(tk->next != NULL){
        showAtoms(tk->next);
    }
}

void terminare(Token *token) {
    Token *tk;
    tk = token->next;
    if(token->code == ID || token->code == CT_STRING){
        free(token->text);
    }
    free(token);
    if (tk != NULL) {
        terminare(tk);
    }
}

int main() {
    int f;
    if((f = open("test.txt", O_RDONLY)) < 0){
        printf("Eroare la deschidere fisier citire");
        exit(EXIT_FAILURE);
    }
    char buff[30001];
    int code;
    if(read(f, buff, 30000) < 0){
        printf("Read error");
        close(f);
        exit(EXIT_FAILURE);
    }

    close(f);
    //buff[strlen(buff)] = '\0';
    pCrtCh = buff;

    while(flag) {
        getNextToken();
    }

    showAtoms(tokens);
    terminare(tokens);

    return 0;
}
