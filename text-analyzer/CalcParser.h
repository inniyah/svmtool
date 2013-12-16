#ifndef CALCPARSER_H_E9E83CF6_2CC0_11E3_A59D_F23C91AEC05E
#define CALCPARSER_H_E9E83CF6_2CC0_11E3_A59D_F23C91AEC05E

// Bare bones scanner and parser for the following LL(1) grammar:
// expr -> term { [+-] term }     ; An expression is terms separated by add ops.
// term -> factor { [*/] factor } ; A term is factors separated by mul ops.
// factor -> unsigned_factor      ; A signed factor is a factor, 
//         | - unsigned_factor    ;   possibly with leading minus sign
// unsigned_factor -> ( expr )    ; An unsigned factor is a parenthesized expression 
//         | NUMBER               ;   or a number
//
// See: http://stackoverflow.com/questions/13853774/infix-calculator-expression-parser

enum class CalcParserToken {
	ADD_OP, MUL_OP, LEFT_PAREN, RIGHT_PAREN, NUMBER, HEXADECIMAL, END_INPUT, BAD_CHAR
};

template <typename NUM>
class CalcParser {
public:
	bool getResult(const char * str, unsigned int len, NUM & num);
	bool getNextResult(NUM & result);

protected:
	virtual int get_char();
	virtual bool a_to_num(const char * txt, NUM & num);
	virtual bool hex_to_num(const char * txt, NUM & num);

	const char * CalcParserTokenName(CalcParserToken t) {
	static const char * n[] = {
		"ADD_OP", "MUL_OP", "LEFT_PAREN", "RIGHT_PAREN", "NUMBER", "HEXADECIMAL", "END_INPUT", "BAD_CHAR"
	};
	return n[(int)t];
}

private:
	void read(); // Buffer the current character and read a new one.
	void ignore(); // Ignore the current character.
	void reset(); // Reset the token buffer.
	CalcParserToken scan(); // The scanner.  A tiny deterministic finite automaton.
	void advance(); // To advance is just to replace the look-ahead.
	void init(); // Clear the token buffer and read the first look-ahead.
	bool unsigned_factor(NUM & num);
	bool factor(NUM & num);
	bool term(NUM & num);
	bool expr(NUM & num);

	const char * input_data;
	unsigned int input_length;

	static const int MAX_BUF_SIZE = 1024;
	char buf[MAX_BUF_SIZE]; // The token buffer. Warning: we never check for overflow!
	int n = 0;

	int ch; // The current character.
	CalcParserToken look_ahead; // The look-ahead token.  This is the 1 in LL(1).
};

#endif // CALCPARSER_H_E9E83CF6_2CC0_11E3_A59D_F23C91AEC05E
