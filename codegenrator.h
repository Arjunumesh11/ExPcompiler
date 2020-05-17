#pragma once
#include <string.h>

FILE *label_file;
int reg[20];
extern int labels;
extern int pos;         //position of commands
extern int memlocation; //current memory location
extern int localmem;
extern int paramem;
extern int declflag, decltypeflag;
extern int Class_index;

#define INTE "int"
#define BOLE "bool"
#define STRE "str"
#define NULL1 "NULL"
#define FUNCTION 4

#define NUMBER 1
#define VARIABLE 2
#define READ0 3
#define WRITE0 4
#define OPERATOR 5
#define CONNECTOR 6
#define POWER 7
#define IFST 8
#define WHILEST 9
#define BREAKST 10
#define CONTINUEST 11
#define DECLARATION 12  //local
#define GDECLARATION 13 //global
#define PDECLARATION 14 //parameter
#define STRI 15
#define RETURNST 16
#define FUNCALL 17
#define ARGUMENT 18
#define FUNCDEF 19
#define FIELD 20
#define ALOC 21
#define INIT 22
#define CFUNCALL 23
#define BREAKP 24
#define NEW0 25
#define DEALOC 26

#define No_labels 100
#define start_adress 2102
struct parameter
{
    char *type;
    char *name;
    struct parameter *prev;
};
struct symboltable
{
    char *name;
    char *type;
    char *ctype;
    int size;
    int binding;
    struct parameter *paramlist;
    int flabel;
    struct symboltable *prev;
};
struct symboltablelist
{
    struct symboltable *val;
    struct symboltablelist *prev;
};
struct Typetable
{
    char *name;               //type name
    int size;                 //size of the type
    struct Fieldlist *fields; //pointer to the head of fields list
    struct Typetable *next;   // pointer to the next type table entry
};
struct Classtable
{
    char *Name;                      //name of the class
    struct Fieldlist *Memberfield;   //pointer to Fieldlist
    struct Memberfunclist *Vfuncptr; //pointer to Memberfunclist
    struct Classtable *Parentptr;    //pointer to the parent's class table
    int Class_index;                 //position of the class in the virtual function table
    int Fieldcount;                  //count of fields
    int Methodcount;                 //count of methods
    struct Classtable *Next;         //pointer to next class table entry
};
struct Fieldlist
{
    char *name; //name of the field
    char *typename;
    int fieldIndex;           //position of the field
    struct Typetable *type;   //pointer to typetable
    struct Classtable *Ctype; //pointer to the class containing the field
    struct Fieldlist *next;   //pointer to next fieldlist entry
};
struct Memberfunclist
{
    char *Name;                  //name of the member function in the class
    struct Typetable *Type;      //pointer to typetable
    struct parameter *paramlist; //pointer to the head of the formal parameter list
    int Funcposition;            //position of the function in the class table
    int Flabel;                  //A label for identifying the starting address of the function's code in the memory
    struct Memberfunclist *Next; //pointer to next Memberfunclist entry
};

typedef struct tnode
{
    int val;
    char *type;
    char *varname;
    int nodetype;
    char *op;
    struct tnode *left, *right;
    struct symboltable *Gsymbol;
    struct Classtable *cptr;
} tnode;

struct labeltable
{
    int address;
    char name[5];
};

struct loop_counter
{
    char break_label[5];
    char continue_label[5];
    struct loop_counter *prev;
};
struct Reg_counter
{
    int *Reg;
    struct Reg_counter *prev;
};
struct symboltable *LLookup(char *name);          //search and return var info
struct symboltable *GLookup(char *name);          //search and return var info
struct symboltable *PLookup(char *name);          //search and return var info
void symbolinit(struct symboltable *SymbolTable); //intialise symbol table
struct tnode *CreateTree(int val, char *type, char *varname, int nodetype, char *op, tnode *l, tnode *r, struct symboltable *table);
struct tnode *makePowerNode(struct tnode *l);

