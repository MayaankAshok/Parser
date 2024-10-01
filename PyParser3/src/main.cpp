#include "pch.h"
#include "Lexer.h"
#include "Parser.h"



void configLogger() {
	spdlog::set_pattern("[%^%l%$] %v");
	spdlog::set_level(spdlog::level::info);
}

int main() {
	configLogger();
	std::string cwd = "C:\\Mayaank\\Programming\\Master\\C++\\PyParser3\\PyParser3\\src\\";
	Lexer lexer{ cwd + "program.txt" };
	Parser parser{ lexer.getTokens() };

}
