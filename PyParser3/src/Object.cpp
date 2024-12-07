#include "pch.h"
#include "Object.h"

#include "Interpreter.h"

Object* FuncObject::call(std::vector<Object*> arguments, Interpreter* interpreter)  {

	if (params.size() != arguments.size()) {
		ERR("Argument length not matching");
	}

	interpreter->env = new Environment{ interpreter->env };
	Environment* env = interpreter->env;
	// Set parameters
	for (size_t i = 0; i < params.size(); i++)
	{
		env->setValueForce(params[i].value, arguments[i]);
	}
	if (binding) {
		env->setValueForce("self", binding);
	}

	env->setValueForce("__retval__", new NilObject{});

	interpreter->execute(body);
	Object* retVal = env->getValue("__retval__");
	env = env->parent;

	return retVal;

}

Object* ClassObject::call(std::vector<Object*> arguments, Interpreter* interpreter){
	ObjObject* obj = new ObjObject{ this };
	Object* objInit = obj->getAttr("init");
	if (objInit) {
		objInit->call(arguments, interpreter);
	}

	return obj;
}