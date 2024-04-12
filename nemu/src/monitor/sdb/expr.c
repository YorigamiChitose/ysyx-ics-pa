/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include "memory/paddr.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ,
  TK_MINUS, TK_PLUS,
  TK_TIMES, TK_DIVIDE,
  TK_REMAINDER,
  TK_NUM, TK_HEXNUM,
  TK_PAR_S, TK_PAR_E,
  TK_REGISTER, TK_MINUS_ONLY,
  TK_TIMES_ONLY,TK_NOTEQ,
  TK_LESS_EQ,TK_BIGGER_EQ,
  TK_LEFTMOVE,TK_RIGHTMOVE,
  TK_LESS,TK_BIGGER,
  TK_AND,TK_OR,TK_NOT,
  TK_CSR, TK_PC,
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {
  {"0[x,X][0-9,a-f,A-F]+", TK_HEXNUM},              // hex number
  {"\\$[0-9,\\$,a-z][a-z,0-9]{0,2}", TK_REGISTER},  // register
  {"[0-9]+", TK_NUM},                               // number
  {" +", TK_NOTYPE},                                // spaces
  {"\\+", TK_PLUS},                                 // '+'
  {"\\-", TK_MINUS},                                // '-'
  {"\\*", TK_TIMES},                                // '*'
  {"\\/", TK_DIVIDE},                               // '/'
  {"\\%", TK_REMAINDER},                            // '%'
  {"\\(", TK_PAR_S},                                // '('
  {"\\)", TK_PAR_E},                                // ')'
  {" +", TK_NOTYPE},                                // ' '
  {"==", TK_EQ},                                    // '=='
  {"!=", TK_NOTEQ},                                 // '!='
  {"<=", TK_LESS_EQ},						                    // '<='
	{">=", TK_BIGGER_EQ},						                  // '>='
  {"<<", TK_LEFTMOVE},						                  // '<<'
	{">>", TK_RIGHTMOVE},						                  // '>>'
  {"<", TK_LESS},						                        // '<'
	{">", TK_BIGGER},						                      // '>'
  {"&&", TK_AND},                                   // '&&'
  {"\\|\\|", TK_OR},                                // '||'
  {"!", TK_NOT},                                    // '!'
  {"m[a-z]+", TK_CSR},                              // csr
  {"pc", TK_PC},                                    // csr
  {"==", TK_EQ},                                    // equal
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

#define TOKENS_SIZE 64
#define TOKENSTR_SIZE 64
typedef struct token {
  int type;
  char str[TOKENSTR_SIZE];
} Token;

static Token tokens[TOKENS_SIZE] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

bool is_only(int addr);
int get_operator_priority(int operator_type);
int cut_expression(int start, int end);
bool pair_first_bracket(int start, int end);

bool is_only(int addr);
int get_operator_priority(int operator_type);
int cut_expression(int start, int end);
bool pair_first_bracket(int start, int end);

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;
  memset(tokens, 0, sizeof(tokens));
  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        // Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
        //     i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_DIVIDE: case TK_PLUS:    case TK_NUM:    
          case TK_HEXNUM: case TK_REGISTER: case TK_PAR_S:  
          case TK_PAR_E: case TK_EQ: case TK_NOTEQ:
          case TK_LESS_EQ: case TK_BIGGER_EQ:case TK_LEFTMOVE:
          case TK_RIGHTMOVE: case TK_LESS: case TK_BIGGER:
          case TK_AND:case TK_OR:case TK_NOT: case TK_REMAINDER:
          case TK_CSR: case TK_PC:
            tokens[nr_token].type = rules[i].token_type;
            strncpy(tokens[nr_token++].str,substr_start,substr_len);
            break;
          case TK_TIMES:
            if (is_only(nr_token)) {
              tokens[nr_token].type = TK_TIMES_ONLY;
              strncpy(tokens[nr_token++].str, substr_start, substr_len);
            }
            else {
              tokens[nr_token].type = rules[i].token_type;
              strncpy(tokens[nr_token++].str,substr_start,substr_len);
            }
            break;
          case TK_MINUS:
            if (is_only(nr_token)) {
              tokens[nr_token].type = TK_MINUS_ONLY;
              strncpy(tokens[nr_token++].str, substr_start, substr_len);
            }
            else {
              tokens[nr_token].type = rules[i].token_type;
              strncpy(tokens[nr_token++].str,substr_start,substr_len);
            }
            break;
          default: break;
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

word_t expression_value(int start,int end) {
  if (start > end) {
    Log("Error! Expression position error.");
    assert(0);
  }
  int cut_add = cut_expression(start, end);
  if (pair_first_bracket(start, end)) {
    return expression_value(start + 1, end - 1);
  }
  else if (start == end) {
    word_t answer = 0;
    bool flag = true;
    switch (tokens[start].type) {
      case TK_NUM:
        answer = atoi(tokens[start].str);
        return answer;
      case TK_HEXNUM:
        sscanf(tokens[start].str, FMT_WORD, &answer);
        return answer;
      case TK_REGISTER:
        answer =  isa_reg_str2val(tokens[start].str + 1,&flag);
        if (flag) {
          return answer;
        }
        else {
          assert(0);
        }
        break;
      case TK_CSR:
        if (!strcmp(tokens[start].str, "mepc")) {return cpu.csr.mepc;}
        if (!strcmp(tokens[start].str, "mcause")) {return cpu.csr.mcause;}
        if (!strcmp(tokens[start].str, "mstatus")) {return cpu.csr.mstatus;}
        break;
      case TK_PC:
        return cpu.pc;
        break;
      default:assert(0);
    }
  }
  else {
    switch (tokens[cut_add].type) {
      case TK_MINUS_ONLY: return (-1) * expression_value(cut_add + 1, end);break;
      case TK_TIMES_ONLY: return paddr_read(expression_value(cut_add + 1, end), 4);break;
      case TK_NOT: return !expression_value(cut_add + 1, end);break;

      case TK_PLUS: return expression_value(start, cut_add - 1) + expression_value(cut_add + 1, end);break;
      case TK_MINUS: return expression_value(start, cut_add - 1) - expression_value(cut_add + 1, end);break;
      case TK_TIMES: return expression_value(start, cut_add - 1) * expression_value(cut_add + 1, end);break;
      case TK_REMAINDER:
        Assert(expression_value(cut_add + 1, end) != 0,"Error! Number division by zero.");
        return expression_value(start, cut_add - 1) % expression_value(cut_add + 1, end);
        break;
      case TK_DIVIDE: 
        Assert(expression_value(cut_add + 1, end) != 0,"Error! Number division by zero.");
        return expression_value(start, cut_add - 1) / expression_value(cut_add + 1, end);
        break;
      case TK_BIGGER: return (expression_value(start, cut_add - 1) > expression_value(cut_add + 1, end));break;
      case TK_LESS: return (expression_value(start, cut_add - 1) < expression_value(cut_add + 1, end));break;
      case TK_BIGGER_EQ: return (expression_value(start, cut_add - 1) >= expression_value(cut_add + 1, end));break;
      case TK_LESS_EQ: return (expression_value(start, cut_add - 1) <= expression_value(cut_add + 1, end));break;
      case TK_RIGHTMOVE: return (expression_value(start, cut_add - 1) >> expression_value(cut_add + 1, end));break;
      case TK_LEFTMOVE: return (expression_value(start, cut_add - 1) << expression_value(cut_add + 1, end));break;
      case TK_NOTEQ: return (expression_value(start, cut_add - 1) != expression_value(cut_add + 1, end));break;
      case TK_EQ: return (expression_value(start, cut_add - 1) == expression_value(cut_add + 1, end));break;
      case TK_AND: return (expression_value(start, cut_add - 1) && expression_value(cut_add + 1, end));break;
      case TK_OR: return (expression_value(start, cut_add - 1) || expression_value(cut_add + 1, end));break;
      default: assert(0);
    }
  }
  assert(0);
}

bool is_only(int addr) {
  if (addr == 0) {return true;}
  switch (tokens[addr - 1].type) {
    case TK_MINUS:case TK_PLUS:case TK_TIMES:
    case TK_DIVIDE:case TK_PAR_S:case TK_MINUS_ONLY:
    case TK_TIMES_ONLY: return true;
    default: return false;
  }
}

int get_operator_priority(int operator_type) {
  switch (operator_type) {
    case TK_MINUS_ONLY: case TK_TIMES_ONLY:
      return 8;
    case TK_TIMES: case TK_DIVIDE: case TK_REMAINDER:
      return 7;
    case TK_PLUS: case TK_MINUS:
      return 6; 
    case TK_LEFTMOVE: case TK_RIGHTMOVE:
      return 5;
    case TK_LESS_EQ: case TK_BIGGER_EQ:
    case TK_LESS: case TK_BIGGER:
      return 4;
    case TK_NOTEQ: case TK_EQ:
      return 3;
    case TK_AND: 
      return 2;
    case TK_OR:
      return 1;
  }
  return 0xff;
}

int cut_expression(int start, int end) {
  int cnt = 0;
  int add = -1;
  for (int i = start; i<= end; i++) {
    switch (tokens[i].type) {
      case TK_NUM: case TK_HEXNUM: case TK_REGISTER: case TK_CSR: case TK_PC: break;
      case TK_PAR_S:++cnt;break;
      case TK_PAR_E:--cnt;break;
      default:
        if(get_operator_priority(tokens[i].type) <= get_operator_priority(tokens[add].type) && cnt == 0) {
          if (!(get_operator_priority(tokens[i].type) == 8 && get_operator_priority(tokens[add].type) == 8)) {
            add = i;
          }
        }
        break;
    }
  }
  return add;
}

bool pair_first_bracket(int start, int end) {
  if (tokens[start].type != TK_PAR_S || tokens[end].type != TK_PAR_E) {
    return false;
  }
  int cnt = 0;
  for (int i = start; i <= end; i++) {
    switch (tokens[i].type) {
      case TK_PAR_S:++cnt;break;
      case TK_PAR_E:--cnt;if (cnt == 0 && i != end) {return false;}break;
      default:break;
    }
  }
  return true;
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  TODO();

  return 0;
}
