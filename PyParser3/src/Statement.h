#pragma once
#include "pch.h"
#include "Lexer.h"
#include "Expression.h"

// Statements

enum class StmtType {
	BLOCK,
	EXPRESSION,
	PRINT,
	IF,
	WHILE,
	FUNC_DECL,
	RETURN,
	CLASS_DECL,

};

struct Stmt {
	StmtType type;

};

struct BlockStmt : public Stmt {
	std::vector<Stmt*> stmts;
	BlockStmt(std::vector<Stmt*> _stmts) {
		type = StmtType::BLOCK;
		stmts = _stmts;
	}
};

struct ExprStmt : public Stmt {
	Expr* expr;
	ExprStmt(Expr* _expr) {
		type = StmtType::EXPRESSION;
		expr = _expr;
	}
};

struct PrintStmt : public Stmt {
	Expr* expr;
	PrintStmt(Expr* _expr) {
		type = StmtType::PRINT;
		expr = _expr;
	}
};

struct IfStmt : public Stmt {
	Expr* condition;
	Stmt* thenStmt;
	Stmt* elseStmt;
	IfStmt(Expr* _cond, Stmt* _thenStmt, Stmt* _elseStmt) {
		type = StmtType::IF;
		condition = _cond;
		thenStmt = _thenStmt;
		elseStmt = _elseStmt;
	}
};

struct WhileStmt : public Stmt {
	Expr* condition;
	Stmt* main;
	WhileStmt(Expr* _cond, Stmt* _main) {
		type = StmtType::WHILE;
		condition = _cond;
		main = _main;
	}
};

struct FuncDeclStmt : public Stmt {
	Token name;
	std::vector<Token> params;
	BlockStmt* body;
	FuncDeclStmt(Token _name, std::vector<Token> _params, BlockStmt* _body) {
		type = StmtType::FUNC_DECL;
		name = _name;
		params = _params;
		body = _body;
	}
};

struct ReturnStmt : public Stmt {
	Expr* retVal;
	ReturnStmt(Expr* _retVal) {
		type = StmtType::RETURN;
		retVal = _retVal;
	}
};

struct ClassDeclStmt : public Stmt {
	Token name;
	std::vector<FuncDeclStmt*> methods;
	ClassDeclStmt(Token _name, std::vector<FuncDeclStmt*> _methods) {
		type = StmtType::CLASS_DECL;
		name = _name;
		methods = _methods;
	}
};
