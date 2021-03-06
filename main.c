#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include "ad.h"

//ad.c
Domain *symTable=NULL;
char *globalMemory=NULL;
int nGlobalMemory=0;

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
    string[size - 1] = '\0';
    return string;
}


//ANALIZATOR LEXICAL//
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


//ANALIZATOR SINTACTIC//

Symbol *owner = NULL;

Token *iTk; // iteratorul ??n lista de atomi. Ini??ial pointeaz?? la primul atom din list??.
Token *consumedTk; // atomul care tocmai a fost consumat. Va fi folosit ??n etapele urm??toare ale compilatorului.

bool consume(int code){
    if(iTk->code==code){// dac?? la pozi??ia curent?? avem codul cerut, consum??m atomul
        consumedTk=iTk;
        printf("%s\n",enumID[consumedTk->code]);
        iTk=iTk->next;
        return true;
    }
    return false; // dac?? la pozi??ia curent?? se afl?? un atom cu un alt cod dec??t celcerut, nu are loc nicio ac??iune
}

bool typeBase(Type *t){
    t->n=-1;
    if(consume(INT)){
        t->tb=TB_INT;
        return true;
    }
    if(consume(DOUBLE)){
        t->tb=TB_DOUBLE;
        return true;
    }
    if(consume(CHAR)){
        t->tb=TB_CHAR;
        return true;
    }
    if(consume(STRUCT)){
        if(consume(ID)){
            Token *tkName = consumedTk;
            t->tb=TB_STRUCT;
            t->s=findSymbol(tkName->text);
            if(!t->s) tkerr(iTk,"structura nedefinita: %s",tkName->text);
            return true;
        }else tkerr(iTk,"Lispa nume dupa structura");
    }
    return false;
}

bool expr();

bool exprPrimary(){
    Token *start=iTk;
    if(consume(ID)) {
        if (consume(LPAR)) {
            if (expr()) {
                for (;;) {
                    if (consume(COMMA)) {
                        if (expr()) {}
                        else break;
                    } else break;
                }
            }
            if (consume(RPAR)){
                return true;
            }else{
                tkerr(iTk,"Lipseste \")\" dupa (");
            }


        } else return true;
    }
    iTk = start;
    if(consume(CT_INT)){
        return true;
    }
    if(consume(CT_REAL)){
        return true;
    }
    if(consume(CT_CHAR)){
        return true;
    }
    if(consume(CT_STRING)){
        return true;
    }
    if(consume(LPAR)){
        if(expr()){
            if(consume(RPAR)){
                return true;
            }else{
                tkerr(iTk,"Lipseste \")\" dupa (");
            }
        }
    }
    iTk = start;
    return false;
}



bool exprPostfixPrim(){
    if(consume(LBRACKET)){
        if(expr()){
            if(consume(RBRACKET)){
                if(exprPostfixPrim())
                    return true;
            }else{
                tkerr(iTk,"Lipsete \"]\" dupa [");
            }
        }
    }
    if(consume(DOT)){
        if(consume(ID)) {
            if (exprPostfixPrim())
                return true;
        }else tkerr(iTk,"Lipsa nume variabila");
    }
    return true;
}

bool exprPostfix(){
    Token *start=iTk;
    if(exprPrimary())
        if(exprPostfixPrim())
            return true;

    iTk = start;
    return false;
}

bool arrayDecl(Type *t);

bool exprUnary(){
    Token *start=iTk;
    if(consume(SUB) ) {
        if (exprUnary()){
            return true;
        }else tkerr(iTk,"Lipsa expresie dupa -");
    }
    if( consume(NOT)) {
        if (exprUnary()){
            return true;
        }else tkerr(iTk,"Lipsa expresie dupa !");

    }
    iTk = start;
    if(exprPostfix())
            return true;
    iTk = start;
    return false;
}

bool exprCast(){
    Token *start=iTk;
    Type t;
    if(consume(LPAR)) {
        if (typeBase(&t)) {
            if (arrayDecl(&t) || !arrayDecl(&t)) {
                if (consume(RPAR)) {
                    if (exprCast())
                        return true;
                } else {
                    tkerr(iTk, "Lipseste \")\" dupa (");
                }
            }
        }
    }
    iTk = start;
    if(exprUnary())
        return true;
    iTk = start;
    return false;
}

bool exprMulPrim() {
    if (consume(MUL)) {
        if (exprCast()) {
            if (exprMulPrim())
                return true;
        }else tkerr(iTk,"Lipsa expresie dupa *");
    }
    if ( consume(DIV)) {
        if (exprCast()) {
            if (exprMulPrim())
                return true;
        }else tkerr(iTk,"Lipsa expresie dupa /");
    }
    return true;

}

