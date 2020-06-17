#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <time.h>

// parse expression
#define arraylen(a) (sizeof(a) / sizeof(*(a)))
enum TokenType {
	TK_DUMMY, TK_NUMBER, TK_EOF, TK_NAME,
	TK_PLUS, TK_MINUS, TK_STAR, TK_SLASH,
	TK_LPAREN, TK_RPAREN, TK_POW, TK_COMMA,
};

typedef struct Token {
	enum TokenType type;
	size_t pos;
	union {
		double num;
		const char* str;
	};
} Token;
struct variable {
	const char* name;
	double value;
};

static const char* str;
static size_t str_pos, str_len;
static struct variable vars[10] = {
	{ "PI", 3.1415926 },
	{ "E",  2.7182818 },
	{ NULL, 0 },
};

#define peek() (str_pos < str_len ? str[str_pos  ] : EOF)
#define next() (str_pos < str_len ? str[str_pos++] : EOF)
static Token read_token(void) {
	if (peek() == '\n') return (Token){ TK_EOF, str_pos };
	while (isspace(peek())) next();
	const size_t start = str_pos;
	char ch = peek();
	if (isdigit(ch)) {
		double val = 0.0;
		while (isdigit(peek())) val = val * 10 + (next() - '0');
		if (peek() == '.') {
			int exp = 0;
			next();
			while (isdigit(peek())) val += (next() - '0') * pow(10, --exp);
		}
		if (peek() == 'e') {
			int exp = 0;
			next();
			while (isdigit(peek())) exp = exp * 10 + (next() - '0');
			val = pow(val, exp);
		}
		return (Token){ TK_NUMBER, start, .num = val };
	}
	else if (isalpha(ch)) {
		char* buf = (char*)malloc(11);
		size_t i;
		for (i = 0; i < 10 && isalpha(peek()); ++i)
			buf[i] = next();
		buf[i] = '\0';
		return (Token){ TK_NAME, start, .str = buf };
	}
	else {
		next();
		switch (ch) {
		case '+':	return (Token){ TK_PLUS, start };
		case '-':	return (Token){ TK_MINUS, start };
		case '*':	return (Token){ TK_STAR, start };
		case '/':	return (Token){ TK_SLASH, start };
		case '(':	return (Token){ TK_LPAREN, start };
		case ')':	return (Token){ TK_RPAREN, start };
		case '^':	return (Token){ TK_POW, start };
		case '\0':
		case '\n':
		case EOF:	return (Token){ TK_EOF, start };
		default:
			printf("%d: illegal input: '%c'\n", start + 1, ch);
			exit(1);
		}
	}
}
#undef peek
#undef next

static Token peekd = { 0 };
#define peek() (peekd.type ? peekd : (peekd = read_token()))
static Token next(void) {
	if (peekd.type) {
		const Token tmp = peekd;
		peekd.type = TK_DUMMY;
		return tmp;
	}
	else return read_token();
}
#define matches(t) (peek().type == (t))
#define match(t) (peek().type == (t) ? next(), true : false)

static double parse(void);
static double unary(void);
static double prim(void) {
	if (matches(TK_NUMBER)) return next().num;
	else if (match(TK_LPAREN)) {
		const double v = parse();
		if (!match(TK_RPAREN)) {
			printf("%d: expected )\n", str_pos);
			exit(1);
		}
		else return v;
	}
	else if (matches(TK_NAME)) {
		const Token name = next();
		if (strcmp(name.str, "random") == 0)
			return ((double)rand() / (double)RAND_MAX);
		for (size_t i = 0; vars[i].name; ++i) {
			if (strcmp(name.str, vars[i].name) == 0)
				return vars[i].value;
		}
		const double val = unary();
		if (strcmp(name.str, "sin") == 0)
			return sin(val);
		if (strcmp(name.str, "cos") == 0)
			return cos(val);
		if (strcmp(name.str, "tan") == 0)
			return tan(val);
		else {
			printf("%d: unknown function %s\n", name.pos, name.str);
			exit(1);
		}
	}
	else {
		printf("%d: expected expression\n", str_pos);
		exit(1);
	}
}
static double unary(void) {
	if (match(TK_PLUS)) return unary();
	else if (match(TK_MINUS)) return -unary();
	else return prim();
}
static double power(void) {
	double left = unary();
	if (match(TK_POW)) left = pow(left, power());
	return left;
}
static double multiplication(void) {
	double left = power();
	while (matches(TK_STAR) || matches(TK_SLASH)) {
		const Token op = next();
		const double right = power();
		if (op.type == TK_STAR) left *= right;
		else left /= right;
	}
	return left;
}
static double addition(void) {
	double left = multiplication();
	while (matches(TK_PLUS) || matches(TK_MINUS)) {
		const Token op = next();
		const double right = multiplication();
		if (op.type == TK_PLUS) left += right;
		else left -= right;
	}
	return left;
}
static double parse(void) {
	double left = addition();
	while (!match(TK_EOF) && !matches(TK_RPAREN))
		left *= addition();
	return left;
}
static double parse_expr(const char* s) {
	str = s;
	str_pos = 0;
	str_len = strlen(s);
	return parse();
}

static bool add_var(const char* name, double val) {
	size_t i;
	for (i = 0; i < arraylen(vars) && vars[i].name; ++i);
	if (i >= arraylen(vars) - 1) return false;
	vars[i].name = name;
	vars[i].value = val;
	return true;
}
static bool set_var(const char* name, double val) {
	for (size_t i = 0; i < arraylen(vars); ++i) {
		if (strcmp(name, vars[i].name) == 0) {
			vars[i].value = val;
			return true;
		}
	}
	return false;
}

static void quit(void) {
	SDL_CloseAudio();
	SDL_Quit();
}

static float* normalize(float* a, int len) {
	float m = a[0];
	for (int i = 1; i < len; ++i) {
		if (a[i] > m) m = a[i];
	}
	for (int i = 0; i < len; ++i) a[i] /= m;
	return a;
}
static void my_callback(void* userdata, uint8_t* stream, int len) {
	float* a = (float*)stream;
	for (int i = 0; i < len; ++i) {
		set_var("x", i);
		a[i] = (float)parse_expr((const char*)userdata);
	}
	normalize(a, len);

	/*float* buf = (double*)malloc(sizeof(double)*len);
	for (int i = 0; i < len; ++i) {
		set_var("x", i);
		buf[i] = parse_expr((const char*)userdata);
	}
	normalize(buf, len);
	for (int i = 0; i < len; ++i) stream[i] = (uint8_t)(buf[i] * 255.f);
	free(buf);*/
}

int main(int argc, const char* argv[]) {
	if (SDL_Init(SDL_INIT_AUDIO) > 0) {
		printf("Couldn't initialize SDL2: %s\n", SDL_GetError());
		return 1;
	}
	char buf[400];
	add_var("x", 0);
	srand(time(NULL));
	atexit(quit);

	if (argc > 1) {
		buf[0] = 0;
		for (int i = 1; i < argc; ++i) {
			strcat(buf, argv[i]); // TODO: Fix buffer overflow
			strcat(buf, " ");
		}
	} else {
		printf("f(x)=");
		fgets(buf, sizeof(buf), stdin);
	}

	SDL_AudioSpec spec;
	SDL_zero(spec);
	spec.callback = my_callback;
	spec.userdata = buf;
	spec.format = AUDIO_F32SYS;

	if (SDL_OpenAudio(&spec, NULL) < 0) {
		printf("Failed to open audio: %s\n", SDL_GetError());
		return 1;
	}
	SDL_PauseAudio(0);
	puts("Press CTRL + C to exit");
	while (1);
}
