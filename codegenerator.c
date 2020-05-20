#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Header.h"

int labels = 1;
int pos = 0; //position of commands
const int memory_address = 4097;
int memlocation = 4097; //current memory location
int localmem = 1;
int paramem = -2;
int reg_temp[20], reg_temp2[20];
int junk;
int declflag;
int decltypeflag;
int Class_index = 0;

struct labeltable LabelTable[No_labels];
struct loop_counter *LOOP_COUNTER_HEAD = NULL, *LOOP_COUNTER_TEMP = NULL;
struct symboltable *Symbol_Table = NULL, *temptable = NULL, *GLOBAL_TABLE = NULL, *LOCAL_TABLE = NULL, *PARAM_TABLE = NULL;
char *Cur_type = NULL;

struct parameter *Temp_Parameter = NULL, *V_Parameter = NULL;
struct Reg_counter *REG_COUNTER = NULL;
struct Typetable *TYPE_TABLE;
struct Classtable *CLASS_TABLE, *Cptr, *Current_class;

struct symboltable *GLookup(char *name)
{
    struct symboltable *Symbol_Temp = GLOBAL_TABLE;
    while (Symbol_Temp)
    {
        if (strcmp(Symbol_Temp->name, name) == 0)
            return Symbol_Temp;
        Symbol_Temp = Symbol_Temp->prev;
    }
    return NULL;
}
struct symboltable *LLookup(char *name)
{
    if (LOCAL_TABLE == NULL)
        return NULL;
    struct symboltable *Symbol_Temp = LOCAL_TABLE;
    while (Symbol_Temp)
    {
        if (strcmp(Symbol_Temp->name, name) == 0)
            return Symbol_Temp;
        Symbol_Temp = Symbol_Temp->prev;
    }
    return NULL;
}
struct symboltable *PLookup(char *name)
{
    if (PARAM_TABLE == NULL)
        return NULL;
    struct symboltable *Symbol_Temp = PARAM_TABLE;
    while (Symbol_Temp)
    {
        if (strcmp(Symbol_Temp->name, name) == 0)
            return Symbol_Temp;
        Symbol_Temp = Symbol_Temp->prev;
    }
    return NULL;
}
struct tnode *CreateTree(int val, char *type, char *varname, int nodetype, char *op, tnode *l, tnode *r, struct symboltable *table)
{
    struct Classtable *class_temp = NULL;
    if (nodetype == OPERATOR)
    {

        if (r == NULL)
        {
            if (l->type != INTE)
            {
                printf("ERROR type mismatch in operator %s %s  \n", l->varname, op);
                exit(0);
            }
        }
        if (l->type && r->type)
        {
            if (strcmp(l->type, r->type))
            {
                //printf("adsf %s", r->type);
                struct Classtable *left_class = CLookup(l->type), *right_class = CLookup(r->type);
                if (left_class && right_class)
                {
                    if (!is_parent(left_class, right_class))
                    {
                        printf("ERROR type mismatch (not a parent) %s %s = %s %s\n", l->type, l->varname, r->type, r->varname);
                        exit(0);
                    }
                }
                else if (strcmp(r->type, "NULL"))
                {
                    printf("ERROR type mismatch %s %s = %s %s\n", l->type, l->varname, r->type, r->varname);
                    exit(0);
                }
            }
        }
    }
    if ((nodetype == WHILEST) || (nodetype == IFST))
    {
        if (strcmp(l->type, BOLE))
        {
            printf("\nERROR type mismatch BOOL\n");
            exit(0);
        }
    }
    if (nodetype == VARIABLE)
    {
        if (strcmp(varname, "NULL"))
        {
            struct symboltable *Symbol_Temp = NULL;
            if ((Symbol_Temp = LLookup(varname)) || (Symbol_Temp = PLookup(varname)))
            {
                if (Symbol_Temp->type)
                    type = strdup(Symbol_Temp->type);
                else
                    type = strdup(Symbol_Temp->ctype);
            }
            else if (Symbol_Temp = GLookup(varname))
            {
                if (Symbol_Temp->type)
                    type = strdup(Symbol_Temp->type);
                else
                {
                    printf("CTYPE");
                    type = strdup(Symbol_Temp->ctype);
                }
            }
            if (l != NULL)
            {
                if (strcmp(l->type, INTE))
                {
                    printf("type mismatch INT\n");
                    exit(0);
                }
            }
            if (Symbol_Temp == NULL)
            {
                printf("\nERROR Variable undeclared : %s\n", varname);
                exit(0);
            }
        }
    }
    if (nodetype == FUNCALL)
    {
        struct symboltable *Symbol_Temp;
        Symbol_Temp = GLookup(varname);
        if (Symbol_Temp == NULL)
        {
            printf("\nFUNERROR Variable Not declared : %s", varname);
            exit(0);
        }
        type = strdup(Symbol_Temp->type);
        //printf("%s", type);
        if (arguementcheck(Symbol_Temp->paramlist, l))
        {
            printf("\nERROR CALL argument unmatched in %s", varname);
            exit(0);
        }
    }
    if (nodetype == CFUNCALL)
    {

        class_temp = CLookup(l->type);
        struct Memberfunclist *Mfieldt;
        if (class_temp)
        {
            Mfieldt = Class_Mlookup(class_temp, varname);
            if (Mfieldt == NULL)
            {
                printf("ERROR Undeclared CFUNCTION(field5) : ...%s(%s).%s", l->varname, l->type, varname);
                exit(0);
            }
            if (Mfieldt->Type)
                type = strdup(Mfieldt->Type->name);
            else
            {
                printf("ERROR Undeclared CFUNCTION(field5) : ...%s(%s).%s", l->varname, l->type, varname);
                exit(0);
            }
        }
        else
        {
            printf("ERROR Undeclared variable(field3) : ...%s(%s).%s", l->varname, l->type, varname);
            exit(0);
        }
        if (type == NULL)
        {
            printf("TYPEERROR : %s", varname);
            exit(0);
        }

        if (arguementcheck(Mfieldt->paramlist, r) == 1)
        {
            printf("\nERROR C_CALL argument unmatched in %s", varname);
            exit(0);
        }
    }
    if ((nodetype == FUNCDEF) && (strcmp(varname, "MAIN")))
    {

        struct symboltable *Symbol_Temp;
        struct Memberfunclist *fun_temp;
        struct parameter *para_temp;
        char *type_temp;
        Symbol_Temp = GLookup(varname);
        if (Symbol_Temp == NULL)
        {
            if (Current_class)
            {
                fun_temp = Class_Mlookup(Current_class, varname);
                if (fun_temp)
                {
                    type_temp = strdup(fun_temp->Type->name);
                    para_temp = fun_temp->paramlist;
                }
                else
                {
                    //printf("%s", Current_class->Name);
                    printf("\nFUNDEFERROR Variable Not declared : %s", varname);
                    exit(0);
                }
            }
            else
            {
                printf("\nFUNDEFERROR Variable Not declared : %s", varname);
                exit(0);
            }
        }
        else
        {
            type_temp = strdup(Symbol_Temp->type);
            para_temp = Symbol_Temp->paramlist;
        }
        if (type_temp == NULL)
        {
            printf("check");
            exit(0);
        }
        //   printf("%s", type);
        /*if (strcmp(type_temp, type))
        {
            printf("ERROR type mismatch expected %s   %s %s(...)\n", Symbol_Temp->type, type, varname);
            exit(0);
        }*/
        // help_viewtable(l->Gsymbol, 3);

        if (l == NULL)
        {
            if (para_temp != NULL)
            {
                printf("\nERROR CALL argument unmatched in %s\n", varname);
                exit(0);
            }
        }
        if (l != NULL)
        {
            if (arguementcheck2(para_temp, l->Gsymbol))
            {
                printf("\nERROR Definition argument unmatched in %s\n", varname);
                exit(0);
            }
        }
    }
    if (nodetype == GDECLARATION)
    {
        GLOBAL_TABLE = table;
    }
    if (nodetype == DECLARATION)
    {
        LOCAL_TABLE = table;
    }
    if (nodetype == PDECLARATION)
    {
        struct symboltable *temptable, *temp;
        if (Current_class)
        {
            temptable = (struct symboltable *)malloc(sizeof(struct symboltable));
            temptable->name = strdup("self");
            temptable->ctype = strdup(Current_class->Name);
            temptable->size = 2;

            if (table == NULL)
            {
                table = temptable;
            }
            else
            {
                temp = table;
                while (temp->prev)
                {
                    temp = temp->prev;
                }
                temp->prev = temptable;
            }
        }
        PARAM_TABLE = table;
    }
    if (nodetype == FIELD)
    {
        if (l == NULL)
        {
            struct symboltable *Symbol_Temp = NULL;
            if (strcmp(varname, "self") == 0)
            {

                if (Current_class)
                    type = strdup(Current_class->Name);
                else
                {
                    printf("CERROR invalid use of self");
                    exit(0);
                }
                // printf("%s", type);
            }
            else
            {
                if ((Symbol_Temp = LLookup(varname)) || (Symbol_Temp = PLookup(varname)))
                {
                    if (Symbol_Temp->type)
                        type = strdup(Symbol_Temp->type);
                    else
                        type = strdup(Symbol_Temp->ctype);
                }
                else if (Symbol_Temp = GLookup(varname))
                {
                    if (Symbol_Temp->type)
                        type = strdup(Symbol_Temp->type);
                    else
                        type = strdup(Symbol_Temp->ctype);
                }
                if (Symbol_Temp == NULL)
                {
                    printf("\nERROR Variable undeclared(field1) : %s\n", varname);
                    exit(0);
                }
            }
        }
        else
        {
            struct Typetable *typet = TLookup(l->type);
            struct Classtable *class_temp = CLookup(l->type);
            struct Fieldlist *fieldt;
            if (typet)
            {
                fieldt = FLookup(typet, varname);
                if (fieldt->type == NULL)
                {
                    type = strdup(TLookup(fieldt->typename)->name);
                    if (type == NULL)
                    {
                        printf("ERROR Undeclared variable(field2) : %s", fieldt->typename);
                        exit(0);
                    }
                    // printf("%s %s \n", type, varname);
                }
                else
                    type = strdup(fieldt->type->name);
            }
            else if (class_temp)
            {
                fieldt = Class_Flookup(class_temp, varname);
                if (fieldt == NULL)
                {
                    printf("ERROR Undeclared variable(field3) : ...%s(%s).%s", l->varname, l->type, varname);
                    exit(0);
                }
                if (fieldt->Ctype)
                    type = strdup(fieldt->Ctype->Name);
                else if (fieldt->type)
                    type = strdup(fieldt->type->name);
                else
                {
                    printf("ERROR Undeclared variable(field3) : ...%s(%s).%s", l->varname, l->type, varname);
                    exit(0);
                }
            }
            else
            {
                printf("ERROR Undeclared variable(field3) : ...%s(%s).%s", l->varname, l->type, varname);
                exit(0);
            }
        }
        if (type == NULL)
        {
            printf("TYPEERROR : %s", varname);
        }
    }
    if (nodetype == NEW0)
    {
        type = varname;
    }
    struct tnode *temp;
    temp = (struct tnode *)malloc(sizeof(struct tnode));

