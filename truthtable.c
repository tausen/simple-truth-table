/*
 *  #######################################################################
 *                       Simple truth table generator
 *  #######################################################################
 *    
 *  Version history:
 *  Date      Version  Developer       Description
 *  -----------------------------------------------------------------------
 *  21/07-12  0.1      Mathias Tausen  Initial beta release.
 *  
 *  #######################################################################
 *  
 *  This is a simple command-line truth table generator.
 *  
 *  It currently supports 10 independent variables and a total boolean 
 *  expression length of 128 characters, but should be very easy to adjust.
 *  
 *  Place the truthtable.c and Makefile files in the same directory and 
 *  use the "make" command to compile.
 *  As an alternative, execute the following:
 *    gcc truthtable.c -o truthtable -lm
 *  The project only relies on core gcc libraries and should not have any 
 *  extra dependencies.
 *  
 *  Feel free to edit the source code, redistribute, share and improve.
 *  If you do make any improvements, though, please send me an e-mail
 *  and allow me to commit those changes to the project.
 *  
 *  If you have questions, comments or need help, feel free to contact
 *  me at:
 *      mathias.tausen@gmail.com
 *  
 *  Get example syntax:
 *    truthtable
 *  Generate a truth table for a simple boolean expression:
 *    truthtable "(A+B)&(C+~D)+D"
 *  Get the result of a boolean expression with specific symbol values:
 *    truthtable "(A+B)&(C+~D)+D" 0010
 *  Here, A, B and D values 0 and C values 1.
 *  
 *  OR:             +
 *  AND:            &
 *  NOT:            ~
 *  Group:          ( )
 *  Valid symbols:  A-Z and a-z
 *  
 *  Order of evaluation:
 *      Parenthesis, NOT, AND, OR
 *  
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define STRLEN 128
#define NUMVARS 10

typedef struct {
  char symbol;
  unsigned char value;
} boolvar;

boolvar Vars[NUMVARS];
unsigned int CurrNumVars;

char Debug = 0;

void removechar(char* str, int charno) {
  int i, a;
  
  a = 0;
  char new[STRLEN];
  
  for (i = 0; i < strlen(str); i++) {
    if (i < charno) {
      new[a] = str[i];
      a++;
    } else if (i > charno) {
      new[a] = str[i];
      a++;
    }
  }
  
  new[a] = '\0';
  strcpy(str, new);
}

void removespaces(char* str) {
  int i;
  i = 0;
  while (i < strlen(str)) {
    if (str[i] == ' ') {
      removechar(str, i);
    } else {
      i++;
    }
  }
  str[strlen(str)] = '\0';
}

char getbit(char no, char bitno) {
  char retval = (no & (1 << bitno)) >> bitno;
  //printf("no:%x\tbitno:%x\tretval:%x\n",no,bitno,retval);
  return retval;
}

unsigned char getval(char symbol) {
  int i;
  
  for (i = 0; i < CurrNumVars; i++) {
    if (Vars[i].symbol == symbol)
      return Vars[i].value;
  }
  
  return -1;
}

void findchars(char* expr) {
  unsigned int i, len, a;
  len = strlen(expr);
  
  for (i = 0; i < len; i++) {
    // if its a character
    if ((expr[i] >= 65 && expr[i] <= 90) || (expr[i] >= 97 && expr[i] <= 122)) {
      
      char new = 1;
      for (a = 0; a < CurrNumVars; a++) {
        if (Vars[a].symbol == expr[i]) {
          new = 0;
          break;
        }
      }
      
      if (new) {
        Vars[CurrNumVars].symbol = expr[i];
        CurrNumVars++;
      }
      
    }
  }
}

void setvalues(char* expr) {
  int i;
  char len = strlen(expr);
  
  for (i = 0; i < len; i++) {
    if ((expr[i] >= 65 && expr[i] <= 90) || (expr[i] >= 97 && expr[i] <= 122)) {
      if (i > 0 && expr[i-1] == '~') {
        expr[i-1] = ' ';
        expr[i] = getval(expr[i]) == 1 ? '0' : '1';
      } else {
        expr[i] = getval(expr[i]) == 1 ? '1' : '0';
      }
    }
  }
  
  removespaces(expr);
}

// operator: + or &
char calc(char* expr, char operator) {
  
  char* p = expr;
  int i;
  char len = strlen(expr);
  i = 0;
  
  //for (i = 0; i < len; i++) {
  while (i < len) {
    
    if (p[i] == '(') { // if its parenthesis
      
      char* tmp = p + i + 1;
      char tmp2[STRLEN];
      strcpy(tmp2, tmp);
      char* endingpar = strchr(tmp2, ')');
      *endingpar = '\0';
      
      if (Debug)
        printf("found parenthesis, re-running with %s\n",tmp2);
        
      calc(tmp2, '&');
      calc(tmp2, '+');
      
      // take all characters from and including ( to and including ) and replace them with the result
      // of the above call to calc()
      tmp = p + i; // tmp points to (
      while (*tmp != ')') {
        *tmp = ' ';
        tmp++;
      }
      *tmp = ' ';
      
      p[i] = tmp2[0];
      removespaces(p);
      
      i -= 2;
      
    } else if (p[i] == operator) { // if its an operator
      if (Debug)
        printf("found %c\n",p[i]);
      if (p[i+1] == '(' || p[i+1] == '~') {
        i++;
      } else {
        if (p[i] == '+') {
          if (atoi(&p[i-1]) | atoi(&p[i+1])) {
            p[i-1] = '1';
            /*if (Debug)
              printf("%i | %i = 1",atoi(&p[i-1]),atoi(&p[i+1]));*/
          } else {
            p[i-1] = '0';
            /*if (Debug)
              printf("%i | %i = 0",atoi(&p[i-1]),atoi(&p[i+1]));*/
          }
        } else if (p[i] == '&') {
          if (atoi(&p[i-1]) & atoi(&p[i+1])) 
            p[i-1] = '1';
          else
            p[i-1] = '0';
        }
        p[i] = ' ';
        p[i+1] = ' ';
        removespaces(p);
      }
    } else if (p[i] == '~') {
      if (p[i+1] == '(') {
        i++;
      } else {
        if (p[i+1] == '1')
          p[i] = '0';
        else if (p[i+1] == '0')
          p[i] = '1';
        p[i+1] = ' ';
        removespaces(p);
        i--;
      }
    } else {
      i++;
    }
    len = strlen(expr);
    if (Debug)
      printf("now %s [%c]\n",expr,p[i]);
  }
  
  return 0;
}

