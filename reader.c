//
// Created by Legion on 04.12.2021.
//

#include <ctype.h>
#include <string.h>

#include "id.h"
#include "reader.h"
#include "handle_err.h"
#include "src/utils/macro_utils.h"

#define SET_DEFAULT_VALUES(current_reader, source_lib)                                                              \
    current_reader->source          =        NULL;                                                                  \
    current_reader->token           =     UNKNOWN;                                                                  \
    current_reader->last_peek       =           0;                                                                  \
    current_reader->is_indent       =        true;                                                                  \
    current_reader->string[0]       =           0;                                                                  \
    current_reader->total_indent    =           0;                                                                  \
    current_reader->indentation[0]  =           0;                                                                  \
    current_reader->source          =  source_lib


static struct {
	char* keyword;
	token_t token;
} keywords[] = {
	{ "and",      AND },
	{ "break",    BREAK },
	{ "char",     CHAR_INIT },
	{ "continue", CONTINUE },
	{ "def",    FUNC_INIT },
	{ "do",     DO },
	{ "else",   ELSE },
	{ "float",  FLOAT_INIT },
	{ "for",    FOR },
	{ "if",     IF },
	{ "import", IMPORT },
	{ "in",     IN },
	{ "input",  INPUT },
	{ "int",    INT_INIT },
	{ "list",   LIST_INIT},
	{ "or",     OR },
	{ "pass",   PASS },
	{ "print",  PRINT },
	{ "return", RETURN },
	{ "str",    STR_INIT },
	{ "while",  WHILE }
};

static token_t ParseToken(char* buffer, int buffer_size);


static int rd_GetChar() {
	REPORT(main_reader.source);

	return main_reader.source->GetChar(main_reader.source);
}

static int rd_PeekChar() {
    REPORT(main_reader.source);

	return main_reader.source->PeekChar(main_reader.source);
}

static int rd_PushChar(const int symbol) {
    REPORT(main_reader.source);

	return main_reader.source->PushChar(main_reader.source, (char) symbol);
}

static void ConstructReader(reader* current_reader, library* source_lib) {
	REPORT(current_reader);

	*current_reader = main_reader;

    SET_DEFAULT_VALUES(current_reader, source_lib);
}

static void CopyReader(reader* local_reader) {
	REPORT(local_reader);

	*local_reader = main_reader;
}

static void UploadReader(reader* local_reader) {
    REPORT(local_reader);

    main_reader = *local_reader;
}

static token_t GetToken() {
	if (!main_reader.last_peek) {
        main_reader.token = ParseToken(main_reader.string, sizeof(main_reader.string));
    } else {
        main_reader.token  =  main_reader.last_peek;
        main_reader.last_peek = !main_reader.last_peek;
	}

	debug_printf(LEXER_DEBUG, "\ntoken: %s %s", tokenName(main_reader.token), main_reader.string);

	return main_reader.token;
}

static token_t PeekToken() {
	if (!main_reader.last_peek) {
        main_reader.last_peek = ParseToken(main_reader.string, sizeof(main_reader.string));
    }

	return main_reader.last_peek;
}

static token_t GetString(char* string, int buffer_size) {
    REPORT(string);

	if (buffer_size < 2) {
        MAKE_ERR("can't make string %s, size is %i :( \n", NAME(buffer_size), buffer_size);
    }

	int symbol   = 0;
	int count    = 0;
	buffer_size--;

    /* getting string in cycle by chars, checking for escape sequences */
	while (true) {
        symbol = rd_GetChar();
		if (symbol != EOF && symbol != '\"') {
			if (symbol == '\\') {
                switch (rd_PeekChar()) {
                    case '0' :
                        rd_GetChar();
                        symbol = '\0';
                        break;

                    case 'a' :
                        rd_GetChar();
                        symbol = '\a';
                        break;

                    case 'b' :
                        rd_GetChar();
                        symbol = '\b';
                        break;

                    case 'f' :
                        rd_GetChar();
                        symbol = '\f';
                        break;

                    case 'n' :
                        rd_GetChar();
                        symbol = '\n';
                        break;

                    case 'r' :
                        rd_GetChar();
                        symbol = '\r';
                        break;

                    case 't' :
                        rd_GetChar();
                        symbol = '\t';
                        break;

                    case 'v' :
                        rd_GetChar();
                        symbol = '\v';
                        break;

                    case '\\':
                        rd_GetChar();
                        symbol = '\\';
                        break;

                    case '\'':
                        rd_GetChar();
                        symbol = '\'';
                        break;

                    case '\"':
                        rd_GetChar();
                        symbol = '\"';
                        break;
                }
            }
			if (count < buffer_size) {
                string[count++] = (char) symbol;
            }
		} else {
			string[count] = 0;
			break;
		}
	}
	return STR;
}