    temp->op = op;
    temp->varname = varname;
    temp->nodetype = nodetype;
    temp->val = val;
    temp->type = type;
    temp->left = l;
    temp->right = r;
    temp->Gsymbol = table;
    temp->cptr = class_temp;
    return temp;
}
int Gallocatemem(int n)
{
    int mem = 0;
    mem = memlocation;
    memlocation = memlocation + n;
    return mem;
}
int Lallocatemem(int n, FILE *targetfile)
{
    int temp = localmem;
    localmem = localmem + n;
    if (n > 1)
        printf("var size greater");
    for (int i = 0; i < n; i++)
    {
        fprintf(targetfile, " PUSH R19\n");
        pos++;
    }
    return temp;
}
int Pallocatemem(int n)
{
    paramem = paramem - n;
    return paramem;
}
int min(int a, int b)
{
    if (a > b)
        return b;
    else
        return a;
}
int max(int a, int b)
{
    if (a > b)
        return a;
    else
        return b;
}
int getReg(int *Reg)
{
    int min = 30;
    for (int i = 0; i < 20; i++)
    {
        if (!Reg[i])
        {
            Reg[i] = 1;
            return i;
        }
    }
    printf("Run time error : NO REG AVAILABLE");
    return -1;
}
int getlabel()
{
    return labels++;
}
void freeReg(int *Reg)
{
    for (int i = 19; i >= 0; i--)
    {
        if (Reg[i])
        {
            Reg[i] = 0;
            return;
        }
    }
}
void pushReg(int *Reg, FILE *targetfile)
{
    for (int i = 0; i < 20; i++)
    {
        if (Reg[i])
        {
            fprintf(targetfile, " PUSH R%d\n", i);
            pos++;
        }
    }
    return;
}
void popReg(int *Reg, FILE *targetfile)
{
    for (int i = 19; i >= 0; i--)
    {
        if (Reg[i])
        {
            fprintf(targetfile, " POP R%d\n", i);
            pos++;
        }
    }
    return;
}
void popArgument(struct parameter *paramlist, FILE *targetfile)
{
    while (paramlist)
    {
        fprintf(targetfile, " POP R19\n");
        pos++;
        paramlist = paramlist->prev;
    }
}
int codeGen(struct tnode *t, FILE *targetfile, int option) //option 1 = value 0 = return address
{
    int r1, r2, r3, r4;
    if (t == NULL)
    {
        return 0;
    }
    if (t->nodetype == CONNECTOR)
    {
        if (t->left)
        {
            junk = codeGen(t->left, targetfile, 0);
        }
        if (t->right)
        {
            junk = codeGen(t->right, targetfile, 0);
        }
        return 0;
    }
    if (t->nodetype == OPERATOR)
    {

        if (strcmp(t->op, "+") == 0)
        {
            r1 = codeGen(t->left, targetfile, 1);
            r2 = codeGen(t->right, targetfile, 1);
            fprintf(targetfile, " ADD R%d,R%d\n", r1, r2);
            pos++;
            freeReg(REG_COUNTER->Reg);
            return (min(r1, r2));
        }
        if (strcmp(t->op, "-") == 0)
        {
            r1 = codeGen(t->left, targetfile, 1);
            r2 = codeGen(t->right, targetfile, 1);
            fprintf(targetfile, " SUB R%d,R%d\n", r1, r2);
            pos++;
            freeReg(REG_COUNTER->Reg);
            return (min(r1, r2));
        }
        if (strcmp(t->op, "*") == 0)
        {
            r1 = codeGen(t->left, targetfile, 1);
            r2 = codeGen(t->right, targetfile, 1);
            fprintf(targetfile, " MUL R%d,R%d\n", r1, r2);
            pos++;
            freeReg(REG_COUNTER->Reg);
            return (min(r1, r2));
        }
        if (strcmp(t->op, "/") == 0)
        {
            r1 = codeGen(t->left, targetfile, 1);
            r2 = codeGen(t->right, targetfile, 1);
            fprintf(targetfile, " DIV R%d,R%d\n", r1, r2);
            pos++;
            freeReg(REG_COUNTER->Reg);
            return (min(r1, r2));
        }
        if (strcmp(t->op, "=") == 0)
        {
            r1 = codeGen(t->left, targetfile, 0);
            if (t->right->nodetype == NEW0)
            {
                fprintf(targetfile, " ADD R%d,%d\n", r1, 1);
                pos++;
                r2 = codeGen(t->right, targetfile, 1);
                fprintf(targetfile, " MOV [R%d],R%d\n", r1, r2);
                pos++;
            }
            else if (CLookup(t->left->type) && CLookup(t->right->type))
            {
                r2 = codeGen(t->right, targetfile, 0);
                fprintf(targetfile, " MOV [R%d],[R%d]\n", r1, r2);
                pos++;
                fprintf(targetfile, " ADD R%d,%d\n", r1, 1);
                pos++;
                fprintf(targetfile, " ADD R%d,%d\n", r2, 1);
                pos++;
                fprintf(targetfile, " MOV [R%d],[R%d]\n", r1, r2);
                pos++;
            }
            else
            {
                r2 = codeGen(t->right, targetfile, 1);
                fprintf(targetfile, " MOV [R%d],R%d\n", r1, r2);
                pos++;
            }
            freeReg(REG_COUNTER->Reg);
            freeReg(REG_COUNTER->Reg);
            return 0;
        }
        if (strcmp(t->op, "<") == 0)
        {
            r1 = codeGen(t->left, targetfile, 1);
            r2 = codeGen(t->right, targetfile, 1);
            fprintf(targetfile, " LT R%d,R%d\n", r1, r2);
            pos++;
            freeReg(REG_COUNTER->Reg);
            return (min(r1, r2));
        }
        if (strcmp(t->op, "<=") == 0)
        {
            r1 = codeGen(t->left, targetfile, 1);
            r2 = codeGen(t->right, targetfile, 1);
            fprintf(targetfile, " LE R%d,R%d\n", r1, r2);
            pos++;
            freeReg(REG_COUNTER->Reg);
            return (min(r1, r2));
        }
        if (strcmp(t->op, ">") == 0)
        {
            r1 = codeGen(t->left, targetfile, 1);
            r2 = codeGen(t->right, targetfile, 1);
            fprintf(targetfile, " GT R%d,R%d\n", r1, r2);
            pos++;
            freeReg(REG_COUNTER->Reg);
            return (min(r1, r2));
        }
        if (strcmp(t->op, ">=") == 0)
        {
            r1 = codeGen(t->left, targetfile, 1);
            r2 = codeGen(t->right, targetfile, 1);
            fprintf(targetfile, " GE R%d,R%d\n", r1, r2);
            pos++;
            freeReg(REG_COUNTER->Reg);
            return (min(r1, r2));
        }
        if (strcmp(t->op, "==") == 0)
        {
            r1 = codeGen(t->left, targetfile, 1);
            r2 = codeGen(t->right, targetfile, 1);
            fprintf(targetfile, " EQ R%d,R%d\n", r1, r2);
            pos++;
            freeReg(REG_COUNTER->Reg);
            return (min(r1, r2));
        }
        if (strcmp(t->op, "!=") == 0)
        {
            r1 = codeGen(t->left, targetfile, 1);
            r2 = codeGen(t->right, targetfile, 1);
            fprintf(targetfile, " NE R%d,R%d\n", r1, r2);
            pos++;
            freeReg(REG_COUNTER->Reg);
            return (min(r1, r2));
        }
    }
    if (t->nodetype == WRITE0)
    {
        r1 = codeGen(t->left, targetfile, 0);
        if (t->left->nodetype == VARIABLE || t->left->nodetype == FIELD)
        {
            r2 = getReg(REG_COUNTER->Reg);
            fprintf(targetfile, " MOV R%d, \"Write\"\n", r2);
            pos++;
            fprintf(targetfile, " PUSH R%d\n", r2);
            pos++;
            fprintf(targetfile, " MOV R%d, -2\n", r2);
            pos++;
            fprintf(targetfile, " PUSH R%d\n", r2);
            pos++;
            fprintf(targetfile, " MOV R%d,[R%d]\n", r2, r1);
            pos++;
            fprintf(targetfile, " PUSH R%d\n", r2);
            pos++;
            fprintf(targetfile, " PUSH R1\n");
            pos++;
            fprintf(targetfile, " PUSH R1\n");
            pos++;
            fprintf(targetfile, " CALL 0\n");
            pos++;
            fprintf(targetfile, " POP R0\n");
            pos++;
            fprintf(targetfile, " POP R1\n");
            pos++;
            fprintf(targetfile, " POP R1\n");
            pos++;
            fprintf(targetfile, " POP R1\n");
            pos++;
            fprintf(targetfile, " POP R1\n");
            pos++;
            freeReg(REG_COUNTER->Reg);
        }
        else if ((t->left->nodetype == NUMBER) || (t->left->nodetype == OPERATOR) || (t->left->nodetype == STRI) || t->left->nodetype == CFUNCALL)
        {
            fprintf(targetfile, " MOV R0,R%d\n", r1);
            pos++;
            fprintf(targetfile, " PUSH R1\n");
            pos++;
            fprintf(targetfile, " PUSH R0\n");
            pos++;
            fprintf(targetfile, " MOV R1, \"Write\"\n");
            pos++;
            fprintf(targetfile, " PUSH R1\n");
            pos++;
            fprintf(targetfile, " MOV R1, -2\n");
            pos++;
            fprintf(targetfile, " PUSH R1\n");
            pos++;
            fprintf(targetfile, " PUSH R0\n");
            pos++;
            fprintf(targetfile, " PUSH R1\n");
            pos++;
            fprintf(targetfile, " PUSH R1\n");
            pos++;
            fprintf(targetfile, " CALL 0\n");
            pos++;
            fprintf(targetfile, " POP R0\n");
            pos++;
            fprintf(targetfile, " POP R1\n");
            pos++;
            fprintf(targetfile, " POP R1\n");
            pos++;
            fprintf(targetfile, " POP R1\n");
            pos++;
            fprintf(targetfile, " POP R1\n");
            pos++;
            fprintf(targetfile, " POP R0\n");
            pos++;
            fprintf(targetfile, " POP R1\n");
            pos++;
        }
        else
        {
            printf("ERROR WRITE()");
            exit(0);
        }
        freeReg(REG_COUNTER->Reg);
        return 0;
    }
    if (t->nodetype == POWER)
    {
        int l = 0;
        char name[6];
        r2 = getReg(REG_COUNTER->Reg);
        r1 = codeGen(t->left, targetfile, 1);

        //printf("%s",LabelTable[0].name);
        fprintf(targetfile, " MOV R%d,1\n", r2);
        pos++;
        fprintf(targetfile, " PUSH R0\n");
        pos++;
        fprintf(targetfile, " MOV R0,R%d\n", r1);
        pos++;
        l = getlabel();
        fprintf(targetfile, "L%d:\n", l);
        LabelTable[l].address = pos * 2 + start_adress;
        //printf("%d\n",l);
        snprintf(name, 6, "L%d", l);
        strcpy(LabelTable[l].name, name);
        fwrite(&LabelTable[l], sizeof(struct labeltable), 1, label_file);
        fprintf(targetfile, " MUL R%d,2\n", r2);
        pos++;
        fprintf(targetfile, " SUB R0,1\n");
        pos++;
        fprintf(targetfile, " JNZ R0,L%d\n", l);
        pos++;
        freeReg(REG_COUNTER->Reg);
        return r2;
    }
    if (t->nodetype == READ0)
    {
        r1 = codeGen(t->left, targetfile, 0);
        r2 = getReg(REG_COUNTER->Reg);
        fprintf(targetfile, " MOV R%d, \"Read\"\n", r2);
        pos++;
        fprintf(targetfile, " PUSH R%d\n", r2);
        pos++;
        fprintf(targetfile, " MOV R%d,-1\n", r2);
        pos++;
        fprintf(targetfile, " PUSH R%d\n", r2);
        pos++;
        fprintf(targetfile, " PUSH R%d\n", r1);
        pos++;
        fprintf(targetfile, " PUSH R%d\n", r2);
        pos++;
        fprintf(targetfile, " PUSH R%d\n", r2);
        pos++;
        fprintf(targetfile, " CALL 0\n");
        pos++;
        fprintf(targetfile, " POP R19\n");
        pos++;
        fprintf(targetfile, " POP R19\n");
        pos++;
        fprintf(targetfile, " POP R19\n");
        pos++;
        fprintf(targetfile, " POP R19\n");
        pos++;
        fprintf(targetfile, " POP R19\n");
        pos++;
        freeReg(REG_COUNTER->Reg);
        freeReg(REG_COUNTER->Reg);
        return 0;
    }
    if (t->nodetype == NUMBER)
    {
        r1 = getReg(REG_COUNTER->Reg);
        fprintf(targetfile, " MOV R%d,%d\n", r1, t->val);
        pos++;
        return r1;
    }
    if (t->nodetype == STRI)
    {
        r1 = getReg(REG_COUNTER->Reg);
        fprintf(targetfile, " MOV R%d,%s\n", r1, t->varname);
        pos++;
        return r1;
    }
    if (t->nodetype == VARIABLE)
    {
        struct symboltable *Symbol_Temp = NULL;
        if (strcmp(t->varname, "NULL") == 0)
        {

            r2 = getReg(REG_COUNTER->Reg);
            fprintf(targetfile, " MOV R%d,0\n", r2);
            pos++;
            return r2;
        }
        if ((Symbol_Temp = LLookup(t->varname)) || (Symbol_Temp = PLookup(t->varname)))
        {

            if (t->left == NULL)
            {
                if (option == 1)
                {
                    r1 = Symbol_Temp->binding;
                    r2 = getReg(REG_COUNTER->Reg);
                    r3 = getReg(REG_COUNTER->Reg);
                    fprintf(targetfile, " MOV R%d,BP\n", r3);
                    pos++;
                    fprintf(targetfile, " ADD R%d,%d\n", r3, r1);
                    pos++;
                    fprintf(targetfile, " MOV R%d,[R%d]\n", r2, r3);
                    pos++;
                    freeReg(REG_COUNTER->Reg);
                    return r2;
                }
                if (option == 0)
                {
                    r1 = Symbol_Temp->binding;
                    r2 = getReg(REG_COUNTER->Reg);
                    fprintf(targetfile, " MOV R%d,BP\n", r2);
                    pos++;
                    fprintf(targetfile, " ADD R%d,%d\n", r2, r1);
                    pos++;
                    return r2;
                }
            }
            else
            {
                printf("not null");
                int size = t->left->val;
                if (option == 1)
                {
                    r1 = Symbol_Temp->binding;
                    r2 = getReg(REG_COUNTER->Reg);
                    r3 = getReg(REG_COUNTER->Reg);
                    r4 = codeGen(t->left, targetfile, 1);
                    fprintf(targetfile, " MOV R%d,BP\n", r3);
                    pos++;
                    fprintf(targetfile, " ADD R%d,%d\n", r3, r4);
                    pos++;
                    fprintf(targetfile, " ADD R%d,%d\n", r3, r1);
                    pos++;
                    fprintf(targetfile, " MOV R%d,[R%d]\n", r2, r3);
                    pos++;
                    freeReg(REG_COUNTER->Reg);
                    freeReg(REG_COUNTER->Reg);
                    return r2;
                }
                if (option == 0)
                {
                    r1 = Symbol_Temp->binding;
                    r2 = getReg(REG_COUNTER->Reg);
                    r4 = codeGen(t->left, targetfile, 1);
                    fprintf(targetfile, " MOV R%d,BP\n", r2);
                    pos++;
                    fprintf(targetfile, " ADD R%d,%d\n", r3, r4);
                    pos++;
                    fprintf(targetfile, " ADD R%d,%d\n", r2, r1);
                    pos++;
                    freeReg(REG_COUNTER->Reg);

                    return r2;
                }
            }
        }

        else if (Symbol_Temp = GLookup(t->varname))
        {
            if (Symbol_Temp == NULL)
            {
                printf("\n Variable Not declared : %s", t->varname);
                exit(0);
            }
            if (t->left == NULL)
            {
                if (option == 1)
                {
                    r1 = Symbol_Temp->binding;
                    r2 = getReg(REG_COUNTER->Reg);
                    fprintf(targetfile, " MOV R%d,[%d]\n", r2, r1);
                    pos++;
                    return r2;
                }
                if (option == 0)
                {
                    r1 = Symbol_Temp->binding;
                    r2 = getReg(REG_COUNTER->Reg);
                    fprintf(targetfile, " MOV R%d,%d\n", r2, r1);
                    pos++;
                    return r2;
                }
            }
            else
            {
                int size = t->left->val;
                if (option == 1)
                {
                    r2 = getReg(REG_COUNTER->Reg);
                    r3 = codeGen(t->left, targetfile, 1);
                    r1 = Symbol_Temp->binding;
                    fprintf(targetfile, " ADD R%d,%d\n", r3, r1);
                    pos++;
                    fprintf(targetfile, " MOV R%d,[R%d]\n", r2, r3);
                    pos++;
                    freeReg(REG_COUNTER->Reg);
                    return r2;
                }
                if (option == 0)
                {
                    r2 = getReg(REG_COUNTER->Reg);
                    r3 = codeGen(t->left, targetfile, 1);
                    r1 = Symbol_Temp->binding;
                    fprintf(targetfile, " ADD R%d,%d\n", r3, r1);
                    pos++;
                    fprintf(targetfile, " MOV R%d,R%d\n", r2, r3);
                    pos++;
                    freeReg(REG_COUNTER->Reg);

                    return r2;
                }
            }
        }

        return 0;
    }

    if (t->nodetype == IFST)
    {
        int l_if = 0, l_end = 0;
        char name[6];
        l_if = getlabel();
        l_end = getlabel(); //L_END:
        r1 = codeGen(t->left, targetfile, 1);
        fprintf(targetfile, " JZ R%d,L%d\n", r1, l_if);
        pos++;
        freeReg(REG_COUNTER->Reg);
        junk = codeGen(t->right->left, targetfile, 1);
        fprintf(targetfile, " JMP L%d\n", l_end);
        pos++;
        //SLIST
        LabelTable[l_if].address = pos * 2 + start_adress; //JMP L_WHILE
        snprintf(name, 6, "L%d", l_if);
        strcpy(LabelTable[l_if].name, name);
        fwrite(&LabelTable[l_if], sizeof(struct labeltable), 1, label_file);
        fprintf(targetfile, "L%d:\n", l_if);
        if (t->right->right)
        {
            junk = codeGen(t->right->right, targetfile, 1);
        }
        LabelTable[l_end].address = pos * 2 + start_adress; //JMP L_WHILE
        snprintf(name, 6, "L%d", l_end);
        strcpy(LabelTable[l_end].name, name);
        fwrite(&LabelTable[l_end], sizeof(struct labeltable), 1, label_file);
        fprintf(targetfile, "L%d:\n", l_end);
        return 0;
    }
    if (t->nodetype == WHILEST)
    {
        LOOP_COUNTER_TEMP = LOOP_COUNTER_HEAD;
        LOOP_COUNTER_HEAD = (struct loop_counter *)malloc(sizeof(struct loop_counter));
        LOOP_COUNTER_HEAD->prev = LOOP_COUNTER_TEMP;

        int l_while = 0, l_end = 0;                           //WHILE AND END LABEL
        char name[6];                                         //L_WHILE:
        l_while = getlabel();                                 //CONDI
        l_end = getlabel();                                   //JZ L_END
        fprintf(targetfile, "L%d:\n", l_while);               //SLIST
        LabelTable[l_while].address = pos * 2 + start_adress; //JMP L_WHILE
        snprintf(name, 6, "L%d", l_while);
        strcpy(LOOP_COUNTER_HEAD->continue_label, name); //L_END:
        strcpy(LabelTable[l_while].name, name);
        fwrite(&LabelTable[l_while], sizeof(struct labeltable), 1, label_file);
        r1 = codeGen(t->left, targetfile, 1);
        fprintf(targetfile, " JZ R%d,L%d\n", r1, l_end);
        pos++;
        freeReg(REG_COUNTER->Reg);
        snprintf(name, 6, "L%d", l_end);
        strcpy(LOOP_COUNTER_HEAD->break_label, name);
        junk = codeGen(t->right, targetfile, 1);
        fprintf(targetfile, " JMP L%d\n", l_while);
        pos++;
        fprintf(targetfile, "L%d:\n", l_end);
        LabelTable[l_end].address = pos * 2 + start_adress;
        snprintf(name, 6, "L%d", l_end);
        strcpy(LabelTable[l_end].name, name);
        fwrite(&LabelTable[l_end], sizeof(struct labeltable), 1, label_file);
        LOOP_COUNTER_HEAD = LOOP_COUNTER_HEAD->prev;
        return 0;
    }
    if (t->nodetype == BREAKST)
    {
        if (LOOP_COUNTER_HEAD == NULL)
            return 0;
        fprintf(targetfile, " JMP %s\n", LOOP_COUNTER_HEAD->break_label);
        pos++;
        return 0;
    }
    if (t->nodetype == CONTINUEST)
    {
        if (LOOP_COUNTER_HEAD == NULL)
            return 0;
        fprintf(targetfile, " JMP %s\n", LOOP_COUNTER_HEAD->continue_label);
        pos++;
        return 0;
    }
    if (t->nodetype == DECLARATION)
    {
        LOCAL_TABLE = t->Gsymbol;
        temptable = LOCAL_TABLE;
        //help_viewtable(temptable, 1);
        while (temptable)
        {
            temptable->binding = Lallocatemem(temptable->size, targetfile);
            temptable = temptable->prev;
        }
        return 0;
    }
    if (t->nodetype == GDECLARATION)
    {

        temptable = t->Gsymbol;
        while (temptable)
        {
            temptable->binding = Gallocatemem(temptable->size);
            temptable = temptable->prev;
        }
        // help_viewtable(GLOBAL_TABLE, 1);
        return 0;
    }
    if (t->nodetype == PDECLARATION)
    {
        PARAM_TABLE = t->Gsymbol;
        temptable = PARAM_TABLE;
        while (temptable)
        {
            temptable->binding = Pallocatemem(temptable->size);
            temptable = temptable->prev;
        }
        paramem = -2;
        //  help_viewtable(PARAM_TABLE->val, 2);
        return 0;
    }
    if (t->nodetype == ARGUMENT)
    {
        r1 = codeGen(t->left, targetfile, 1);
        fprintf(targetfile, " PUSH R%d\n", r1);
        pos++;
        freeReg(REG_COUNTER->Reg);
        return 0;
    }
    if (t->nodetype == FUNCALL)
    {
        int *reg_temp1 = (int *)malloc(sizeof(int) * 20);
        struct parameter *paramlist;
        memcpy(reg_temp1, REG_COUNTER->Reg, sizeof(int) * 20); //backing up reg
        memset(REG_COUNTER->Reg, 0, sizeof(int) * 20);         //reseting available reg
        pushReg(reg_temp1, targetfile);                        //storing memory
        junk = codeGen(t->left, targetfile, 1);                // generating args
        fprintf(targetfile, " PUSH R19\n");
        pos++;
        struct symboltable *func_info = GLookup(t->varname);
        if (func_info == NULL)
        {
            printf("ERROR  function undeclared : %s ", t->varname);
            exit(0);
        }
        fprintf(targetfile, " CALL L%d\n", func_info->flabel);
        pos++;
        memcpy(REG_COUNTER->Reg, reg_temp1, sizeof(int) * 20); //restoring reg
        r1 = getReg(REG_COUNTER->Reg);                         //reg for return
        fprintf(targetfile, " POP R%d\n", r1);
        pos++;
        paramlist = func_info->paramlist;
        popArgument(paramlist, targetfile);
        popReg(reg_temp1, targetfile);
        return r1;
    }
    if (t->nodetype == CFUNCALL)
    {
        int *reg_temp1 = (int *)malloc(sizeof(int) * 20);
        //help_viewtable(PARAM_TABLE->val, 2);
        struct parameter *paramlist;
        memcpy(reg_temp1, REG_COUNTER->Reg, sizeof(int) * 20);
        memset(REG_COUNTER->Reg, 0, sizeof(int) * 20);
        pushReg(reg_temp1, targetfile);
        r2 = codeGen(t->left, targetfile, 1);   //calling field reducer returns the
        fprintf(targetfile, " PUSH R%d\n", r2); //reference value in object
        pos++;
        freeReg(REG_COUNTER->Reg);
        r3 = getReg(REG_COUNTER->Reg);
        r2 = codeGen(t->left, targetfile, 0);
        fprintf(targetfile, " ADD R%d ,%d\n", r2, 1);
        pos++;
        fprintf(targetfile, " MOV R%d ,[R%d]\n", r3, r2); //virtual fuctionpointer in object
        pos++;
        freeReg(REG_COUNTER->Reg);
        fprintf(targetfile, " PUSH R%d\n", r3); //virtual fuctionpointer in object
        pos++;
        // freeReg(REG_COUNTER->Reg);
        junk = codeGen(t->right, targetfile, 1); //calling arguments pushing args
        fprintf(targetfile, " PUSH R19\n");      // reg for return value
        pos++;
        struct Memberfunclist *func_info = Class_Mlookup(t->cptr, t->varname);
        if (func_info == NULL)
        {
            printf("ERROR  function undeclared : %s ", t->varname);
            exit(0);
        }
        r1 = getReg(REG_COUNTER->Reg);
        fprintf(targetfile, " ADD R%d,%d\n", r3, func_info->Funcposition);
        pos++;
        fprintf(targetfile, " MOV R%d,[R%d]\n", r1, r3);
        pos++;
        freeReg(REG_COUNTER->Reg);
        fprintf(targetfile, " CALL R%d\n", r1);
        pos++;
        freeReg(REG_COUNTER->Reg);

        memcpy(REG_COUNTER->Reg, reg_temp1, sizeof(int) * 20);
        r1 = getReg(REG_COUNTER->Reg);         //reg for return value
        fprintf(targetfile, " POP R%d\n", r1); //reg for return
        pos++;
        fprintf(targetfile, " POP R19\n");
        pos++;
        fprintf(targetfile, " POP R19\n");
        pos++;
        paramlist = func_info->paramlist;
        popArgument(paramlist, targetfile);
        popReg(reg_temp1, targetfile);

        return r1;
    }
    if (t->nodetype == RETURNST)
    {
        r1 = codeGen(t->left, targetfile, 1);
        r2 = getReg(REG_COUNTER->Reg);
        fprintf(targetfile, " MOV R%d,BP\n", r2);
        pos++;
        fprintf(targetfile, " ADD R%d,-2\n", r2);
        pos++;
        fprintf(targetfile, " MOV [R%d],R%d\n", r2, r1);
        pos++;
        freeReg(REG_COUNTER->Reg);
        freeReg(REG_COUNTER->Reg);
        localmem = 1;
        fprintf(targetfile, " MOV SP,BP\n");
        pos++;
        fprintf(targetfile, " POP BP\n");
        pos++;
        fprintf(targetfile, " RET\n");
        pos++;
        return 0;
    }

    if (t->nodetype == FUNCDEF)
    {
        //help_viewtable(GLOBAL_TABLE, 1);
        struct Reg_counter *TEMP_REG_COUNTER = NULL;
        TEMP_REG_COUNTER = (struct Reg_counter *)malloc(sizeof(struct Reg_counter));
        TEMP_REG_COUNTER->Reg = (int *)malloc(sizeof(int) * 20);
        TEMP_REG_COUNTER->prev = REG_COUNTER;
        REG_COUNTER = TEMP_REG_COUNTER;
        struct symboltable *Symbol_Temp;
        struct Memberfunclist *Mfield;
        char *name = malloc(sizeof(char) * 6);
        int label;

        if (strcmp(t->varname, "MAIN") == 0)
        {
            label = 0; //main label is 0
        }
        else
        {
            Symbol_Temp = GLookup(t->varname);
            Mfield = Class_Mlookup(Current_class, t->varname);

            if (Symbol_Temp == NULL && Mfield == NULL)
            {
                printf("\nFunction Not declared : %s\n", t->varname);
                exit(0);
            }
            else
            {
                if (Symbol_Temp)
                    label = Symbol_Temp->flabel;
                else if (Mfield)
                    label = Mfield->Flabel;
            }
        }
        fprintf(targetfile, "L%d:\n", label);
        LabelTable[label].address = pos * 2 + start_adress;
        if (strcmp(t->varname, "MAIN") == 0)
        {
            create_virtual_class_table(targetfile);
        }
        snprintf(name, 6, "L%d", label);
        strcpy(LabelTable[label].name, name);
        fwrite(&LabelTable[label], sizeof(struct labeltable), 1, label_file);
        fprintf(targetfile, " PUSH BP\n");
        pos++;
        fprintf(targetfile, " MOV BP,SP\n");
        pos++;
        if (t->left)
        {
            junk = codeGen(t->left, targetfile, 0);
        }
        if (t->right)
        {
            junk = codeGen(t->right, targetfile, 0);
        }
        REG_COUNTER = REG_COUNTER->prev;
        return 0;
    }
    if (t->nodetype == FIELD)
    {
        if (t->left == NULL)
        {

            struct symboltable *Symbol_Temp = NULL;
            if ((Symbol_Temp = LLookup(t->varname)) || (Symbol_Temp = PLookup(t->varname)))
            {

                if (option == 1)
                {
                    r1 = Symbol_Temp->binding;
                    r2 = getReg(REG_COUNTER->Reg);
                    r3 = getReg(REG_COUNTER->Reg);
                    fprintf(targetfile, " MOV R%d,BP\n", r3);
                    pos++;
                    fprintf(targetfile, " ADD R%d,%d\n", r3, r1);
                    pos++;
                    fprintf(targetfile, " MOV R%d,[R%d]\n", r2, r3);
                    pos++;
                    freeReg(REG_COUNTER->Reg);
                }
                if (option == 0)
                {
                    r1 = Symbol_Temp->binding;
                    r2 = getReg(REG_COUNTER->Reg);
                    fprintf(targetfile, " MOV R%d,BP\n", r2);
                    pos++;
                    fprintf(targetfile, " ADD R%d,%d\n", r2, r1);
                    pos++;
                }
            }
            else if (Symbol_Temp = GLookup(t->varname))
            {
                if (Symbol_Temp == NULL)
                {
                    printf("\nFERROR Variable Not declared : %s", t->varname);
                    exit(0);
                }
                if (t->left == NULL)
                {
                    if (option == 1)
                    {
                        r1 = Symbol_Temp->binding;
                        r2 = getReg(REG_COUNTER->Reg);
                        fprintf(targetfile, " MOV R%d,[%d]\n", r2, r1);
                        pos++;
                    }
                    if (option == 0)
                    {
                        r1 = Symbol_Temp->binding;
                        r2 = getReg(REG_COUNTER->Reg);
                        fprintf(targetfile, " MOV R%d,%d\n", r2, r1);
                        pos++;
                    }
                }
            }
            if (Symbol_Temp->type)
                Cur_type = strdup(Symbol_Temp->type);
            else
                Cur_type = strdup(Symbol_Temp->ctype);
            return r2;
        }
        else
        {
            r2 = getReg(REG_COUNTER->Reg);
            r1 = codeGen(t->left, targetfile, 1);
            struct Typetable *Type_temp = TLookup(Cur_type);
            struct Classtable *class_temp = CLookup(Cur_type);
            if (Type_temp)
            {
                struct Fieldlist *Field_temp = FLookup(Type_temp, t->varname);
                if (Field_temp->type)
                    Cur_type = strdup(Field_temp->type->name);
                fprintf(targetfile, " ADD R%d,%d\n", r1, Field_temp->fieldIndex);
                pos++;
                if (option == 1)
                {
                    fprintf(targetfile, " MOV R%d,[R%d]\n", r2, r1);
                    pos++;
                    freeReg(REG_COUNTER->Reg);
                    return r2;
                }
                if (option == 0)
                {
                    fprintf(targetfile, " MOV R%d,R%d\n", r2, r1);
                    pos++;
                    freeReg(REG_COUNTER->Reg);
                    return r2;
                }
            }
            else if (class_temp)
            {
                struct Fieldlist *Field_temp = Class_Flookup(class_temp, t->varname);
                if (Field_temp->type)
                    Cur_type = strdup(Field_temp->type->name);
                fprintf(targetfile, " ADD R%d,%d\n", r1, Field_temp->fieldIndex);
                pos++;
                if (option == 1)
                {
                    fprintf(targetfile, " MOV R%d,[R%d]\n", r2, r1);
                    pos++;
                    freeReg(REG_COUNTER->Reg);
                    return r2;
                }
                if (option == 0)
                {
                    fprintf(targetfile, " MOV R%d,R%d\n", r2, r1);
                    pos++;
                    freeReg(REG_COUNTER->Reg);
                    return r2;
                }
            }
        }
    }
    if (t->nodetype == ALOC)
    {
        memcpy(reg_temp, REG_COUNTER->Reg, sizeof(int) * 20);
        pushReg(reg_temp, targetfile);
        r2 = getReg(REG_COUNTER->Reg);
        fprintf(targetfile, " MOV R19,\"Alloc\"\n");
        pos++;
        fprintf(targetfile, " PUSH R19\n");
        pos++;
        fprintf(targetfile, " MOV R0,8\n");
        pos++;
        fprintf(targetfile, " PUSH R19\n");
        pos++;
        fprintf(targetfile, " PUSH R19\n");
        pos++;
        fprintf(targetfile, " PUSH R19\n");
        pos++;
        fprintf(targetfile, " PUSH R%d\n", r2);
        pos++;
        fprintf(targetfile, " CALL 0\n");
        pos++;
        fprintf(targetfile, " POP R%d\n", r2);
        pos++;
        fprintf(targetfile, " POP R19\n");
        pos++;
        fprintf(targetfile, " POP R19\n");
        pos++;
        fprintf(targetfile, " POP R19\n");
        pos++;
        fprintf(targetfile, " POP R19\n");
        pos++;
        popReg(reg_temp, targetfile);

        return r2;
    }
    if (t->nodetype == DEALOC)
    {
        memcpy(reg_temp, REG_COUNTER->Reg, sizeof(int) * 20);
        pushReg(reg_temp, targetfile);
        r2 = codeGen(t->left, targetfile, 1);
        fprintf(targetfile, " MOV R19,\"Alloc\"\n");
        pos++;
        fprintf(targetfile, " PUSH R19\n");
        pos++;
        fprintf(targetfile, " PUSH R%d\n", r2);
        pos++;
        fprintf(targetfile, " PUSH R19\n");
        pos++;
        fprintf(targetfile, " PUSH R19\n");
        pos++;
        fprintf(targetfile, " PUSH R19\n");
        pos++;
        fprintf(targetfile, " CALL 0\n");
        pos++;
        fprintf(targetfile, " POP R19\n");
        pos++;
        fprintf(targetfile, " POP R19\n");
        pos++;
        fprintf(targetfile, " POP R19\n");
        pos++;
        fprintf(targetfile, " POP R19\n");
        pos++;
        fprintf(targetfile, " POP R19\n");
        pos++;
        freeReg(REG_COUNTER->Reg);
        popReg(reg_temp, targetfile);

        return 0;
    }
    if (t->nodetype == INIT)
    {
        memcpy(reg_temp, REG_COUNTER->Reg, sizeof(int) * 20);
        pushReg(reg_temp, targetfile);
        fprintf(targetfile, " MOV R19,\"intialize\"\n");
        pos++;
        fprintf(targetfile, " PUSH R19\n");
        pos++;
        fprintf(targetfile, " PUSH R19\n");
        pos++;
        fprintf(targetfile, " PUSH R19\n");
        pos++;
        fprintf(targetfile, " PUSH R19\n");
        pos++;
        fprintf(targetfile, " PUSH R19\n");
        pos++;
        fprintf(targetfile, " CALL 0\n");
        pos++;
        fprintf(targetfile, " POP R19\n");
        pos++;
        fprintf(targetfile, " POP R19\n");
        pos++;
        fprintf(targetfile, " POP R19\n");
        pos++;
        fprintf(targetfile, " POP R19\n");
        pos++;
        fprintf(targetfile, " POP R19\n");
        pos++;
        popReg(reg_temp, targetfile);
    }
    if (t->nodetype == BREAKP)
    {
        fprintf(targetfile, " BRKP\n");
        pos++;
    }
    if (t->nodetype == NEW0)
    {

        struct Classtable *cptr;
        cptr = CLookup(t->varname);
        if (!cptr)
        {
            printf("\nCERROR class not found :%s\n", t->varname);
            exit(0);
        }
        r1 = getReg(REG_COUNTER->Reg);
        //printf("virtual table %d", cptr->Class_index * 8 + memory_address);
        fprintf(targetfile, "MOV R%d,%d\n", r1, cptr->Class_index * 8 + memory_address);
        pos++;
        return r1;
    }
}
int evaluate(struct tnode *t)
{
    if (t == NULL)
        return 0;
    printf("\nnode type : %d \n", t->nodetype);
    if (t->nodetype == CONNECTOR)
    {
        evaluate(t->left);
        evaluate(t->right);
    }
    if (t->nodetype == OPERATOR)
    {
        evaluate(t->left);
        evaluate(t->right);
    }
    if (t->nodetype == IFST)
    {
        evaluate(t->left);
        evaluate(t->right->left);
        if (t->right->right)
            evaluate(t->right->right);
    }
    if (t->nodetype == WHILEST)
    {
        evaluate(t->left);
        evaluate(t->right);
    }
    if (t->nodetype == READ0)
    {
        evaluate(t->left);
    }
    if (t->nodetype == WRITE0)
    {
        evaluate(t->left);
    }
    if (t->nodetype == NUMBER)
    {
        return 0;
    }
    if (t->nodetype == DECLARATION)
    {
        help_viewtable(t->Gsymbol, 0);
    }
    if (t->nodetype == GDECLARATION)
    {
        help_viewtable(t->Gsymbol, 1);
    }
    if (t->nodetype == PDECLARATION)
    {
        help_viewtable(t->Gsymbol, 2);
    }
    return 0;
}

