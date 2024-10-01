#pragma once
#include "pch.h"
#include "Lexer.h"
#include "Expression.h"
#include "Statements.h"

// Objects

struct Environment; // Forward Decl

struct Object {
	enum class Type {
		NIL,
		OBJ,
		FLOAT,
		FUNC,
		CLASS,
		METHOD,
	};
	virtual Object::Type getType() = 0;
};

struct NilObject : public Object {
	Type getType() override { return Type::NIL; }
};



struct FloatObject : public Object {
	float value;
	Type getType() override { return Type::FLOAT; }

	FloatObject(float value)
		: value(value) {}
};

struct FuncObject : public Object {
	std::vector<Token> params;
	BlockStmt* body;
	Object* binding;

	Type getType() override { return Type::FUNC; }

	FuncObject(const std::vector<Token>& params, BlockStmt* body)
		: params(params), body(body)
	{
	}
};

struct ClassObject : public Object {
	std::unordered_map<std::string, Object*> attrs;

	Type getType() override { return Type::CLASS; }

	ClassObject() {};
	void addMethod(std::string name, FuncObject* funcObj) {
		attrs[name] = funcObj;
	}

	Object* getAttr(std::string name, Object* ref) {
		if (attrs.count(name) != 0) {
			Object* obj = attrs[name];
			//if (obj->getType() == Object::Type::FUNC) {
			//	return new MethodObject{ (FuncObject* )obj, ref };
			//}
			return attrs[name];
		}
		spdlog::error("Cant find attribute in class");
		return nullptr;
	}

	void setAttr(std::string name, Object* obj) {
		attrs[name] = obj;
	}

	bool isAttr(std::string name) {
		return attrs.count(name) != 0;
	}


};

struct ObjObject : public Object {
	ClassObject* clsObj;
	std::unordered_map<std::string, Object*> attrs;
	Type getType() override { return Type::OBJ; }

	ObjObject(ClassObject* clsObj)
		:clsObj(clsObj) {}

	Object* getAttr(std::string name) {
		if (attrs.count(name) != 0) {
			return attrs[name];
		}
		return clsObj->getAttr(name, this);
	}

	void setAttr(std::string name, Object* obj) {
		if (isLocalAttr(name)) {
			attrs[name] = obj;
		}
		else {
			clsObj->setAttr(name, obj);
		}
	}

	bool isLocalAttr(std::string name) {
		return attrs.count(name) != 0;
	}

	bool isAttr(std::string name) {
		return isLocalAttr(name) || clsObj->isAttr(name);
	}
};

struct MethodObject : public Object {
	FuncObject* funcObj;
	Object* ref;
	Type getType() override { return Type::METHOD; }



	MethodObject(FuncObject* funcObj, Object* ref)
		: funcObj(funcObj), ref(ref)
	{
	}
};