static token_t rd_GetNumber(char* number, int buffer_size) {
    REPORT(number);
    if (buffer_size < 2) {
        MAKE_ERR("not enough memory allocated for the number: %s = %i \n", NAME(buffer_size), buffer_size);
    }

	int symbol = 0, exponent = 0, num_length = 0, count = 0;
	buffer_size--;

	while (true) {
        symbol = rd_GetChar();
		if (symbol != EOF && (isdigit(symbol) || symbol == '.')) {
			if (symbol == '.') { {
                if (++num_length > 1) {
                    RaiseError(VALUE_ERROR, "oh, cmon, why did you put extra decimal points here");
                }
            }
			}
			if (count < buffer_size)
				number[count++] = (char) symbol;
		} else {
			if (symbol == 'e' || symbol == 'E') {
                exponent = 1;
				if (count < buffer_size)
					number[count++] = (char) symbol;
                symbol = rd_GetChar();

				if (symbol == '-' || symbol == '+') {
					if (count < buffer_size)
						number[count++] = (char) symbol;
                    symbol = rd_GetChar();
				}
				if (!isdigit(symbol)) {
                    RaiseError(VALUE_ERROR, "where the fuck is exponent");
                }
				while (symbol != EOF && isdigit(symbol)) {
					if (count < buffer_size) {
                        number[count++] = (char) symbol;
                    }
                    symbol = rd_GetChar();
				}
			}
			number[count] = 0;
            rd_PushChar(symbol);
			break;
		}
	}

	if (num_length == 1 || exponent == 1)
		return FLOAT;

	return INT;
}

/* binary FindLibrary is possible because of sorted names */
static int FindIdentifier(char* name, int* compare_result) {
    int left = 0, right = 0, middle = 0;
    right = sizeof (keywords) / sizeof (keywords[0]) - 1;;

    while (left <= right) {
        middle = (left + right) / 2;
        /* strcmp() return difference between last chars, so result can be 0 (equal) or >0, or <0 */
        *compare_result = strcmp(&name[0], keywords[middle].keyword);
        if (*compare_result < 0) {
            right = middle - 1;
        }
        if (*compare_result > 0) {
            left = middle + 1;
        }
        if (!*compare_result) break;
    }
    return middle;
}

static token_t rd_GetId(char* name, int buffer_size) {
    REPORT(name);
    if (buffer_size < 2) {
        MAKE_ERR("not enough memory allocated for the identifier: %s = %i \n", NAME(buffer_size), buffer_size);
    }

	int symbol = 0;
	int id_length = 0;
    buffer_size--;

	while (true) {
        symbol = rd_GetChar();
		if (symbol != EOF && (isalnum(symbol) || symbol == '_')) {
			if (id_length < buffer_size)
				name[id_length++] = (char) symbol;
		} else {
			name[id_length] = 0;
            rd_PushChar(symbol);
			break;
		}
	}
    int compare_result = 0;
    int id_index = FindIdentifier(name, &compare_result);

	if (!compare_result) {
		name[0] = 0;
		return keywords[id_index].token;
	} else return IDENTIFIER;
}

static token_t GetSimpleChar(char* c, int buffer_size) {
    REPORT(c);

    if (buffer_size < 2) {
        MAKE_ERR("not enough memory allocated for the identifier: %s = %i \n", NAME(buffer_size), buffer_size);
    }

	int symbol = 0;
    symbol = rd_GetChar();

	if (symbol == '\\') {
		symbol = rd_GetChar();
		switch (symbol) {
			case '0' :
                c[0] = '\0';
                break;

			case 'a' :
                c[0] = '\a';
                break;

			case 'b' :
                c[0] = '\b';
                break;

			case 'f' :
                c[0] = '\f';
                break;

			case 'n' :
                c[0] = '\n';
                break;

			case 'r' :
                c[0] = '\r';
                break;

			case 't' :
                c[0] = '\t';
                break;

			case 'v' :
                c[0] = '\v';
                break;

			case '\\':
                c[0] = '\\';
                break;

			case '\'':
                c[0] = '\'';
                break;

			case '\"':
                c[0] = '\"';
                break;

			default  :
                RaiseError(SYNTAX_ERROR, "I have no fucking idea what is that: %c", symbol);
                break;
		}
	} else {
		if (symbol == '\'' || symbol == EOF) {
            RaiseError(SYNTAX_ERROR, "empty character constant");
        } else *(c + 1) = (char) symbol;
	}
    symbol = rd_GetChar();
	if (symbol != '\'')
        RaiseError(SYNTAX_ERROR, "to many characters in character constant");

	*(c + 1) = 0; // null-byte as a termination symbol

	return CHAR;
}