void help_viewtable(struct symboltable *table, int scope)
{
    if (scope == 1)
        printf("Golbal ");
    else if (scope == 2)
        printf("Parameter ");
    else
        printf("local ");
    printf("Table view : \n");
    struct symboltable *temptable;
    temptable = table;
    while (temptable)
    {
        printf("\n name : %s type : %s size : %d binding : %d flabel : %d", temptable->name, temptable->type, temptable->size, temptable->binding, temptable->flabel);
        temptable = temptable->prev;
    }
    printf("\nEND \n");
}

void help_viewReg(int *Reg)
{
    printf("\n REG: ");
    for (int i = 0; i < 20; i++)
    {
        if (Reg[i])
            printf("%d", i);
    }
}

int arguementcheck(struct parameter *parameters, struct tnode *l)
{
    int flag = 0;
    if (parameters)
        if (strcmp(parameters->name, "self") == 0)
        {
            parameters = parameters->prev;
        }
    if (l == NULL && parameters == NULL)
        return 0;
    if ((l != NULL && parameters == NULL) || (l == NULL && parameters != NULL))
    {
        printf(" arguement NULL\n");
        return 1;
    }
    if (l->nodetype == CONNECTOR)
    {
        flag = arguementcheck(parameters, l->right);
        if (flag == 1 || flag == 0)
            return flag;
        parameters = parameters->prev;
        flag = arguementcheck(parameters, l->left);
        if (flag == 1 || flag == 0)
            return flag;
        parameters = parameters->prev;
    }
    if (l->nodetype == ARGUMENT)
    {
        if (strcmp(parameters->type, l->type))
        {
            printf("%s %s : %s %s", parameters->type, parameters->name, l->type, l->varname);
            return 1;
        }
    }
    if (parameters == NULL)
        return 0;
    return 2;
}
int arguementcheck2(struct parameter *parameter1, struct symboltable *symboltable1)
{

    while (parameter1 || symboltable1)
    {
        if (symboltable1)
        {
            if (strcmp(symboltable1->name, "self") == 0)
                symboltable1 = symboltable1->prev;
        }
        if ((parameter1 == NULL) && (symboltable1 != NULL))
        {
            printf("para : ( %s )\n", symboltable1->name);
            printf("mismatch_type : 2");
            return 1;
        }
        if ((symboltable1 == NULL) && (parameter1 != NULL))
        {
            printf("para : ( %s )\n", parameter1->name);
            printf(" mismatch_type :  1");
            return 1;
        }
        if (symboltable1)
        {
            if (symboltable1->type)
            {
                if (strcmp(parameter1->type, symboltable1->type))
                {
                    printf("%s %s", parameter1->type, symboltable1->type);
                    return 1;
                }
            }
            else
            {
                if (strcmp(parameter1->type, symboltable1->ctype))
                {
                    printf("%s %s", parameter1->type, symboltable1->type);
                    return 1;
                }
            }

            parameter1 = parameter1->prev;
            symboltable1 = symboltable1->prev;
        }
    }
    return 0;
}
int is_parent(struct Classtable *left, struct Classtable *right)
{
    while (right)
    {
        if (strcmp(right->Name, left->Name) == 0)
            return 1;
        right = right->Parentptr;
    }
    return 0;
}

