#pragma once
#include "pch.h"
#include "Lexer.h"


// Expressions

enum class ExprType {
	Literal,
	Identifier,

	Unary,
	Binary,

	Assign,
	FuncCall,
};

struct Expr {
	ExprType type;

};

struct LiteralExpr : public Expr {
	Token token;
	LiteralExpr(Token _token) {
		type = ExprType::Literal;
		token = _token;
	}
};

struct IdentifierExpr : public Expr {
	Token token;
	IdentifierExpr(Token _token) {
		type = ExprType::Identifier;
		token = _token;
	}
};

struct BinaryExpr : public Expr {
	Expr* left;
	Token oper;
	Expr* right;

	BinaryExpr(Expr* _left, Token _oper, Expr* _right) {
		type = ExprType::Binary;
		left = _left;
		oper = _oper;
		right = _right;
	}
};

struct UnaryExpr : public Expr {
	Token oper;
	Expr* right;

	UnaryExpr(Token _oper, Expr* _right) {
		type = ExprType::Unary;
		oper = _oper;
		right = _right;
	}
};

struct AssignExpr : public Expr {
	Token name;
	Expr* value;
	AssignExpr(Token _name, Expr* _value) {
		type = ExprType::Assign;
		name = _name;
		value = _value;
	}
};

struct FuncCallExpr : public Expr {
	Expr* name;
	std::vector<Expr*> args;
	FuncCallExpr(Expr* _name, std::vector<Expr*> _args) {
		type = ExprType::FuncCall;
		name = _name;
		args = _args;
	}
};