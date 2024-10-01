#pragma once
#include "pch.h"



struct Token {
	enum class Type {
		INTEGER,
		IDENTIFIER,

		PLUS,
		MINUS,
		STAR,
		DIV,

		BANG,
		BANG_EQUAL,
		EQUAL_EQUAL,
		LESS,
		LESS_EQUAL,
		GREAT,
		GREAT_EQUAL,

		EQUAL,

		L_PAREN,
		R_PAREN,
		L_BRACE,
		R_BRACE,

		//Keywords
		VAR,
		PRINT,
		IF,
		ELSE,
		WHILE,
		FUN,
		RETURN,
		CLASS,
		SELF,

		SEMICOLON,
		COMMA,
		DOT,

		END

	};
	Token::Type type;
	std::string value;
};
class Lexer {
public:
	Lexer(std::string path) {
		set_kw_map();

		INFO("Reading file : {}", path);
		source = readFile(path);
		
		INFO("Starting Lexing");
		parseSource();

	}
	
	bool isEnd(int offset) {
		return idx + offset >= source.length();
	}

	bool isEnd() {
		return idx >= source.length();
	}

	char peek() {
		if (isEnd()) return '\0';
		return source[idx];
	}

	char peek(int offset) {
		if (isEnd(offset)) return '\0';
		return source[idx + offset];
	}

	char advance() {
		return source[idx++];
	}
	
	bool isDigit(char chr) {
		return chr >= '0' && chr <= '9';
	}

	bool isAlpha(char chr) {
		return chr >= 'A' && chr <= 'Z' || chr >= 'a' && chr <= 'z' || chr == '_';
	}

	bool isWhitespace(char chr) {
		return chr == ' ' || chr == '\t' || chr == '\r' || chr == '\n';
	}


	void parseSource() {
		while (!isEnd()) {
			char chr = peek();
			switch (chr)
			{
			case '+':
				AddToken(Token::Type::PLUS);
				advance();
				break;
			case '-':
				AddToken(Token::Type::MINUS);
				advance();
				break;
			case '*':
				AddToken(Token::Type::STAR);
				advance();
				break;
			case '/':
				AddToken(Token::Type::DIV);
				advance();
				break;
			case '=':
				advance();
				if (peek() == '=') {
					AddToken(Token::Type::EQUAL_EQUAL);
					advance();
					break;
				}
				AddToken(Token::Type::EQUAL);
				break;
			case '!':
				advance();
				if (peek() == '=') {
					AddToken(Token::Type::BANG_EQUAL);
					advance();
					break;
				}
				AddToken(Token::Type::BANG);
				break;
			case '<':
				advance();
				if (peek() == '=') {
					AddToken(Token::Type::LESS_EQUAL);
					advance();
					break;
				}
				AddToken(Token::Type::LESS);
				break;
			case '>':
				advance();
				if (peek() == '=') {
					AddToken(Token::Type::GREAT_EQUAL);
					advance();
					break;
				}
				AddToken(Token::Type::GREAT);
				break;

			case '(':
				AddToken(Token::Type::L_PAREN);
				advance();
				break;
			case ')':
				AddToken(Token::Type::R_PAREN);
				advance();
				break;
			case '{':
				AddToken(Token::Type::L_BRACE);
				advance();
				break;
			case '}':
				AddToken(Token::Type::R_BRACE);
				advance();
				break;
			case ';':
				AddToken(Token::Type::SEMICOLON);
				advance();
				break;
			case ',':
				AddToken(Token::Type::COMMA);
				advance();
				break;
			case '.':
				AddToken(Token::Type::DOT);
				advance();
				break;

			default:
				if (isDigit(chr)) {
					AddNumber();
				}
				else if (isAlpha(chr)) {
					AddIdentifier();
				}
				else if (isWhitespace(chr)) {
					advance();
				}
				break;
			}
		}
		AddToken(Token::Type::END);
	}

	void AddToken(Token::Type type) {
		tokens.push_back({ type, "" });
	}
	void AddToken(Token::Type type, std::string value) {
		tokens.push_back({ type, value });
	}

	void AddNumber() {
		int start = idx;
		while (isDigit(peek())) advance();
		int end = idx;
		AddToken(Token::Type::INTEGER, source.substr(start, end - start));
	}

	void AddIdentifier() {
		int start = idx;
		while (isAlpha(peek()) || isDigit(peek())) advance();
		int end = idx;
		std::string token_name = source.substr(start, end - start);

		if (kw_map.count(token_name) > 0) {
			AddToken(kw_map[token_name]);
		}
		else {
			AddToken(Token::Type::IDENTIFIER, token_name);
		}
	}

	std::vector<Token> getTokens() { return tokens; }

	void set_kw_map() {
		kw_map["var"] = Token::Type::VAR;
		kw_map["print"] = Token::Type::PRINT;
		kw_map["if"] = Token::Type::IF;
		kw_map["else"] = Token::Type::ELSE;
		kw_map["while"] = Token::Type::WHILE;
		kw_map["fun"] = Token::Type::FUN;
		kw_map["return"] = Token::Type::RETURN;
		kw_map["class"] = Token::Type::CLASS;
		//kw_map["self"] = Token::Type::SELF;
	}

private:
	std::unordered_map<std::string, Token::Type> kw_map;
	std::vector<Token> tokens;
	std::string source;
	int idx = 0;
};



