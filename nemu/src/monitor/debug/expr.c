#include "nemu.h"
/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <elf.h>




// 实现算术表达式的词法分析
//
// 想要求出表达式的值，第一步要解决的问题是识别字符串中的数字、符号、括号等等，
// 解决方法是利用正则表达式刻画字符的组合规律，将字符串切割成一个个的有确定类型的 token 。
// 
// 表达式中可能出现的类型：
// 	① 数字：十进制 ，十六进制 …
// 	② 运算符：+，-，*，/，（…
// 	③ 符号：test_case，…
// 	④ 寄存器：$ eax，$ edx，…
// 
// 利用正则表达式的规则补充 rules[]，其中要特别注意，如果识别的符号为正则表达式的元符号则需要加上 \ 符号。
// 
// 扩充完正则表达式规则以后，需要做的就是对输入的字符串进行分析，对每一个符号进行分类，
// 再将各个类型存储在 tokens[] 数组中，完成此操作的函数为 make_token() 函数。
// 已给出代码的部分可以成功识别得到该字符或者字符串的对应规则，而我们需要补充的部分是 switch 语句，
// switch 语句将表达式中每一个部分用对应的类型及具体值存储到 tokens[nr_token].str 中
//（如NUM类型里存具体的数字，RESITER类型里存具体的寄存器的名字等等），
// 此过程中用到了strncpy()函数来复制具体的值（如NUM类型的具体值等等）。
//
//
// 


enum {
	NOTYPE = 256, EQ, NUMBER, HEXNUMBER, REGISTER, NEQ, AND, OR, NOT, MINUS, POINTER, MARK

	/* TODO: Add more token types */

};

static struct rule {
	char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */
	{" +",	NOTYPE},				// spaces
	{"\\+", '+'},					// plus
	{"==", EQ},						// equal
	{"!=", NEQ},
	{"!", NOT},
	{"&&", AND},
	{"\\|\\|", OR},
	{"\\$[a-zA-z]+", REGISTER},		// register
	{"\\b0[xX][0-9a-fA-F]+\\b", HEXNUMBER}, // hexnumber
	{"\\b[0-9]+\\b", NUMBER},				// number
	{"-", '-'},						// minus
	{"\\*", '*'},					// multiply
	{"/", '/'},						// divide
	{"\\(", '('},					// left bracket
	{"\\)", ')'},					// right bracket
	{"\\b[a-zA-Z0-9_]+\\b",MARK},
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

uint32_t GetMarkValue(char *str,bool *success);

static regex_t re[NR_REGEX];

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
			Assert(ret == 0, "regex compilation failed: %s\n%s", error_msg, rules[i].regex);
		}
	}
}

typedef struct token {
	int type;
	char str[32];
} Token;