bool exprMul(){
    Token *start=iTk;
    if(exprCast())
        if(exprMulPrim())
            return true;
    iTk = start;
    return false;
}

bool exprAddPrim(){
    if(consume(ADD)){
        if(exprMul()){
            if(exprAddPrim())
                return true;
        }else tkerr(iTk,"Lipsa expresie dupa +");
    }
    if( consume(SUB)){
        if(exprMul()){
            if(exprAddPrim())
                return true;
        }else tkerr(iTk,"Lipsa expresie dupa -");
    }


    return true;

}

bool exprAdd(){
    Token *start=iTk;
    if(exprMul())
        if(exprAddPrim())
            return true;
    iTk = start;
    return false;
}

bool exprRelPrim(){
    if(consume(LESS)){
        if(exprAdd()){
            if(exprRelPrim())
                return true;
        }else tkerr(iTk,"Lipsa expresie dupa <");

    }
    if(consume(LESSEQ)){
        if(exprAdd()){
            if(exprRelPrim())
                return true;
        }else tkerr(iTk,"Lipsa expresie dupa <=");
    }
    if(consume(GREATER)){
        if(exprAdd()){
            if(exprRelPrim())
                return true;
        }else tkerr(iTk,"Lipsa expresie dupa >");
    }
    if(consume(GREATEREQ)) {
        if (exprAdd()) {
            if (exprRelPrim())
                return true;
        } else tkerr(iTk, "Lipsa expresie dupa >=");
    }
    return true;

}

bool exprRel(){
    Token *start=iTk;
    if(exprAdd())
        if(exprRelPrim())
            return true;
    iTk = start;
    return false;
}

bool exprEqPrim(){
    if(consume(EQUAL)){
        if(exprRel()) {
            if (exprEqPrim())
                return true;
        }else tkerr(iTk,"Lipsa expresie dupa =");
    }
    if( consume(NOTEQ)){
        if(exprRel()) {
            if (exprEqPrim())
                return true;
        }else tkerr(iTk,"Lipsa expresie dupa !=");
    }
    return true;
}

bool exprEq(){
    Token *start=iTk;
    if(exprRel())
        if(exprEqPrim())
            return true;
    iTk = start;
    return false;
}

bool exprAndPrim(){
    if(consume(AND)) {
        if (exprEq()) {
            if (exprAndPrim()) {
                return true;
            }
        }else tkerr(iTk,"Lipsa expresie dupa &&");
    }
    return true;
}


bool exprAnd(){
    Token *start=iTk;
    if(exprEq())
        if(exprAndPrim())
            return true;
    iTk = start;
    return false;
}



bool exprOrPrim(){
    if(consume(OR)){
        if(exprAnd()){
            if(exprOrPrim()){
                return true;
            }
        }else tkerr(iTk, "Lipsa expresie dupa ||");
    }
    return true;
}

bool exprOr(){
    Token *start=iTk;
    if(exprAnd())
        if(exprOrPrim())
            return true;
    iTk = start;
    return false;
}

bool exprAssign(){
    Token *start=iTk;
    if(exprUnary()) {
        if (consume(ASSIGN)){
            if (exprAssign()){
                return true;
            }else tkerr(iTk,"Lipsa valoare dupa =");
        }
    }
    iTk = start;
    if(exprOr())
        return  true;
    iTk = start;
    return false;
}

bool expr(){
    Token *start=iTk;
    if(exprAssign())
        return true;
    iTk = start;
    return false;
}

bool arrayDecl(Type *t){
    Token *start=iTk;
    if(consume(LBRACKET)){
        //if(expr()||!expr()){
        if(consume(CT_INT)){
            Token *tkSize = consumedTk;
            t->n = tkSize->i;
        }else{
            t->n = 0;
        }
        if(consume(RBRACKET)){
            return true;
        }else{
            tkerr(iTk,"Lipsete \"]\" dupa [");
        }
    }

    iTk = start;
    return false;
}

bool fnParam(){
    Token *start=iTk;
    Type t;
    if(typeBase(&t)){
        if(consume(ID)){
            Token *tkName = consumedTk;
            if(arrayDecl(&t)){
                t.n=0;
            }

            Symbol *param=findSymbolInDomain(symTable,tkName->text);
            if(param)tkerr(iTk,"symbol redefinition: %s",tkName->text);
            param=newSymbol(tkName->text,SK_PARAM);
            param->type=t;
            param->paramIdx=symbolsLen(owner->fn.params);
            addSymbolToDomain(symTable,param);
            addSymbolToList(&owner->fn.params,dupSymbol(param));
            return true;
        }else tkerr(iTk, "Lipsa nume parametru");
    }else{
        if(consume(ID)){
            tkerr(iTk, "Lipsa tip parametru");
        }
    }
    iTk=start;
    return false;
}

