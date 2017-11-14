#include"class.h"

Production::Production() {
	left = "";
	right = new vector<string>;
	la = "";
	dot = 0;
}

Production::~Production() {
	delete right;
}

State::State(int num) {
	id = num;
	prods = new vector<Production*>;
}

State::~State() {
	delete prods;
}