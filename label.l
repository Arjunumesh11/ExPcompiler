%option noyywrap
%{
#include <stdlib.h>
#include <stdio.h>
#include "exprtree.h"
#include <string.h>
FILE *outfile,*label_file;
char name[5];
int i=0;
struct labeltable table[No_labels]; 
%}
%%
[L][0-9]+[:]"\n" {;}
[L][0-9]+ {
            for(int j=0;j<No_labels;j++)
            {
                if(strcmp(table[j].name,yytext)==0)
                {
                    printf("%s ",table[j].name);
                    printf("%d ",table[j].address);
                    fprintf(outfile,"%d",table[j].address);
                }
            }
         }
.        {fprintf(outfile,"%c",yytext[0]);}
"\n"      {fprintf(outfile,"\n");}
%%
int main()
{
   
   label_file=fopen("label.dat","r");
    while(fread(&table[i],sizeof(struct labeltable),1,label_file))
    {
      // printf("%d ",table[i].address);
     //  printf("%s ",table[i].name);
        i++;
    }
   outfile=fopen("output.xsm","w");
   yyin=fopen("output.txt","r");
   yylex();
}