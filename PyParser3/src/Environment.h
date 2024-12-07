#pragma once
#include "pch.h"
#include "Lexer.h"
#include "Expression.h"
#include "Statement.h"
#include "Object.h"


// Environment

struct Environment {
	Environment* parent;
	bool isDead = false;
	std::unordered_map<std::string, Object*> values;

	Environment(Environment* _parent) : parent(_parent) {}

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
			ERR("Undefined variable {}", name);
			return new NilObject{};
		}
		if (isLocalValue(name))
			return values[name];
		return parent->getValue(name);
	}

	ObjRef getRef(std::string name) {
		if (!isValue(name)) {
			values[name] = new NilObject{};
			return { &values[name] };
		}
		if (isLocalValue(name))
			return { &values[name] };
		return parent->getRef(name);
	}

	std::unordered_map<std::string, Object*>* getMap(std::string name) {
		if (!isValue(name)) {
			return &values;
		}
		if (isLocalValue(name))
			return &values;
		return parent->getMap(name);
	}

	//Object* setRef(std::string name) {
	//	if (!isValue(name)) {
	//		values[name] = nullptr;
	//		return values[name];
	//	}
	//	if (isLocalValue(name))
	//		return new RefObject{ &values[name] };
	//	return parent->getRef(name);
	//}

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

		Environment newEnv = new Environment{ Copy(env->parent) };
		newEnv.isDead = env->isDead;
		newEnv.values = env->values;

	}
};