bool varDef(){
    Token *start=iTk;
    Type t;
    if(typeBase(&t)) {
        if (consume(ID)) {
            Token *tkName = consumedTk;
            if (arrayDecl(&t) ) {
                if (t.n == 0)tkerr(iTk, "a vector variable must have a specified dimension");
            }
            if (consume(SEMICOLON)) {

                Symbol *var=findSymbolInDomain(symTable,tkName->text);
                if(var)tkerr(iTk,"symbol redefinition: %s",tkName->text);
                var=newSymbol(tkName->text,SK_VAR);
                var->type=t;
                var->owner=owner;
                addSymbolToDomain(symTable,var);
                if(owner){
                    switch(owner->kind){
                        case SK_FN:
                            var->varIdx=symbolsLen(owner->fn.locals);
                            addSymbolToList(&owner->fn.locals,dupSymbol(var));
                            break;
                        case SK_STRUCT:
                            var->varIdx=typeSize(&owner->type);
                            addSymbolToList(&owner->structMembers,dupSymbol(var));
                            break;
                    }
                }else{
                    var->varIdx=allocInGlobalMemory(typeSize(&t));
                }

                return true;
            } else tkerr(iTk, "Lipseste \";\"");

        } else tkerr(iTk, "Lipsa nume variabila");
    }


    iTk = start;
    return false;
}

bool structDef(){
    Token *start = iTk;
    if(consume(STRUCT)) {
        if (consume(ID)) {
            Token *tkName = consumedTk;
            if (consume(LACC)) {

                Symbol *s=findSymbolInDomain(symTable, tkName->text);
                if(s)tkerr(iTk,"symbol redefinition: %s",tkName->text);
                s=addSymbolToDomain(symTable,newSymbol(tkName->text,SK_STRUCT));
                s->type.tb=TB_STRUCT;
                s->type.s=s;
                s->type.n=-1;
                pushDomain();
                owner=s;

                for (;;) {
                    if (varDef()) {}
                    else break;
                }
                if (consume(RACC)) {
                    if (consume(SEMICOLON)) {
                        owner=NULL;
                        dropDomain();
                        return true;
                    } else tkerr(iTk, "Lipseste \";\" dupa }");

                } else tkerr(iTk, "Lipseste \"}\" dupa {");

            }
        }
    }
    iTk = start;
    return false;
}

bool stmCompound(bool newDomain);

bool stm(){
    Token *start = iTk;
    if(stmCompound(true)){
        return true;
    }
    iTk = start;
    if(consume(IF)) {
        if (consume(LPAR)){
            if (expr()){
                if (consume(RPAR)){
                    if (stm()) {
                        if (consume(ELSE)) {
                            if (stm()){
                                return true;
                            }else tkerr(iTk,"Lipseste instructiune dupa else");

                        } else return true;
                    }else tkerr(iTk,"Lipsa instructiune/set de instructiuni dupa if");
                }else tkerr(iTk,"Lipseste \")\" dupa if");
            }else tkerr(iTk,"Lipsa expresie la if");
        }else tkerr(iTk,"Lipseste \"(\" dupa if");
    }
    iTk = start;
    if(consume(WHILE)){
        if(consume(LPAR)){
            if(expr()){
                if(consume(RPAR)){
                    if(stm()){
                        return true;
                    }else tkerr(iTk,"Lipsa instructiune/set de instructiuni dupa while");
                }else tkerr(iTk,"Lipseste \")\" dupa while");
            }else tkerr(iTk,"Lipsa expresie la while");
        }else tkerr(iTk,"Lipseste \"(\" dupa while");
    }
    iTk = start;
    if(consume(FOR)){
        if(consume(LPAR)){
            if(expr() || !expr()){
                if(consume(SEMICOLON)){
                    if(expr() || !expr()){
                        if(consume(SEMICOLON)){
                            if(expr() || !expr()){
                                if(consume(RPAR)){
                                    if(stm()){
                                        return true;
                                    }else tkerr(iTk,"Lipsa instructiune/set de instructiuni dupa for");
                                }else tkerr(iTk,"Lipseste \")\" dupa for");
                            }
                        }else tkerr(iTk,"Lipseste \";\" dupa expresie in for");
                    }
                }else tkerr(iTk,"Lipseste \";\" dupa expresie in for");
            }
        }else tkerr(iTk,"Lipseste \"(\" dupa for");
    }
    iTk = start;
    if(consume(BREAK)){
        if(consume(SEMICOLON)){
            return true;
        }else{
            tkerr(iTk,"Lipseste \";\" dupa break");
        }
    }
    iTk = start;
    if(consume(RETURN)){
        if(expr() || !expr()){
            if(consume(SEMICOLON)){
                return true;
            }else{
                tkerr(iTk,"Lipseste \";\" dupa return");
            }
        }

    }
    iTk = start;
    if(expr() ){
        if(consume(SEMICOLON)){
            return true;
        }else tkerr(iTk, "Lipseste \";\" dupa expresie");
    }

    if(!expr()){
        if(consume(SEMICOLON)){
            return true;
        }
    }

    iTk = start;
    return false;
}