int main(int argc, char *argv[]) {

  if (argc < 2) {
    printf("Not enough parameters.\n");
    printf(" OR: +\n");
    printf(" AND: &\n");
    printf(" NOT: ~\n");
    printf(" Group: ( )\n");
    printf(" Valid symbols: A-Z, a-z\n");
    printf(" Example usage:  truthtable \"(A&B) + ~C\"\n");
    printf(" Specify values for symbols: truthtable \"(A&B) + ~C\" 010\n");
    return 0;
  }
  
  CurrNumVars = 0;
  char str[STRLEN], org[STRLEN];
  strcpy(str,argv[1]);
  strcpy(org,str);
  removespaces(str);
  findchars(str);
  
  if (argc == 2) {
    printf("Evaluating: %s\n", argv[1]);
    
    unsigned char a;
    unsigned int i;
    
    for (a = 0; a < CurrNumVars; a++) {
      printf("%c\t", Vars[a].symbol);
    }
    printf("R\n\n");
    
    unsigned int numruns = pow(2, CurrNumVars);
    
    for (i = 0; i < numruns; i++) {
      
      // Update Vars values with bit values from i
      for (a = 0; a < CurrNumVars; a++) {
        Vars[a].value = (i & (1 << a)) >> a;
        if (Debug)
          printf("%x",Vars[a].value);
      }
      if (Debug)
        printf("\n");
      
      strcpy(str,org);
      if (Debug)
        printf("Setting values in: %s\n",str);
      setvalues(str);
      if (Debug)
        printf("Is now: %s\n",str);
      
      calc(str, '&');
      calc(str, '+');
      for (a = 0; a < CurrNumVars; a++) {
        printf("%x\t", Vars[a].value);
      }
      printf("%x\n", atoi(str));
    }
  
  } else if (argc > 2) {
    
    char* vals = argv[2];
    int i;
    for (i = 0; i < strlen(vals); i++) {
      Vars[i].value = vals[i] == '0' ? 0 : 1;
    }
    
    setvalues(str);
    calc(str, '&');
    calc(str, '+');
    printf("%x\n", atoi(str));
    
  }
 
  return 0;
}