void help_viewtypetable()
{
    struct Typetable *TABLE = TYPE_TABLE;
    struct Fieldlist *temp_field;
    printf("\n Type Table \n");
    while (TABLE)
    {
        printf("\n    %s size : %d ", TABLE->name, TABLE->size);
        temp_field = TABLE->fields;
        if (temp_field)
        {
            printf("\n     fields : \n");
            while (temp_field)
            {
                printf("         %s index %d \n", temp_field->name, temp_field->fieldIndex);
                temp_field = temp_field->next;
            }
        }
        TABLE = TABLE->next;
    }
    printf("END\n");
}
void help_viewclasstable()
{
    struct Classtable *TABLE = CLASS_TABLE;
    struct Memberfunclist *method_temp;
    struct Fieldlist *field_temp;
    printf("\n class Table \n");
    while (TABLE)
    {
        printf("\n %s index %d \n", TABLE->Name, TABLE->Class_index);
        field_temp = TABLE->Memberfield;
        method_temp = TABLE->Vfuncptr;
        printf("\n      Fields \n");
        while (field_temp)
        {
            printf("    %s %s\n", field_temp->type->name, field_temp->name);
            field_temp = field_temp->next;
        }
        printf("\n      Methods \n");
        while (method_temp)
        {
            // printf("    %s %s  label %d position %d\n", method_temp->Type->name, method_temp->Name, method_temp->Flabel, method_temp->Funcposition);
            method_temp = method_temp->Next;
        }

        TABLE = TABLE->Next;
    }
    printf("END\n\n");
}
void help_addbreak(FILE *targetfile)
{
    fprintf(targetfile, "   BRKP");
    pos++;
}
void TypeTableCreate()
{

    TYPE_TABLE = TInstall("int", 1, NULL, TYPE_TABLE);
    TYPE_TABLE = TInstall("bool", 1, NULL, TYPE_TABLE);
    TYPE_TABLE = TInstall("str", 1, NULL, TYPE_TABLE);
}
struct Typetable *TInstall(char *name, int size, struct Fieldlist *fields, struct Typetable *Typetable)
{
    struct Typetable *temp, *temp_table = (struct Typetable *)malloc(sizeof(struct Typetable));
    temp = Typetable;