Token tokens[32];
int nr_token;
bool* cando;
static bool make_token(char *e) {
	int position = 0;
	int i;
	regmatch_t pmatch;

	nr_token = 0;

	while (e[position] != '\0') {
		/* Try all rules one by one. */
		for (i = 0; i < NR_REGEX; i ++) {
			if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

//				Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);

				position += substr_len;
				/* TODO: Now a new token is recognized with rules[i]. Add codes
				 * to record the token in the array `tokens'. For certain types
				 * of tokens, some extra actions should be performed.
				 */

				switch (rules[i].token_type) {
				case NOTYPE: break;
				default: {
					tokens[nr_token].type = rules[i].token_type;
					if (rules[i].token_type == REGISTER) { //Register
						char* reg_start = e + (position - substr_len) + 1;
						strncpy(tokens[nr_token].str, reg_start, substr_len - 1);
						int t;
						for (t = 0; t <= strlen(tokens[nr_token].str); t++) { // tolower
							int x = tokens[nr_token].str[t];
							if (x >= 'A' && x <= 'Z') x += ('a' - 'A');
							tokens[nr_token].str[t] = (char)x;
						}
					} else
						strncpy(tokens[nr_token].str, substr_start, substr_len);

					tokens[nr_token].str[substr_len] = '\0';

					//						printf("%s\n", tokens[nr_token].str);
					nr_token ++;
				}
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

bool check_bracket(int l, int r) {
	if (tokens[l].type != '(' || tokens[r].type != ')') return false;
	int b_num = 0, i;
	for (i = l; i <= r; i ++) {
		if (tokens[i].type == '(') b_num ++;
		if (tokens[i].type == ')') b_num --;
		if (b_num == 0 && i != r) return false;
	}
	if (b_num == 0) return true;
	return false;
}

int dominant_op(int l, int r) {
	if (*cando == false) return 0;
	int i;
	int pos = l;
	int pri = 0;
	int b_num = 0;
	for (i = r; i >= l; i --) {
		if (tokens[i].type == ')') b_num++;
		if (tokens[i].type == '(') b_num--;
		if (b_num != 0) continue;
		switch (tokens[i].type) {
		case '+': { // pri = 4
			if (pri < 4) pos = i, pri = 4;
			break;
		}
		case '-': { // pri = 4
			if (pri < 4) pos = i, pri = 4;
			break;
		}
		case '*': { // pri = 3
			if (pri < 3) pos = i, pri = 3;
			break;
		}
		case '/': { // pri = 3
			if (pri < 3) pos = i, pri = 3;
			break;
		}
		case NOT: { // pri = 2
			if (pri < 2) pos = i, pri = 2;
			break;
		}
		case EQ: { // pri = 7
			if (pri < 7) pos = i, pri = 7;
			break;
		}
		case NEQ: { // pri = 7
			if (pri < 7) pos = i, pri = 7;
			break;
		}
		case AND: { // pri = 11
			if (pri < 11) pos = i, pri = 11;
			break;
		}
		case OR: { // pri = 12
			if (pri < 12) pos = i, pri = 12;
			break;
		}
		case MINUS: { // pri = 2
			if (pri < 2) pos = i, pri = 2;
			break;
		}
		case POINTER: { // pri = 2
			if (pri < 2) pos = i, pri = 2;
			break;
		}
		default: break;
		}
	}
//	printf("%d-%d %d %d\n",l,r,pos,pri);
	if (pri == 0) {*cando = false; return 0;}

	if (pri == 2) {
		pri = 0;
		for (i = l; i <= r; i ++) {
			if (tokens[i].type == '(') b_num++;
			if (tokens[i].type == ')') b_num--;
			if (b_num != 0) continue;
			switch (tokens[i].type) {
			case MINUS: { // pri = 2
				if (pri < 2) pos = i, pri = 2;
				break;
			}
			case NOT: { // pri = 2
				if (pri < 2) pos = i, pri = 2;
				break;
			}
			case POINTER: { // pri = 2
				if (pri < 2) pos = i, pri = 2;
				break;
			}
			}
		}
	}
	return pos;
}

// eval是一个递归函数，用来求值，其中若p>q则显然不符合，直接跳出循环；若p==q则说明该位置应当是一个数值，直接判断tokens[p].type类型然后用sscanf转成数字返回就行。
// 那么对于p<q，又有两种情况，先要进行括号判断，即如果当前p到q区间类似于(1+2)情况，需要将括号去掉，取中间1+2递归计算值；如果不满足，则需要找到“主符号”（dominant operator）。
// 对于主符号的定义，你可以理解为是从右往左出现的第一个优先级最低的符号，因为它需要最后计算。比如1+2*3-4的主符号是-。由于在这个阶段只需要实现四则运算，所以优先级的概念可以模糊，只要认为+和-优先级低于*和/就行。
// 找到之后递归求出左右区间的值，再用switch进行计算值即可，具体框架已经在指导书中已经写得十分详细了。
// 需要注意，在check_parentheses()函数中不要把形如(1+2)*(3+4)的两边的括号匹配上，否则就会出错！
// 
// Tip：check_parentheses()的实现利用循环从左往右遍历，进行左括号计数，遇到左括号加一，右括号减一，来判断两者是否匹配。
// 最后记得修改cmd_p使其可以计算表达式！
uint32_t eval(int l, int r) {
	if (*cando == false) return 0;
	if (l > r) {
		*cando = false;
		return 0;
	}
	if (l == r) {
		uint32_t num = 0;
		if (tokens[l].type == MARK){
			num = GetMarkValue(tokens[l].str, cando);
			if (*cando == false) return 0;
		} else if (tokens[l].type == NUMBER) {
			sscanf(tokens[l].str, "%d", &num);
		} else if (tokens[l].type == HEXNUMBER) {
			sscanf(tokens[l].str, "%x", &num);
		} else if (tokens[l].type == REGISTER) {
			if (strlen(tokens[l].str) == 3) { //length = 3
				int i;
				for (i = R_EAX; i <= R_EDI ; i ++) {
					if (strcmp(tokens[l].str, regsl[i]) == 0) {
						break;
					}
				}
				if (i > R_EDI) {
					if (strcmp(tokens[l].str, "eip") == 0) {
						num = cpu.eip;
					} else {*cando = false; return 0;}
				} else return num = reg_l(i);
			}
			else if (strlen(tokens[l].str) == 2) { //length = 2
				int i;
				for (i = R_AX; i <= R_DI; i ++) {
					if (strcmp(tokens[l].str, regsw[i]) == 0) {
						break;
					}
				}
				if (i <= R_DI) return num = reg_w(i);
				for (i = R_AL; i <= R_BH; i ++) {
					if (strcmp(tokens[l].str, regsb[i]) == 0) {
						break;
					}
				}
				if (i <= R_BH) return num = reg_b(i);
				*cando = false; return 0;
			}
			else {*cando = false; return 0;}
		} else {*cando = false; return 0;}
		return num;
	}
	uint32_t ans = 0;

	if (check_bracket(l, r)) return eval(l + 1, r - 1);
	else {
		int pos = dominant_op(l, r);
		if (l == pos || tokens[pos].type == NOT || tokens[pos].type == MINUS || tokens[pos].type == POINTER) {
			uint32_t r_ans = eval(pos + 1, r);
			switch (tokens[pos].type) {
			case POINTER: current_sreg = R_DS;return swaddr_read(r_ans, 4);
			case NOT: return !r_ans;
			case MINUS: return -r_ans;
			default: {*cando = false; return 0;}
			}
		}
		uint32_t l_ans = eval(l, pos - 1), r_ans =  eval(pos + 1, r);
		switch (tokens[pos].type) {
		case '+': ans = l_ans + r_ans; break;
		case '-': ans = l_ans - r_ans; break;
		case '*': ans = l_ans * r_ans; break;
		case '/': if (r_ans == 0) {*cando = false; return 0;} else ans = l_ans / r_ans; break;
		case EQ : ans = l_ans == r_ans; break;
		case NEQ: ans = l_ans != r_ans; break;
		case AND: ans = l_ans && r_ans; break;
		case OR : ans = l_ans && r_ans; break;
		default: {*cando = false; return 0;}
		}
	}
	return ans;
}

uint32_t expr(char *e, bool *success) {
	if (!make_token(e)) {
		*success = false;
		return 0;
	}
	cando = success;
	/* TODO: Insert codes to evaluate the expression. */
	/*check whether brackets are matched*/
	int i, brack = 0;
	for (i = 0; i < nr_token ; i++) {
		if (tokens[i].type == '(') brack ++;
		if (tokens[i].type == ')') brack --;
		if (brack < 0) {
			*success = false;
			return 0;
		}
		if (tokens[i].type == '-' && (i == 0 || (tokens[i - 1].type != ')' && tokens[i - 1].type != NUMBER && tokens[i - 1].type != HEXNUMBER && tokens[i - 1].type != REGISTER))) {
			tokens[i].type = MINUS;
		}
		if (tokens[i].type == '*' && (i == 0 || (tokens[i - 1].type != ')' && tokens[i - 1].type != NUMBER && tokens[i - 1].type != HEXNUMBER && tokens[i - 1].type != REGISTER))) {
			tokens[i].type = POINTER;
		}
	}
	if (brack != 0) {
		*success = false;
		return 0;
	}

//	panic("please implement me");
	*success = true;
	return eval(0, nr_token - 1);
}