void TypeTableCreate();                                                                                  //Function to initialise the type table entries with primitive types
struct Typetable *TLookup(char *name);                                                                   //Search through the type table and return pointer to type table entry of type 'name'.
struct Typetable *TInstall(char *name, int size, struct Fieldlist *fields, struct Typetable *Typetable); //Creates a type table entry for the (user defined) type of 'name' with given 'fields' and returns the pointer to the type table entry
struct Fieldlist *Type_FInstall(char *name, char *type, int index, struct Fieldlist *fieldtable, char *currenttype);
struct Fieldlist *FLookup(struct Typetable *type, char *name); // Searches for a field of given 'name' in the 'fieldlist' of the given user-defined type and returns a pointer to the field entry
int GetSize(struct Typetable *type);                           // Returns the amount of memory words required to store a variable of the given type.

struct Classtable *CInstall(char *name, char *parent_class_name);                                  // Creates a class table entry of given 'name' and extends the fields and the methods of parent class and returns a pointer to the newly created class entry.
struct Classtable *CLookup(char *name);                                                            // Search for a class table entry with the given 'name', if exists, return pointer to class table entry else return NULL.
void Class_Finstall(struct Classtable *cptr, char *typename, char *name);                          // Installs the field into the given class table entry which is given as an argument.
void Class_Minstall(struct Classtable *cptr, char *name, char *type, struct parameter *Paramlist); // Installs the method into the given class table entry which is given as an argument.
struct Memberfunclist *Class_Mlookup(struct Classtable *Ctype, char *Name);                        // Search through the VFunclist of the class using Ctype that is being parsed and return pointer to the entry in the list with function name as Name. Returns NULL if entry is not found.
struct Fieldlist *Class_Flookup(struct Classtable *Ctype, char *Name);                             // Search through the Memberfield of the current class using Ctype that is being parsed and return pointer to the entry in the list with variable name as Name. Returns NULL if entry is not found
struct Memberfunclist *Class_func_copy(struct Memberfunclist *func_ptr);

int yyerror(char const *s);
int min(int a, int b);
int max(int a, int b);
int getReg(int *Reg);
void freeReg(int *Reg);
void pushReg(int *Reg, FILE *targetfile);
void popReg(int *Reg, FILE *targetfile);
void popArgument(struct parameter *paramlist, FILE *targetfile);
int getlabel();
int codeGen(struct tnode *t, FILE *targetfile, int option); //option 1 = value 0 = return adress
int evaluate(struct tnode *t);
int Gallocatemem(int n);
int Pallocatemem(int n);
int Lallocatemem(int n, FILE *targetfile);

void help_viewtable(struct symboltable *table, int scope);
void help_viewReg(int *Reg);
void help_addbreak();
int arguementcheck(struct parameter *parameters, struct tnode *l);
int arguementcheck2(struct parameter *parameter1, struct symboltable *parameter2);
int is_parent(struct Classtable *left, struct Classtable *right);
void help_viewtypetable();
void help_viewclasstable();
void create_virtual_class_table(FILE *targetfile); //to create virtual classtabe

void declaration_typeupdate(char *type, struct symboltable *table);
struct symboltable *declaration_addvar(char *name, int size, struct symboltable *table, struct parameter *paramlist, int label, struct symboltable *GTABLE);
struct symboltable *declaration_addentry(struct symboltable *table, struct symboltable *entry);

extern struct labeltable LabelTable[No_labels];
extern struct loop_counter *LOOP_COUNTER_HEAD, *LOOP_COUNTER_TEMP;
extern struct symboltable *Symbol_Table, *temptable, *GLOBAL_TABLE;
extern struct parameter *Temp_Parameter, *V_Parameter;
extern struct Reg_counter *REG_COUNTER;

extern struct Typetable *TYPE_TABLE;
extern struct Typetable *temptypetable;
extern struct Fieldlist *tempfieldlist;

extern struct Classtable *CLASS_TABLE, *Cptr, *Current_class;