    temp_table->name = name;
    temp_table->size = size;
    temp_table->fields = fields;
    temp_table->next = NULL;
    if (temp != NULL)
    {
        while (temp->next)
        {

            if (strcmp(temp->next->name, name) == 0)
            {
                printf("ERROR type redeclared %s", name);
                exit(0);
            }
            temp = temp->next;
        }
        temp->next = temp_table;
    }
    else
        Typetable = temp_table;
    return Typetable;
}
struct Typetable *TLookup(char *name)
{
    struct Typetable *temp_table = TYPE_TABLE;
    while (temp_table)
    {
        if (strcmp(temp_table->name, name) == 0)
            return temp_table;
        temp_table = temp_table->next;
    }
    return NULL;
}
struct Fieldlist *Type_FInstall(char *name, char *type, int index, struct Fieldlist *fieldtable, char *currenttype)
{
    struct Fieldlist *temp, *temp_field = (struct Fieldlist *)malloc(sizeof(struct Fieldlist));
    temp_field->name = strdup(name);
    temp_field->fieldIndex = index;
    temp_field->type = TLookup(type);
    temp_field->typename = strdup(type);
    if (temp_field->type == NULL)
    {
        if (strcmp(currenttype, type))
        {
            printf("ERROR TYPE not found %s", type);
            exit(0);
        }
    }
    temp = fieldtable;
    if (temp != NULL)
    {
        if (strcmp(temp->name, name) == 0)
        {
            printf("TERROR : varible redeclared : %s \n", name);
            exit(0);
        }
        while (temp->next)
        {
            if (strcmp(temp->next->name, name) == 0)
            {
                printf("TERROR : varible redeclared : %s \n", name);
                exit(0);
            }
            temp = temp->next;
        }
        temp->next = temp_field;
    }
    else
    {
        fieldtable = temp_field;
    }
    return fieldtable;
}
struct Fieldlist *FLookup(struct Typetable *type, char *name)
{
    if (type == NULL)
        return NULL;
    struct Fieldlist *temp_field;
    temp_field = type->fields;
    while (temp_field)
    {
        if (strcmp(name, temp_field->name) == 0)
            return temp_field;
        temp_field = temp_field->next;
    }
    return NULL;
}
int GetSize(struct Typetable *type)
{
    return type->size;
}