static token_t ParseToken(char* buffer, int buffer_size) {
    REPORT(buffer);

    if (buffer_size < 2) {
        MAKE_ERR("not enough memory allocated for the identifier: %s = %i \n", NAME(buffer_size), buffer_size);
    }

	int symbol = 0;
	*buffer    = 0;

	while (main_reader.is_indent) {
		int column            =     0;
        main_reader.is_indent = false;

		while (true) {
            symbol = rd_GetChar();
            switch (symbol) {
                case ' ':
                    column++;
                    break;

                case '\t':
                    column += parser_configs.tab_size;
                    break;

                default:
                    goto read_lines;
                    break;
            }
		}
        read_lines:;

		if (symbol == '#') {
            while (symbol != '\n' && symbol != EOF) {
                symbol = rd_GetChar();
            }
        }
		if (symbol == '\r') {
            symbol = rd_GetChar();
        }
		if (symbol == '\n') {
            main_reader.is_indent = true;
            continue;
		} else if (symbol == EOF) {
            column = 0;
			if (column == main_reader.indentation[main_reader.total_indent]) {
                return END_TRIGGER;
            }
		} else
            rd_PushChar(symbol);

        /* checking different variant of indentation (did it change?) */
		if (column == main_reader.indentation[main_reader.total_indent]) break;
		else if (column > main_reader.indentation[main_reader.total_indent]) {
			if (main_reader.total_indent == TOTAL_IND_MAX) {
                RaiseError(SYNTAX_ERROR, "no more indents for you, bozo!");
            }
            main_reader.indentation[++main_reader.total_indent] = column;
			return INDENT;
		} else {
			if (--main_reader.total_indent < 0) {
                RaiseError(SYNTAX_ERROR, "bro you are discrediting dumb, use tabs normally");
            }
            /* checking if modified indentation old or not */
			if (column != main_reader.indentation[main_reader.total_indent]) {
                main_reader.is_indent = true;
				main_reader.source->buf_char_index = main_reader.source->buf_line_index;
			}
			return DEDENT;
		}
	}

	do {
        symbol = rd_GetChar();
	} while (symbol == ' ' || symbol == '\t');

	if (symbol == '#') {
        while (symbol != '\n' && symbol != EOF) {
            symbol = rd_GetChar();
        }
    }

    /* just skip \r symbols */
	if (symbol == '\r') {
        symbol = rd_GetChar();
    }
	if (symbol == '\n') {
        main_reader.is_indent = true;
		return NEWLINE;
	} else if (symbol == EOF) return END_TRIGGER;

	if (isdigit(symbol)) {
        rd_PushChar(symbol);
		return rd_GetNumber(buffer, buffer_size);
	} else if (isalpha(symbol)) {
        rd_PushChar(symbol);
		return rd_GetId(buffer, buffer_size);
	} else {
		switch (symbol) {
			case '\'':
                return GetSimpleChar(buffer, buffer_size);

			case '\"':
                return GetString(buffer, buffer_size);

			case EOF :
                return END_TRIGGER;

			case '(' :
                return LEFT_PAR;

			case ')' :
                return RIGHT_PAR;

			case '[' :
                return LEFT_SEQ_BLOCK;

			case ']' :
                return RIGHT_SEQ_BLOCK;

			case ',' :
                return COMMA;

			case '.' :
                return METHOD;

			case ':' :
                return COLON;

			case '*' :
                if (rd_PeekChar() == '=') {
                    rd_GetChar();
                    return STAR_EQUAL;
                } else return STAR;

			case '%' :
                if (rd_PeekChar() == '=') {
                    rd_GetChar();
                    return PERCENT_EQUAL;
                } else return PERCENT;

			case '+' :
                if (rd_PeekChar() == '=') {
                    rd_GetChar();
                    return PLUS_EQUAL;
                } else return PLUS;

			case '-' :
                if (rd_PeekChar() == '=') {
                    rd_GetChar();
                    return MINUS_EQUAL;
                } else return MINUS;

			case '/' :
                if (rd_PeekChar() == '=') {
                    rd_GetChar();
                    return SLASH_EQUAL;
                } else return SLASH;

			case '!' :
                if (rd_PeekChar() == '=') {
                    rd_GetChar();
                    return NOT_EQUAL;
                } else return NOT;

            case '=' :
                if (rd_PeekChar() == '=') {
                    rd_GetChar();
                    return EQUAL_EQUAL;
                } else return EQUAL;

            case '<' :
                if (rd_PeekChar() == '=') {
                    rd_GetChar();
                    return LESS_EQUAL;
                } else if (rd_PeekChar() == '>') {
                    rd_GetChar();
                    return NOT_EQUAL;
                } else return LESS;

			case '>' :
                if (rd_PeekChar() == '=') {
                    rd_GetChar();
                    return GREATER_EQUAL;
                } else return GREATER;

            default:
                MAKE_ERR("lol wtf is this: \'%c\'\n", symbol);
                return UNKNOWN;

		}
	}
	return UNKNOWN;
}

reader main_reader = {
	.source = NULL,

	.token = UNKNOWN,
	.last_peek = 0,
	.is_indent = true,
	.string[0] = 0,

	.total_indent = 0,
	.indentation[0] = 0,

	.GetToken = GetToken,
	.PeekToken = PeekToken,
	.MakeReader = ConstructReader,
	.CopyReader = CopyReader,
	.UploadReader = UploadReader
	};
