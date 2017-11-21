#include"class.h"

Production::Production(string leftNT) {
	left = leftNT;
	right = new vector<string>;
	la = new unordered_set<string>;
	dot = 0;
}

Production::~Production() {
	delete right;
	delete la;
}

bool Production::equalTo(Production * p) {
	if (!equalCore(p)) return false;
	for (unordered_set<string>::iterator it = la->begin(); it != la->end(); it++) {
		if (find(p->la->begin(), p->la->end(), *it) == p->la->end()) return false;
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

Production * Production::clone() {
	Production* copy = new Production(left);
	copy->dot = dot;
	copy->la->insert(la->begin(),la->end());
	for (int i = 0; i < right->size(); i++) {
		copy->right->push_back((*right)[i]);
	}
	return copy;
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

bool State::equalTo(vector<Production*>* ns) {
	if (prods->size() != ns->size()) return false;
	for (int i = 0; i < ns->size(); i++) {
		//If no same production is found in this state
		if (none_of(prods->begin(), prods->end(), [ns, i](Production* p) {return ns->at(i)->equalTo(p); })) {
			return false;
		}
	}
	return true;
}

bool State::insert(Production * prod, int dot, string la) {
	Production * np = new Production(prod->left);
	for (int i = 0; i < prod->right->size(); i++) {

	}
	return false;
}

void Analyzer::toClosure(State * st) {
	unordered_map<string, unordered_set<string>*>* lamap = new unordered_map<string, unordered_set<string>*>;
	for (int i = 0; i < st->prods->size(); i++) {
		Production* pr = (*(st->prods))[i];
		//calculate First()
		if (pr->dot < pr->right->size() && !isTerminal(pr->right->at(pr->dot))) {
			if (lamap->find(pr->left) == lamap->end()) (*lamap)[pr->left] = new unordered_set<string>;
			if (pr->dot+1 == pr->right->size()) {
				(*lamap)[pr->left]->insert(pr->la->begin(), pr->la->end());
			}
			else {
				(*lamap)[pr->left]->insert(pr->right->at(pr->dot + 1));
			}
		}
	}
	for (unordered_map<string, unordered_set<string>*>::iterator it = lamap->begin(); it != lamap->end(); it++) {
		vector<int>* head = prodmap->at(it->first);
		for (int j = 0; j < head->size(); j++) {
			Production* pr = grammar->at(head->at(j));
			bool found = false;
			for (int k = 0; k < st->prods->size(); k++) {
				//Same production found, add lookaheads
				if ((*(st->prods))[k]->equalCore(pr)) {
					(*(st->prods))[k]->la->insert(it->second->begin(), it->second->end());
					found = true;
					break;
				}
			}
			//Insert production to this state
			if (!found) {
				Production* copy = pr->clone();
				copy->la->insert(it->second->begin(), it->second->end());
				st->prods->push_back(copy);
			}
		}
	}
	delete lamap;
}

bool Analyzer::isTerminal(string name) {
	return find(tokens->begin(), tokens->end(), name) != tokens->end();
}

vector<unordered_map<string, int>*> Analyzer::toParsingTable(vector<Production*>* augGrammar, vector<string>* tokenList) {
	grammar = augGrammar;
	tokens = tokenList;

	for (int i = 0; i < tokenList->size(); i++) {
		Production* p = (*grammar)[i];
		if (prodmap->find(p->left) != prodmap->end()) {
			(*prodmap)[p->left]->push_back(i);
		}
		else {
			(*prodmap)[p->left] = new vector<int>;
			(*prodmap)[p->left]->push_back(i);
		}
	}
}

Analyzer::Analyzer() {
	states = new vector<State*>;
	prodmap = new unordered_map<string, vector<int>*>;
	grammar = NULL;
}

Analyzer::~Analyzer() {
	delete states;
	delete prodmap;
}