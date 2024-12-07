#pragma once
#include "pch.h"
#include "Lexer.h"
#include "Expression.h"
#include "Statement.h"
#include "Object.h"
#include "Environment.h"

class Interpreter {
public:
	Interpreter() {
		env = new Environment{ nullptr };
	}

	void execute(std::vector<Stmt*> statements) {
		INFO("Starting Execution");

		for (Stmt* statement : statements) {
			execute(statement);
		}
	}

	void execute(Stmt* statement) {
		switch (statement->type) {
		case StmtType::FUNC_DECL:
			DEB("Executing FuncDecl");
			executeFuncDeclStmt((FuncDeclStmt*)statement);
			break;
		case StmtType::CLASS_DECL:
			DEB("Executing ClassDecl");
			executeClassDeclStmt((ClassDeclStmt*)statement);
			break;
		case StmtType::RETURN:
			DEB("Executing Return");
			executeReturnStmt((ReturnStmt*)statement);
			break;

		case StmtType::BLOCK:
			DEB("Executing Block");
			executeBlockStmt((BlockStmt*)statement);
			break;
		case StmtType::EXPRESSION:
			DEB("Executing Expression");
			executeExprStmt((ExprStmt*)statement);
			break;
		case StmtType::PRINT:
			DEB("Executing Print");
			executePrintStmt((PrintStmt*)statement);
			break;
		case StmtType::IF:
			DEB("Executing IF");
			executeIfStmt((IfStmt*)statement);
			break;
		case StmtType::WHILE:
			DEB("Executing While");
			executeWhileStmt((WhileStmt*)statement);
			break;
		}
	}
private:

	FuncObject* getFuncObject(FuncDeclStmt* stmt) {
		return new FuncObject{ stmt->params, stmt->body };
	}

	void executeFuncDeclStmt(FuncDeclStmt* stmt) {
		FuncObject* funcObj = getFuncObject(stmt);
		env->setValue(stmt->name.value, funcObj);
	}
	void executeClassDeclStmt(ClassDeclStmt* stmt) {
		ClassObject* clsObj = new ClassObject{};
		for (FuncDeclStmt* method : stmt->methods) {
			FuncObject* funcObj = getFuncObject(method);
			clsObj->addMethod(method->name.value, funcObj);
		}
		env->setValue(stmt->name.value, clsObj);
	}

	void executeReturnStmt(ReturnStmt* stmt) {
		Object* retVal = stmt->retVal ? evaluate(stmt->retVal) : new NilObject{};
		env->setValueForce("__retval__", retVal);
		env->isDead = true;

	}

	void executeBlockStmt(BlockStmt* stmt) {
		for (Stmt* _stmt : stmt->stmts) {
			if (env->isDead) break;
			execute(_stmt);
		}
	}

	void executeExprStmt(ExprStmt* stmt) {
		evaluate(stmt->expr);
	}

	void executePrintStmt(PrintStmt* stmt) {
		Object* value = evaluate(stmt->expr);
		std::cout << toFloat(value) << std::endl;
	}

	void executeIfStmt(IfStmt* stmt) {
		Object* value = evaluate(stmt->condition);
		if (toFloat(value)) {
			execute(stmt->thenStmt);
		}
		else if (stmt->elseStmt) {
			execute(stmt->elseStmt);
		}
	}

	void executeWhileStmt(WhileStmt* stmt) {
		while (toFloat(evaluate(stmt->condition))) {
			execute(stmt->main);
		}
	}

	// Evaluate Expression
public:

	Object* evaluate(Expr* expr) {
		switch (expr->type)
		{
		case ExprType::Literal:
			return evaluateLiteral((LiteralExpr*)expr);
			//case ExprType::Unary:
				//return evaluateUnary((UnaryExpr*)expr)
		case ExprType::Binary:
			return evaluateBinary((BinaryExpr*)expr);
		case ExprType::Identifier:
			return evaluateIdentifier((IdentifierExpr*)expr);
		case ExprType::Assign:
			return evaluateAssign((AssignExpr*)expr);
		case ExprType::FuncCall:
			return evaluateFuncCall((FuncCallExpr*)expr);
		case ExprType::Get:
			return evaluateGet((GetExpr*)expr);
		default:
			ERR("Unknown Expr Type:");
			break;
		}
	}

private:
	Object* evaluateLiteral(LiteralExpr* expr) {
		return new FloatObject{ (float)std::stoi(expr->token.value) };
	}

	float toFloat(Object* obj) {
		if (obj->getType() == Object::Type::FLOAT) {
			return ((FloatObject*)obj)->value;
		}
		if (obj->getType() == Object::Type::NIL) {
			return 0;
		}
	}

	Object* evaluateBinary(BinaryExpr* expr) {


		float left = toFloat(evaluate(expr->left));
		float right = toFloat(evaluate(expr->right));
		float val = 0;
		switch (expr->oper.type) {
		case Token::Type::PLUS:
			val = left + right; break;
		case Token::Type::MINUS:
			val = left - right; break;
		case Token::Type::STAR:
			val = left * right; break;
		case Token::Type::DIV:
			val = left / right; break;
		case Token::Type::EQUAL_EQUAL:
			val = left == right ? 1 : 0; break;
		case Token::Type::BANG_EQUAL:
			val = left == right ? 0 : 1; break;
		case Token::Type::LESS_EQUAL:
			val = left <= right ? 1 : 0; break;
		case Token::Type::LESS:
			val = left < right ? 1 : 0; break;
		case Token::Type::GREAT_EQUAL:
			val = left >= right ? 1 : 0; break;
		case Token::Type::GREAT:
			val = left > right ? 1 : 0; break;


		default:
			break;
		}
		return new FloatObject{ val };

	}

	Object* evaluateIdentifier(IdentifierExpr* expr) {
		DEB("Getting Value {}", expr->token.value);
		return env->getValue(expr->token.value);
	}

	Object* evaluateAssign(AssignExpr* expr) {
		Object* value = evaluate(expr->value);
		ObjRef left = evaluateRef(expr->left);
		*(left.obj) = value;
		return value;
	}

	Object* evaluateGet(GetExpr* expr) {
		Object* lObject = evaluate(expr->left);
		return lObject->getAttr(expr->right.value);
	}
	ObjRef evaluateRef(Expr* expr) {
		if (expr->type == ExprType::Identifier) {
			IdentifierExpr* idExpr = (IdentifierExpr* )expr;
			return env->getRef(idExpr->token.value);
		}
		if (expr->type == ExprType::Get) {
			GetExpr* getExpr = (GetExpr*)expr;
			Object* lObject = evaluate(getExpr->left);
			return lObject->getAttrRef(getExpr->right.value);
		}
		ERR("Illegal Reference");
		return { nullptr };
	}


	Object* evaluateFuncCall(FuncCallExpr* expr) {
		// Includes object construction

		std::vector<Object*> arguments; 
		for (Expr* arg : expr->args) {
			arguments.push_back(evaluate(arg));
		}

		Object* _func = evaluate(expr->name);

		return _func->call(arguments, this);
	}

public:
	Environment* env;

};