struct Classtable *CInstall(char *name, char *parent_class_name)
{
    struct Classtable *temp;
    temp = (struct Classtable *)malloc(sizeof(struct Classtable));
    temp->Next = NULL;
    temp->Methodcount = 0;
    temp->Fieldcount = 0;
    temp->Class_index = Class_index;
    Class_index++;
    temp->Name = strdup(name);
    if (parent_class_name)
    {
        temp->Parentptr = CLookup(parent_class_name);
        if (!temp->Parentptr)
        {
            printf("CERROR parent class not declared :%s extends %s", name, parent_class_name);
            exit(0);
        }
        temp->Memberfield = temp->Parentptr->Memberfield;
        temp->Vfuncptr = Class_func_copy(temp->Parentptr->Vfuncptr);
        temp->Methodcount = temp->Parentptr->Methodcount;
        temp->Fieldcount = temp->Parentptr->Fieldcount;
    }
    return temp;
}
struct Classtable *CLookup(char *name)
{
    //  printf("CLOOKUP  (%s)\n", name);
    if (name == NULL)
        return NULL;
    struct Classtable *temp = CLASS_TABLE;
    while (temp)
    {
        //  printf("name : %s\n", temp->Name);
        if (strcmp(temp->Name, name) == 0)
            return temp;
        temp = temp->Next;
    }
    return NULL;
}
void Class_Minstall(struct Classtable *cptr, char *name, char *type, struct parameter *Paramlist)
{
    int position = 0, flag = 0;
    struct Memberfunclist *temp, *temp1 = NULL, *temp_prev = NULL, *temp2 = NULL;

    temp = (struct Memberfunclist *)malloc(sizeof(struct Memberfunclist));
    temp->Name = strdup(name);
    if (TLookup(type))
        temp->Type = TLookup(type);
    else
    {
        printf("CERROR Type not identified : %s", type);
        exit(0);
    }
    temp->paramlist = Paramlist;
    temp->Flabel = getlabel();
    if (cptr->Vfuncptr == NULL)
    {
        temp->Funcposition = 0;
        cptr->Vfuncptr = temp;
    }
    else
    {
        temp_prev = NULL;
        if (cptr->Parentptr)
        {
            temp2 = cptr->Parentptr->Vfuncptr;
            temp1 = cptr->Vfuncptr;
            position = temp2->Funcposition;

            while (temp2)
            {
                //printf("%s name %s Name \n", name, temp2->Name);
                if (strcmp(temp2->Name, name) == 0)
                {
                    // printf("name %s label %d", name, temp->Flabel);
                    flag = 1;
                    break;
                }
                position = temp2->Funcposition;
                temp_prev = temp1;
                temp2 = temp2->Next;
                temp1 = temp1->Next;
            }
        }
        if (flag)
        {
            temp->Funcposition = temp1->Funcposition;
            if (temp2->Next)
                temp->Next = temp1->Next;
            else
            {
                temp->Next = NULL;
            }
        }
        else
        {
            temp2 = cptr->Vfuncptr;
            while (temp2)
            {
                temp_prev = temp2;
                temp2 = temp2->Next;
                position = temp_prev->Funcposition;
            }
            temp->Funcposition = position + 1;
        }
        if (temp_prev)
        {
            temp_prev->Next = temp;
        }
        else
        {
            temp->Next = cptr->Vfuncptr->Next;
            cptr->Vfuncptr = temp;
        }
    }
    // printf("name %s label %d", temp->Name, temp->Flabel);
    cptr->Methodcount++;
    if (cptr->Methodcount > 8)
    {
        printf("CERROR method count : %s", cptr->Name);
        exit(0);
    }
}
void Class_Finstall(struct Classtable *cptr, char *typename, char *name)
{
    struct Fieldlist *temp, *temp1;
    struct Classtable *parentclass = cptr->Parentptr;
    if (parentclass != NULL)
    {
        if (Class_Flookup(parentclass, name) != NULL)
        {
            printf("CERROR redeclaration of parent field : %s in %s", name, cptr->Name);
            exit(0);
        }
    }
    temp = (struct Fieldlist *)malloc(sizeof(struct Fieldlist));
    temp->name = strdup(name);
    if (TLookup(typename))
        temp->type = TLookup(typename);
    else if (CLookup(typename))
        temp->Ctype = CLookup(typename);
    else
    {
        printf("CERROR Type not identified : %s", typename);
        exit(0);
    }
    temp->fieldIndex = cptr->Fieldcount;
    cptr->Fieldcount++;
    if (cptr->Fieldcount > 8)
    {
        printf("CERROR Field count : %s", cptr->Name);
        exit(0);
    }
    if (cptr->Memberfield == NULL)
        cptr->Memberfield = temp;
    else
    {
        temp1 = cptr->Memberfield;
        while (temp1->next)
        {
            // printf("%s ", temp1->Name);
            temp1 = temp1->next;
        }
        temp1->next = temp;
    }
}
struct Memberfunclist *Class_Mlookup(struct Classtable *Ctype, char *Name)
{
    struct Memberfunclist *temp;
    temp = Ctype->Vfuncptr;
    while (temp)
    {
        if (strcmp(temp->Name, Name) == 0)
        {
            return temp;
        }
        temp = temp->Next;
    }
    return NULL;
}
struct Fieldlist *Class_Flookup(struct Classtable *Ctype, char *Name)
{
    struct Fieldlist *temp;
    temp = Ctype->Memberfield;
    while (temp)
    {
        if (strcmp(temp->name, Name) == 0)
            return temp;
        temp = temp->next;
    }
    return NULL;
}
void declaration_typeupdate(char *type, struct symboltable *table)
{
    struct symboltable *temptable = table;
    while (temptable)
    {
        if (CLookup(type))
        {
            temptable->size = 2;
            // printf("class");
        }
        temptable->type = strdup(type);
        temptable = temptable->prev;
    }
}
struct symboltable *declaration_addvar(char *name, int size, struct symboltable *table, struct parameter *paramlist, int label, struct symboltable *GTABLE)
{
    struct symboltable *temptable;
    temptable = table;
    while (GTABLE)
    {
        //   printf("%s ", name);
        if (strcmp(GTABLE->name, name) == 0)
        {
            printf("ERROR : variable redeclared %s\n", name);
            exit(0);
        }
        GTABLE = GTABLE->prev;
    }
    while (temptable)
    {
        //  printf("%s ", name);
        if (strcmp(temptable->name, name) == 0)
        {
            printf("ERROR : variable redeclared %s\n", name);
            exit(0);
        }
        temptable = temptable->prev;
    }
    // printf("\n ");
    temptable = (struct symboltable *)malloc(sizeof(struct symboltable));
    temptable->name = strdup(name);
    temptable->size = 1;
    temptable->prev = table;
    temptable->flabel = label;
    temptable->paramlist = paramlist;
    return temptable;
}
struct symboltable *declaration_addentry(struct symboltable *table, struct symboltable *entry)
{
    struct symboltable *temptable = table;
    if (table == NULL)
    {
        table = entry;
    }
    else
    {
        while (temptable->prev)
        {
            temptable = temptable->prev;
        }
        temptable->prev = entry;
    }
    return table;
}
struct Memberfunclist *Class_func_copy(struct Memberfunclist *func_ptr)
{
    if (!func_ptr)
        return NULL;
    struct Memberfunclist *ptr, *temp = NULL;
    ptr = (struct Memberfunclist *)malloc(sizeof(struct Memberfunclist));
    temp = ptr;
    temp->Name = func_ptr->Name;
    temp->Type = func_ptr->Type;
    temp->Flabel = func_ptr->Flabel;
    temp->Funcposition = func_ptr->Funcposition;
    temp->paramlist = func_ptr->paramlist;

    while (func_ptr->Next)
    {
        temp->Next = (struct Memberfunclist *)malloc(sizeof(struct Memberfunclist));
        temp->Next->Name = func_ptr->Next->Name;
        temp->Next->Type = func_ptr->Next->Type;
        temp->Next->Flabel = func_ptr->Next->Flabel;
        temp->Next->Funcposition = func_ptr->Next->Funcposition;
        temp->Next->paramlist = func_ptr->Next->paramlist;
        func_ptr = func_ptr->Next;
        temp = temp->Next;
    }
    return ptr;
}
void create_virtual_class_table(FILE *targetfile)
{
    struct Classtable *temp_class = CLASS_TABLE;
    struct Memberfunclist *temp_fuclist;
    int temp_mem;
    temp_mem = memory_address;
    while (temp_class)
    {
        temp_fuclist = temp_class->Vfuncptr;
        while (temp_fuclist)
        {

            fprintf(targetfile, "MOV [%d],L%d \n", temp_mem + temp_fuclist->Funcposition, temp_fuclist->Flabel);
            pos++;
            temp_fuclist = temp_fuclist->Next;
        }
        temp_mem = temp_mem + 8;
        temp_class = temp_class->Next;
    }
}