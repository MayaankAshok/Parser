#pragma once
#include "pch.h"
#include "Lexer.h"
#include "Expression.h"
#include "Statement.h"
#include "Object.h"


class Parser {
public:
	Parser(std::vector<Token> _tokens) {
		tokens = _tokens;
		INFO("Starting Parsing");

		statements = parse();

		DEB("Printing Representation");
		for (Stmt* stmt: statements ) DEB(repr(stmt));
	}

	std::vector<Stmt*>& getStatements() {
		return statements;
	}

private:
	
	std::vector<Stmt*> parse() {
		std::vector<Stmt*> statements;
		while (!isEnd()) {
			statements.push_back(statement());
		}
		return statements;
	}

	// Parse Statement

	Stmt* statement() {

		if (match(Token::Type::FUN)) {
			return funcDeclStatement();
		}
		if (match(Token::Type::RETURN)) {
			return returnStatement();
		}
		if (match(Token::Type::CLASS)) {
			return classDeclStatement();
		}
		if (match(Token::Type::L_BRACE)) {
			return blockStatement();
		}
		if (match(Token::Type::PRINT)) {
			return printStatement();
		}
		if (match(Token::Type::IF)) {
			return ifStatement();
		}
		if (match(Token::Type::WHILE)) {
			return whileStatement();
		}

		return exprStatement();
	}



	FuncDeclStmt* funcDeclStatement() {
		match(Token::Type::IDENTIFIER);
		Token name = peek(-1);
		match(Token::Type::L_PAREN);
		
		std::vector<Token> params;
		if (!check(Token::Type::R_PAREN)) {
			do {
				match(Token::Type::IDENTIFIER);
				params.push_back(peek(-1));
			} while (match(Token::Type::COMMA));
		}
		match(Token::Type::R_PAREN);
		match(Token::Type::L_BRACE);
		BlockStmt* body = (BlockStmt*)blockStatement();
		return new FuncDeclStmt{ name, params, body };

	}
	
	ReturnStmt* returnStatement() {
		Expr* expr = nullptr;
		if (!check(Token::Type::SEMICOLON)) {
			expr = expression();
		}
		match(Token::Type::SEMICOLON);
		return new ReturnStmt{ expr };
	}

	ClassDeclStmt* classDeclStatement() {
		match(Token::Type::IDENTIFIER);
		Token name = peek(-1);
		
		match(Token::Type::L_BRACE);
		std::vector<FuncDeclStmt*> methods;
		while (!check(Token::Type::R_BRACE)) {
			match(Token::Type::FUN);
			methods.push_back(funcDeclStatement());
		}
		match(Token::Type::R_BRACE);
		return new ClassDeclStmt{ name, methods };

	}

	BlockStmt* blockStatement() {
		std::vector<Stmt*> stmts;
		while (!match(Token::Type::R_BRACE)) {
			stmts.push_back(statement());
		}
		return new BlockStmt(stmts);
	}

	PrintStmt* printStatement() {
		Expr* expr = expression();
		match(Token::Type::SEMICOLON);
		return new PrintStmt(expr);
	}

	ExprStmt* exprStatement() {
		Expr* expr = expression();
		match(Token::Type::SEMICOLON);
		return new ExprStmt(expr);

	}

	IfStmt* ifStatement() {
		match(Token::Type::L_PAREN);
		Expr* condition = expression();
		match(Token::Type::R_PAREN);
		Stmt* thenStmt = statement();
		Stmt* elseStmt = nullptr;
		if (match(Token::Type::ELSE)) {
			elseStmt = statement();
		}
		return new IfStmt{ condition, thenStmt, elseStmt };
	}

	WhileStmt* whileStatement() {
		match(Token::Type::L_PAREN);
		Expr* condition = expression();
		match(Token::Type::R_PAREN);
		Stmt* mainStmt = statement();
		return new WhileStmt{ condition, mainStmt};
	}


	// Grammar
	
	// expression -> assignment
	// assignment -> equality ["=" assignment]
	// equality -> comparison (("!=" | "==") comparison)*
	// comparsion -> term ( (">" | ">=" | "<" | "<=") term)*
	// term -> factor (("+" | "-") factor)*
	// factor -> unary (("*" | "/") unary)*
	// unary -> ( "!" | "-") unary | call 
	// call -> primary "(" [expression ("," expression)*] ")"
	// primary -> INTEGER 
	//			| IDENTIFIER ( "." IDENTIFIER)*
	//			| "(" expression ")"
	
	// Parse Expression

	Expr* expression() {
		return assignment();
	}

	Expr* assignment() {
		Expr* expr = equality();
		if (match(Token::Type::EQUAL)) {
			//Token equal = peek(-1);
			Expr* value = assignment();
			if (expr->type == ExprType::Identifier ||
				expr->type == ExprType::Get) {
				return new AssignExpr(expr, value);
			}
			else {
				std::cout << "Invalid Assignment Target" << std::endl;
			}
		}
		return expr;
	}

	Expr* equality() {
		Expr* expr = comparison();
		while (match({ Token::Type::BANG_EQUAL , Token::Type::EQUAL_EQUAL })) {
			Token oper = peek(-1);
			Expr* right = comparison();
			expr = new BinaryExpr(expr, oper, right);
		}
		return expr;
	}

