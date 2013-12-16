#ifndef CALCPARSER_HPP_E9E8420A_2CC0_11E3_A59D_F23C91AEC05E
#define CALCPARSER_HPP_E9E8420A_2CC0_11E3_A59D_F23C91AEC05E

#include "CalcParser.h"

#define debug_printf printf
//#define debug_printf(...)

// Format Conversions: scanf, fscanf, sscanf
// Format string:
//	% [*] [field_width] [length_modifier] conversion_character
// Length modifier:
//	h	For d, i, o, u or x conversions, the value is to be input as a short, not an int.
//	l	For d, i, o, u or x conversions, the value is to be input as a long, not an int.
//		For e, f or g conversions, the value is to be input as a double, not a float.
//	L	For e, f or g conversions, the value is to be input as a long double. 
// Conversion character:
//	d	Input a decimal integer; argument must be int *.
//	i	Input an integer; argument must be int *. The integer may be in octal (with a leading 0) or hexadecimal (with a leading 0x or 0X).
//	o	Input an octal integer (with or without a leading 0); argument must be int *.
//	u	Input an unsigned decimal integer; argument must be unsigned int *.
//	x	Input a hexadecimal integer (with or without a leading 0x or 0X); argument must be int *.
//	c	Input one or more characters; argument must be char *. The characters are put in the character array, up to the number given by the field width. (No terminating '\0' is added.) If a field width is given, whitespace is not skipped. If no field width is given, only one character is input. (To read the next non-white space character, use %1s.)
//	s	Input a string of non-whitespace characters; the argument must be char *. A terminating '\0' is added.
//	e, f, g	Input a floating point number; argument must be float *. The input format is an optional sign, a string of numbers (possibly containing a decimal point), and an optional exponent field containing an E or e followed by a (possibly signed) integer. So numbers to be input can be in either decimal or scientific notation.
//	p	Input a pointer value; argument must be void *. The representation is implementation dependent and is as printed by printf("%p").
//	n	No input is read. Instead, the argument receives the number of characters read so far by this call; argument must be int *.
//	[...]	Inputs a string; argument must be char *. A terminating '\0' is added. Only the characters listed between the brackets are accepted. Input ends when a non-matching character is reached or the field width is reached. To accept the ']' character, list it first: []...]
//	[^...]	Inputs a string; argument must be char *. A terminating '\0' is added. Only characters not listed between the brackets are accepted. Input ends when a non-matching character is reached or the field width is reached. To reject the ']' character, list it first: [^]...]
//	%	Literal %: reads the '%' character. No assignment is made. 

template <typename NUM>
bool CalcParser<NUM>::getResult(const char * str, unsigned int len, NUM & num) {
	if (str == NULL || len == 0) {
		input_data = NULL;
		input_length = 0;
		debug_printf("[Err:Null Input]\n");
		return false;
	}
	input_data = str;
	input_length = len;

	look_ahead = CalcParserToken::BAD_CHAR;
	ch = ' ';

	reset();
	//ignore(); // junk current character
	advance();

	debug_printf("[Input='%.*s']\n", input_length, input_data);

	if (!expr(num)) {
		debug_printf("[Err:Invalid Expr]\n");
		return false;
	}

	return true;
}

template <typename NUM>
bool CalcParser<NUM>::getNextResult(NUM & num) {
	if (!expr(num)) {
		return false;
	}
	if (look_ahead != CalcParserToken::END_INPUT) {
		return false;
	}
	advance();  // past end of input mark
	return true;
}

template <typename NUM>
int CalcParser<NUM>::get_char() {
	if (input_length == 0 || input_data == NULL) {
		return 0;
	}
	input_length--;
	return (int)*input_data++;
}

template<>
bool CalcParser<double>::a_to_num(const char * txt, double & num) {
	debug_printf("[Double:%s]\n", buf);
	return (sscanf(buf, "%lf", &num) == 1 ? true : false);
}

template<>
bool CalcParser<float>::a_to_num(const char * txt, float & num) {
	debug_printf("[Float:%s]\n", buf);
	return (sscanf(buf, "%f", &num) == 1 ? true : false);
}

template<>
bool CalcParser<int>::a_to_num(const char * txt, int & num) {
	debug_printf("[Int:%s]\n", buf);
	return (sscanf(buf, "%d", &num) == 1 ? true : false);
}

template<>
bool CalcParser<long int>::a_to_num(const char * txt, long int & num) {
	debug_printf("[Long Int:%s]\n", buf);
	return (sscanf(buf, "%ld", &num) == 1 ? true : false);
}

template <typename NUM>
bool CalcParser<NUM>::hex_to_num(const char * txt, NUM & num) {
	debug_printf("[Hexadecimal:%s]\n", buf);
	unsigned long int hex;
	if (sscanf(buf, "%lx", &hex) != 1) return false;
	num = hex;
	return true;
}

template <typename NUM>
void CalcParser<NUM>::read() {
	buf[n++] = ch; // Store current character
	buf[n] = '\0'; // Terminate the string
	ch = get_char(); // Get next character
}

template <typename NUM>
void CalcParser<NUM>::ignore() {
	ch = get_char();
}

template <typename NUM>
void CalcParser<NUM>::reset() {
	n = 0;
	buf[0] = '\0';
}