bool stmCompound(bool newDomain) {
    Token *start = iTk;
    if (consume(LACC)) {
        if(newDomain)pushDomain();
        for (;;) {
            if (varDef()) {}
            else if (stm()) {}
            else break;
        }
        if (consume(RACC)) {
            if(newDomain)dropDomain();
            return true;
        } else tkerr(iTk, "Lipseste \"}\"");
    }

    iTk = start;
    return false;
}

bool fnDef() {
    Token *start = iTk;
    Type t;
    if (typeBase(&t) || consume(VOID)) {
        if (consumedTk->code == VOID) {
            t.tb = TB_VOID;
        }
        if (consume(ID)) {
            Token *tkName = consumedTk;
            if (consume(LPAR)) {

                Symbol *fn=findSymbolInDomain(symTable,tkName->text);
                if(fn)tkerr(iTk,"symbol redefinition: %s",tkName->text);
                fn=newSymbol(tkName->text,SK_FN);
                fn->type=t;
                addSymbolToDomain(symTable,fn);
                owner=fn;
                pushDomain();
                if (fnParam()) {
                    for (;;) {
                        if (consume(COMMA)) {
                            if (fnParam()) {}
                            else {
                                tkerr(iTk, "Lipseste tipul dupa \",\"");
                            }
                        } else break;
                    }
                    if (consume(RPAR)) {
                        if (stmCompound(false)) {
                            dropDomain();
                            owner=NULL;
                            return true;
                        }
                    } else {
                        tkerr(iTk, "Lipseste \")\" dupa (");
                    }
                }
                if (consume(RPAR)) {
                    if (stmCompound(false)) {
                        dropDomain();
                        owner=NULL;
                        return true;
                    }
                } else tkerr(iTk, "Lipseste \")\" dupa (");
            }
        }
    }

    iTk = start;
    return false;
}

bool unit(){
    for(;;){ // bucl?? infinit??
        if(structDef()){}
        else if(fnDef()){}
        else if(varDef()){}
        else break; // dac?? nu se poate consuma nimic la itera??ia curent??, seiese din bucl??
    }
    if(consume(END)){// se ajunge la END chiar ??i dac?? nu se consum?? nimic ??nrepeti??ie, deci ea este op??ional??
        return true;
    }
}


// Analizator de domeniu

//ad.c


int typeBaseSize(Type *t){
    switch(t->tb){
        case TB_INT:return sizeof(int);
        case TB_DOUBLE:return sizeof(double);
        case TB_CHAR:return sizeof(char);
        case TB_VOID:return 0;
        default:{		// TB_STRUCT
            int size=0;
            for(Symbol *m=t->s->structMembers;m;m=m->next){
                size+=typeSize(&m->type);
            }
            return size;
        }
    }
}

int typeSize(Type *t){
    if(t->n<0)return typeBaseSize(t);
    if(t->n==0)return sizeof(void*);
    return t->n*typeBaseSize(t);
}

// elibereaza memoria ocupata de o lista de simboluri
void freeSymbols(Symbol *list){
    for(Symbol *next;list;list=next){
        next=list->next;
        freeSymbol(list);
    }
}

Symbol *newSymbol(const char *name,SymKind kind){
    Symbol *s;
    SAFEALLOC(s,Symbol)
    // seteaza pe 0/NULL toate campurile
    memset(s,0,sizeof(Symbol));
    s->name=name;
    s->kind=kind;
    return s;
}

Symbol *dupSymbol(Symbol *symbol){
    Symbol *s;
    SAFEALLOC(s,Symbol)
    *s=*symbol;
    s->next=NULL;
    return s;
}

