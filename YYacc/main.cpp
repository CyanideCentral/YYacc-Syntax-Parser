#include"class.h"

void test() {
	//S1->S
	Production* p1 = new Production("S1");
	p1->extendRight("S");
	//S->C C
	Production* p2 = new Production("S");
	p1->extendRight("C")->extendRight("C");
	//C->c C
	Production* p3 = new Production("C");
	p1->extendRight("'c'")->extendRight("C");
	//C->d
	Production* p4 = new Production("C");
	p1->extendRight("'d'");
	vector<Production*> grammar{ p1, p2, p3, p4 };
	GrammarAnalyzer* a = new GrammarAnalyzer;
	//a->toParsingTable(&grammar);
}

int main() {
	run();
}