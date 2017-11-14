#include"class.h"

Production::Production(string leftNT) {
	left = leftNT;
	right = new vector<string>;
	la = "";
	dot = 0;
}

Production::~Production() {
	delete right;
}

bool Production::equalTo(Production * p) {
	if (!equalCore(p)) return false;
	for (int i = 0; i < la.size(); i++) {
		if (p->la.find_first_of(la[i]) == string::npos) return false;
	}
	return true;
}

bool Production::equalCore(Production * p) {
	if (left.compare(p->left)) return false;
	if (right->size() != p->right->size()) return false;
	for (int i = 0; i < right->size(); i++) {
		if ((*right)[i].compare((*(p->right))[i])) return false;
	}
	if (dot != p->dot) return false;
	return true;
}

Production * Production::extendRight(string symbol) {
	right->push_back(symbol);
	return this;
}

State::State(int num) {
	id = num;
	prods = new vector<Production*>;
	edges = new unordered_map<string, int>;
}

State::~State() {
	delete prods;
	delete edges;
}

vector<unordered_map<string, int>*> Analyzer::toParsingTable(vector<Production*>* augGrammar) {
	grammar = augGrammar;
}

Analyzer::Analyzer() {
	states = new vector<State*>;
	grammar = NULL;
}

Analyzer::~Analyzer() {
	delete states;
}