// adauga s la sfarsitul listei list
// s->next este NULL de la newSymbol
Symbol *addSymbolToList(Symbol **list,Symbol *s){
    Symbol *iter=*list;
    if(iter){
        while(iter->next)iter=iter->next;
        iter->next=s;
    }else{
        *list=s;
    }
    return s;
}

int symbolsLen(Symbol *list){
    int n=0;
    for(;list;list=list->next)n++;
    return n;
}

void freeSymbol(Symbol *s){
    switch(s->kind){
        case SK_FN:
            freeSymbols(s->fn.params);
            freeSymbols(s->fn.locals);
            break;
        case SK_STRUCT:
            freeSymbols(s->structMembers);
            break;
    }
    free(s);
}

Domain *pushDomain(){
    Domain *d;
    SAFEALLOC(d,Domain)
    d->symbols=NULL;
    d->parent=symTable;
    symTable=d;
    return d;
}

void dropDomain(){
    Domain *d=symTable;
    symTable=d->parent;
    freeSymbols(d->symbols);
    free(d);
}

void showNamedType(Type *t,const char *name){
    switch(t->tb){
        case TB_INT:printf("int");break;
        case TB_DOUBLE:printf("double");break;
        case TB_CHAR:printf("char");break;
        case TB_VOID:printf("void");break;
        default:		// TB_STRUCT
            printf("struct %s",t->s->name);
    }
    if(name)printf(" %s",name);
    if(t->n==0)printf("[]");
    else if(t->n>0)printf("[%d]",t->n);
}

void showSymbol(Symbol *s){
    switch(s->kind){
        case SK_VAR:
            showNamedType(&s->type,s->name);
            printf(";\t// size=%d, idx=%d\n",typeSize(&s->type),s->varIdx);
            break;
        case SK_PARAM:{
            showNamedType(&s->type,s->name);
            printf(" /*size=%d, idx=%d*/",typeSize(&s->type),s->paramIdx);
        }break;
        case SK_FN:{
            showNamedType(&s->type,s->name);
            printf("(");
            bool next=false;
            for(Symbol *param=s->fn.params;param;param=param->next){
                if(next)printf(", ");
                showSymbol(param);
                next=true;
            }
            printf("){\n");
            for(Symbol *local=s->fn.locals;local;local=local->next){
                printf("\t");
                showSymbol(local);
            }
            printf("\t}\n");
        }break;
        case SK_STRUCT:{
            printf("struct %s{\n",s->name);
            for(Symbol *m=s->structMembers;m;m=m->next){
                printf("\t");
                showSymbol(m);
            }
            printf("\t};\t// size=%d\n",typeSize(&s->type));
        }break;
    }
}

void showDomain(Domain *d,const char *name){
    printf("// domain: %s\n",name);
    for(Symbol *s=d->symbols;s;s=s->next){
        showSymbol(s);
    }
    puts("\n");
}

Symbol *findSymbolInDomain(Domain *d,const char *name){
    for(Symbol *s=d->symbols;s;s=s->next){
        if(!strcmp(s->name,name))return s;
    }
    return NULL;
}

Symbol *findSymbol(const char *name){
    for(Domain *d=symTable;d;d=d->parent){
        Symbol *s=findSymbolInDomain(d,name);
        if(s)return s;
    }
    return NULL;
}

Symbol *addSymbolToDomain(Domain *d,Symbol *s){
    return addSymbolToList(&d->symbols,s);
}

int allocInGlobalMemory(int nBytes){
    char *aux=(char*)realloc(globalMemory,nGlobalMemory+nBytes);
    if(!aux)err("not enough memory");
    int idx=nGlobalMemory;
    nGlobalMemory+=nBytes;
    return idx;
}


int main() {
    int f;
    if((f = open("testsin.txt", O_RDONLY)) < 0){
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
    buff[strlen(buff)] = '\0';
    close(f);
/*
    FILE* f;
    if((f = fopen("C:\\Users\\nicu_\\OneDrive\\Desktop\\LFTC\\testAD.c","r")) == NULL){
    //if((f = fopen("C:\\Users\\nicu_\\Desktop\\LFTC\\testsin.txt","r")) == NULL){
        printf("eroare la deschidere fisier");
        exit(EXIT_FAILURE);
    }
    char buff[30001];
    fread(buff,1,30001, f);
    fclose(f);
    */
    pCrtCh = buff;

    while(flag) {
        getNextToken();
    }


    showAtoms(tokens);
    printf("\n");

    pushDomain();

    iTk = tokens;
    unit();

    showDomain(symTable,"global");
    dropDomain();

    terminare(tokens);

    return 0;
}
