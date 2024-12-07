#pragma once
#include "pch.h"
#include "Lexer.h"
#include "Expression.h"
#include "Statement.h"

// Objects
class Interpreter;
struct Environment; // Forward Decl

struct Object;

struct ObjRef {
	Object** obj;
};

struct Object {
	enum class Type {
		NIL,
		OBJ,
		FLOAT,
		FUNC,
		CLASS,
		METHOD,
		REF,
	};
	virtual Object::Type getType() = 0;
	virtual Object* getAttr(std::string name) {
		ERR("Illegal Get Attr {}", name);
		return nullptr;
	}
	virtual ObjRef getAttrRef(std::string name) {
		ERR("Illegal Get Attr Ref {}", name);
		return { nullptr };
	}
	virtual Object* call(std::vector<Object*> arguments, Interpreter* interpreter) {
		ERR("Illegal call");
		return nullptr;

	}
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
	Object* call(std::vector<Object*> arguments, Interpreter* interpreter)override;

};


struct ClassObject : public Object {
	std::unordered_map<std::string, Object*> attrs;

	Type getType() override { return Type::CLASS; }

	ClassObject() {};
	void addMethod(std::string name, FuncObject* funcObj) {
		attrs[name] = funcObj;
	}

	Object* getAttr(std::string name) override {
		if (attrs.count(name) != 0) {
			Object* obj = attrs[name];
			//if (obj->getType() == Object::Type::FUNC) {
			//	return new MethodObject{ (FuncObject* )obj, ref };
			//}
			return attrs[name];
		}
		return nullptr;
		//ERR("Illegal Get Attr {}", name);
	}

	void setAttr(std::string name, Object* obj) {
		attrs[name] = obj;
	}

	bool isAttr(std::string name) {
		return attrs.count(name) != 0;
	}

	Object* call(std::vector<Object*> arguments, Interpreter* interpreter) override ;




};

struct ObjObject : public Object {
	ClassObject* clsObj;
	std::unordered_map<std::string, Object*> attrs;
	Type getType() override { return Type::OBJ; }

	ObjObject(ClassObject* _clsObj)
		:clsObj(_clsObj) {
	}

	Object* getAttr(std::string name) override {
		if (attrs.count(name) != 0) {
			return attrs[name];
		}
		Object* clsAttr = clsObj->getAttr(name);
		if (clsAttr->getType() == Object::Type::FUNC) {
			FuncObject* clsFunc = (FuncObject*)clsAttr;
			clsFunc->binding = this;
			return clsFunc;
		}
		return clsAttr;
	}
	ObjRef getAttrRef(std::string name) override {
		if (attrs.count(name) != 0) {
			return { &attrs[name] };
		}
		attrs[name] = new NilObject{};
		return { &attrs[name] };

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

struct AssignEnv {
	std::unordered_map<std::string, Object*>* enclosing;
	std::string name;
};