	Expr* comparison() {
		Expr* expr = term();
		while (match({ Token::Type::GREAT , Token::Type::GREAT_EQUAL, Token::Type::LESS, Token::Type::LESS_EQUAL })) {
			Token oper = peek(-1);
			Expr* right = term();
			expr = new BinaryExpr(expr, oper, right);
		}
		return expr;
	}

	Expr* term() {
		Expr* expr = factor();
		while (match({ Token::Type::PLUS , Token::Type::MINUS })) {
			Token oper = peek(-1);
			Expr* right = factor();
			expr = new BinaryExpr(expr, oper, right);
		}
		return expr;
	}

	Expr* factor() {
		Expr* expr = unary();
		while (match({ Token::Type::STAR, Token::Type::DIV })) {
			Token oper = peek(-1);
			Expr* right = unary();
			expr = new BinaryExpr(expr, oper, right);
		}
		return expr;
	}

	Expr* unary() {
		if (match({ Token::Type::BANG, Token::Type::MINUS })) {
			Token oper = peek(-1);
			Expr* right = unary();
			return new UnaryExpr(oper, right);
		}
		return call();
	}

	Expr* call() {
		Expr* expr = primary();

		while (true) {
			if (match(Token::Type::L_PAREN)) {
				expr = finishCall(expr);
			}
			else break;	
		}
		return expr;
	}

	Expr* finishCall(Expr* name) {
		std::vector<Expr*> args;
		if (!check(Token::Type::R_PAREN)) {
			do {
				args.push_back(expression());
			} while (match(Token::Type::COMMA));
		}
		match(Token::Type::R_PAREN);
		return new FuncCallExpr(name, args);
	}

	Expr* primary() {
		if (match(Token::Type::INTEGER)) {
			return new LiteralExpr(peek(-1));
		}
		if (match(Token::Type::IDENTIFIER)) {
			Expr* expr = new IdentifierExpr{ peek(-1) };
			while (match(Token::Type::DOT)) {
				expr = new GetExpr(expr,  peek());
				advance();
			}
			return expr;
		}
		if (match(Token::Type::L_PAREN)) {
			Expr* expr = expression();
			match(Token::Type::R_PAREN);
		}
	}

	// Print Representation

	std::string repr(Stmt* stmt) {
		switch (stmt->type) {
		case StmtType::BLOCK:
			return repr((BlockStmt*)stmt);
		case StmtType::EXPRESSION:
			return repr((ExprStmt*)stmt);
		case StmtType::CLASS_DECL:
			return repr((ClassDeclStmt*)stmt);
		case StmtType::PRINT:
			return repr((PrintStmt*)stmt);
		default:
			return "UNKNOWN STMT;";
		}
	}

	std::string repr(PrintStmt* stmt) {
		return std::string("PRINT ") + repr(stmt->expr) ;

	}
	std::string repr(BlockStmt* stmt) {
		std::string output = "( \n";
		for (Stmt* _stmt : stmt->stmts) {
			output += repr(_stmt);
			output += "\n";
		}
		output += ");";
		return output;
	}

	std::string repr(ExprStmt* stmt) {
		return repr(stmt->expr) + ";";
	}

	std::string repr(ClassDeclStmt* stmt) {
		return "CLASS DECL;";
	}

	std::string repr(Expr* expr) {
		if (!expr) return "(ERR NULL EXPR)";
		switch (expr->type)
		{
		case ExprType::Literal:
			return repr((LiteralExpr*)expr);
			//case ExprType::Unary:
				//return reprUnary((UnaryExpr*)expr)
		case ExprType::Binary:
			return repr((BinaryExpr*)expr);
		case ExprType::Assign:
			return repr((AssignExpr*)expr);
		case ExprType::Get:
			return repr((GetExpr*)expr);
		case ExprType::Identifier:
			return repr((IdentifierExpr*)expr);

		default:
			return "(UNKNOWN EXPR)";
		}
	}
	std::string repr(IdentifierExpr* expr) {
		return expr->token.value;
	}
	std::string repr(GetExpr* expr) {
		return repr(expr->left) + "." + expr->right.value;
	}

	std::string repr(AssignExpr* expr) {
		return repr(expr->left) + " = " + repr(expr->value);
	}

	std::string repr(LiteralExpr* expr) {
		return expr->token.value;
	}
	
	std::string repr(BinaryExpr* expr) {
		std::string operStr;
		switch (expr->oper.type)
		{
		case Token::Type::PLUS:
			operStr = "+";

		case Token::Type::MINUS:
			operStr = "+";
		default:
			break;
		}
		return std::string("( ") + operStr + " " + repr(expr->left) + " " + repr(expr->right) + " )";
	}

	// Utilities

	bool check(Token::Type type) {
		return peek().type == type;
	}

	bool match(Token::Type type) {
		return match(std::vector<Token::Type>{ type });
	}
	bool match(std::vector<Token::Type> types) {
		for (Token::Type type : types) {	
			if (check(type)) {
				advance();
				return true;
			}
		}
		return false;
	}


	Token advance() {
		return tokens[idx++];
	}
	
	Token peek() {
		return peek(0);
	}
	Token peek (int offset){
		return tokens[idx+ offset];
	}
	
	bool isEnd() {
		return peek().type == Token::Type::END;
	}
	
private:


	int idx = 0;
	std::vector<Token> tokens;
	std::vector<Stmt*> statements;
};


