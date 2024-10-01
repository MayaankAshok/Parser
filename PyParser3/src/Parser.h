#pragma once
#include "pch.h"
#include "Lexer.h"
#include "Expression.h"
#include "Statements.h"
#include "Objects.h"

// Environment

struct Environment {
	Environment* parent; 
	bool isDead = false;
	std::unordered_map<std::string, Object*> values;

	Environment(Environment* _parent): parent(_parent){}

	bool isLocalValue(std::string name) {
		return values.count(name) != 0;
	}
	bool isValue(std::string name) {
		if (isLocalValue(name)) {
			return true;
		}
		if (parent == nullptr) return false;
		return parent->isValue(name);
	}

	Object* getValue(std::string name) {
		if (!isValue(name)) {
			std::cout << "Undefined variable " << name << std::endl;
			return new NilObject{};
		}
		if (isLocalValue(name))
			return values[name];
		return parent->getValue(name);
	}

	void setValue(std::string name, Object* value) {
		if (isLocalValue(name) || !isValue(name)) {
			values[name] = value;
		}
		else {
			parent->setValue(name, value);
		}
	}

	void setValueForce(std::string name, Object* value) {
		values[name] = value;
	}

	Environment* getRootEnv() {
		if (parent == nullptr) {
			return this;
		}
		return parent->getRootEnv();
	}

	static Environment* Copy(Environment* env) {
		if (env == nullptr) return nullptr;

		Environment newEnv = new Environment{Copy(env->parent)};
		newEnv.isDead = env->isDead;
		newEnv.values = env->values;

	}
};

class Parser {
public:
	Parser(std::vector<Token> _tokens) {
		tokens = _tokens;
		env = new Environment{ nullptr };
		INFO("Starting Parsing");

		std::vector<Stmt*> statements = parse();
		//std::cout << "Hey" << std::endl;
		INFO("Starting Execution");
		execute(statements);

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
			if (expr->type == ExprType::Identifier) {
				Token name = ((IdentifierExpr*)expr)->token;
				return new AssignExpr(name, value);
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
				expr = new BinaryExpr(expr, peek(-1), new IdentifierExpr(peek()));
				advance();
			}
			return expr;
		}
		if (match(Token::Type::L_PAREN)) {
			Expr* expr = expression();
			match(Token::Type::R_PAREN);
		}
	}

	// Execute Statement

	void execute(std::vector<Stmt*> statements) {
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
	
	FuncObject* getFuncObject(FuncDeclStmt* stmt) {
		return new FuncObject{ stmt->params, stmt->body};
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
		default:
			break;
		}
	}

	Object* evaluateLiteral(LiteralExpr* expr) {
		return new FloatObject{ (float) std::stoi(expr->token.value) };
	}

	Object* evaluateDot(Expr* left, Expr* right) {
		Object* _leftObj = evaluate(left);
		if (_leftObj->getType() != Object::Type::OBJ) {
			std::cout << "ERROR : Cant apply dot operator, left error " << std::endl;
		}
		ObjObject* leftObj = (ObjObject*)_leftObj;

		if (right->type != ExprType::Identifier) {
			std::cout << "ERROR : Cant apply dot operator, right error " << std::endl;
		}
		std::string attr = ((IdentifierExpr*)right)->token.value;
		DEB("Getting attr {}", attr);

		Object* _rightObj = leftObj->getAttr(attr);
		if (_rightObj->getType() == Object::Type::FUNC) {
			FuncObject* rightObj = (FuncObject*)_rightObj;
			rightObj->binding = leftObj;
			return rightObj ;
		}

		return _rightObj;
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

		if (expr->oper.type == Token::Type::DOT){
			return evaluateDot(expr->left, expr->right);
		}

		float left = toFloat(evaluate(expr->left));
		float right= toFloat(evaluate(expr->right));
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
		env->setValue(expr->name.value, value);
		return value;
	}

	Object* ConstructObject(ClassObject* clsObj, std::vector<Expr*> args ) {
		return new ObjObject{ clsObj };
	}

	Object* evaluateFuncCall(FuncCallExpr* expr) {
		// Includes object construction

		Object* _func = evaluate(expr->name);

		if (_func->getType() == Object::Type::CLASS) {
			// Object Construction;
			return ConstructObject((ClassObject*)_func, expr->args);
		}
		//if (_func.)
		if (_func->getType() != Object::Type::FUNC) {
			std::cout << "Object not Callable" << std::endl;
		}
		FuncObject* func = (FuncObject*)_func;
		
		if (func->params.size() != expr->args.size()) {
			std::cout << "Argument length not matching" << std::endl;
		}
		
		env = new Environment{ env};
		// Set parameters
		for (size_t i = 0; i < func->params.size(); i++)
		{
			env->setValueForce(func->params[i].value, evaluate(expr->args[i]));
		}
		if (func->binding) {
			env->setValueForce("self", func->binding);
		}

		env->setValueForce("__retval__", new NilObject{});

		executeBlockStmt(func->body);
		Object* retVal = env->getValue("__retval__");
		env = env->parent;

		return retVal;
	}

	// Print Representation

	std::string repr(Expr* expr) {
		switch (expr->type)
		{
		case ExprType::Literal:
			return reprLiteral((LiteralExpr*)expr);
			//case ExprType::Unary:
				//return reprUnary((UnaryExpr*)expr)
		case ExprType::Binary:
			return reprBinary((BinaryExpr*)expr);
		default:
			break;
		}
	}

	std::string reprLiteral(LiteralExpr* expr) {
		return expr->token.value;
	}
	
	std::string reprBinary(BinaryExpr* expr) {
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
	Environment* env;
};