template <typename NUM>
CalcParserToken CalcParser<NUM>::scan() {
	reset();

	START:
	  debug_printf("[Start Char='%c' (0x%02X)\n", ch, ch);
	  switch (ch) {
		case ' ': case '\t': case '\r':
		  ignore();
		  goto START;

		case '-': case '+':
		  read();
		  return CalcParserToken::ADD_OP;

		case '*': case '/':
		  read();
		  return CalcParserToken::MUL_OP;

		case '(':
		  read();
		  return CalcParserToken::LEFT_PAREN;

		case ')':
		  read();
		  return CalcParserToken::RIGHT_PAREN;

		case '0':
		  read();
		  goto IN_LEADING_ZERO;
		
		case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
		  read();
		  goto IN_LEADING_DIGITS;

		case '\n':
		  ch = ' '; // delayed ignore()
		  return CalcParserToken::END_INPUT;

		default:
		  return CalcParserToken::BAD_CHAR;
	  }

	IN_LEADING_ZERO:
	  debug_printf("[Leading Zero Char='%c' (0x%02X)\n", ch, ch);
	  switch (ch) {
		case 'x': case 'X':
		  read();
		  goto IN_HEXA_DIGITS;	  

		default:
		  goto IN_LEADING_DIGITS; // Pass through
	  }

	IN_LEADING_DIGITS:
	  debug_printf("[Leading Digit Char='%c' (0x%02X)\n", ch, ch);
	  switch (ch) {
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
		  read();
		  goto IN_LEADING_DIGITS;

		case '.':
		  read();
		  goto IN_TRAILING_DIGITS;

		default:
		  return CalcParserToken::NUMBER;
	  }

	IN_TRAILING_DIGITS:
	  debug_printf("[Trailing Digit Char='%c' (0x%02X)\n", ch, ch);
	  switch (ch) {
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
		  read();
		  goto IN_TRAILING_DIGITS;

		default:
		  return CalcParserToken::NUMBER;
	  }          

	IN_HEXA_DIGITS:
	  debug_printf("[Hexa Digit Char='%c' (0x%02X)\n", ch, ch);
	  switch (ch) {
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
		case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
		case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
		  read();
		  goto IN_HEXA_DIGITS;

		default:
		  return CalcParserToken::HEXADECIMAL;
	  }

}

template <typename NUM>
void CalcParser<NUM>::advance() {
	look_ahead = scan();
	if (input_data != NULL) {
		debug_printf("[%s:buf='%s':inp='%.*s']\n",CalcParserTokenName(look_ahead), buf, input_length, input_data);
	} else {
		debug_printf("[%s:buf='%s':inp=NULL]\n",CalcParserTokenName(look_ahead), buf);
	}
}

template <typename NUM>
void CalcParser<NUM>::init() {
  reset();
  ignore(); // junk current character
  advance();
}

template <typename NUM>
bool CalcParser<NUM>::unsigned_factor(NUM & num) {
	switch (look_ahead) {
		case CalcParserToken::NUMBER:
			if (!a_to_num(buf, num)) {
			debug_printf("[Err:Invalid Number]\n");
			return false;
		}
		advance();
		break;

		case CalcParserToken::HEXADECIMAL:
			if (!hex_to_num(buf, num)) {
			debug_printf("[Err:Invalid Hexadecimal Number]\n");
			return false;
		}
		advance();
		break;

		case CalcParserToken::LEFT_PAREN:
			advance();
			if (!expr(num)) {
				debug_printf("[Err:Invalid Number in Paren]\n");
				return false;
			}
			if (look_ahead != CalcParserToken::RIGHT_PAREN) {
				debug_printf("[Err:Missing Closing Paren]\n");
				return false; // Missing closing parenthesis
			}
			advance();
			break;

		default:
			debug_printf("[Err:Unexpected %s Token]\n", CalcParserTokenName(look_ahead));
			return false;
  }
  return true;
}

template <typename NUM>
bool CalcParser<NUM>::factor(NUM & num) {
  if (look_ahead == CalcParserToken::ADD_OP && buf[0] == '-') { // If there is a leading minus...
    advance();
    if (!unsigned_factor(num)) {
		debug_printf("[Err:Invalid Unsigned Factor for Neg]\n");
		return false;
	}
    num = -num;
  } else {
    if (!unsigned_factor(num)) {
		debug_printf("[Err:Invalid Unsigned Factor]\n");
		return false;
	}
  }
  return true;
}

template <typename NUM>
bool CalcParser<NUM>::term(NUM & num) {
  if (!factor(num)) {
	debug_printf("[Err:Invalid Factor]\n");
	return false;
  }
  while (look_ahead == CalcParserToken::MUL_OP) {
	NUM f;
    switch(buf[0]) {
      case '*':
        advance(); 
		if (!factor(f)) {
			debug_printf("[Err:Invalid Factor for Mul]\n");
			return false;
		}
        num *= f; 
        break; 

      case '/':
        advance();
		if (!factor(f)) {
			debug_printf("[Err:Invalid Factor for Div]\n");
			return false;
		}
        num /= f;
        break;
    }
  }
  return true;
}

template <typename NUM>
bool CalcParser<NUM>::expr(NUM & num) {
  if (!term(num)) {
	debug_printf("[Err:Invalid Term]\n");
	return false;
  }
  while (look_ahead == CalcParserToken::ADD_OP) {
	NUM t;
    switch(buf[0]) {
      case '+': 
        advance();
		if (!term(t)) {
			debug_printf("[Err:Invalid Term for Add]\n");
			return false;
		}
        num += t; 
        break; 

      case '-':
        advance();
		if (!term(t)) {
			debug_printf("[Err:Invalid Term for Sub]\n");
			return false;
		}
        num -= t;
        break;
    }
  }
  return true;
}

#endif // CALCPARSER_HPP_E9E8420A_2CC0_11E3_A59D_F23C91AEC05E
