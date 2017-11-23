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

bool Production::equalCore(Production * p, bool ignoreDot = false) {
	if (left.compare(p->left)) return false;
	if (right->size() != p->right->size()) return false;
	for (int i = 0; i < right->size(); i++) {
		if ((*right)[i].compare((*(p->right))[i])) return false;
	}
	if (!ignoreDot && dot != p->dot) return false;
	return true;
}

string Production::toString() {
	ostringstream os;
	os << left<<" -> ";
	for (int i = 0; i < right->size(); i++) {
		os << right->at(i) << " ";
	}
	os << ", ";
	for (unordered_set<string>::iterator it = la->begin(); it != la->end(); it++) {
		os << *it;
		if (distance(it, la->end())>1) os << "|";
	}
	return os.str();
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
	for (int i = 0; i < prods->size(); i++) {
		delete prods->at(i);
	}
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

State * GrammarAnalyzer::newState() {
	State* ns = new State(states->size());
	states->push_back(ns);
	return ns;
}

void GrammarAnalyzer::toClosure(State * st) {
	int change = 1;
	while (change) {
		change = 0;
		unordered_map<string, unordered_set<string>*>* lamap = new unordered_map<string, unordered_set<string>*>;
		for (int i = 0; i < st->prods->size(); i++) {
			Production* pr = (*(st->prods))[i];
			//calculate First()
			if (pr->dot < pr->right->size() && !isTerminal(pr->right->at(pr->dot))) {
				if (lamap->find(pr->left) == lamap->end()) (*lamap)[pr->left] = new unordered_set<string>;
				if (pr->dot + 1 == pr->right->size()) {
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
						int oldsize = (*(st->prods))[k]->la->size();
						(*(st->prods))[k]->la->insert(it->second->begin(), it->second->end());
						found = true;
						if ((*(st->prods))[k]->la->size()>oldsize) change++;
						break;
					}
				}
				//Insert production to this state
				if (!found) {
					Production* copy = pr->clone();
					copy->la->insert(it->second->begin(), it->second->end());
					st->prods->push_back(copy);
					change++;
				}
			}
		}
		delete lamap;
	}
}

bool GrammarAnalyzer::isTerminal(string name) {
	return (find(symbols->begin(), symbols->end(), name) - symbols->begin()) < num_tokens;
}

int GrammarAnalyzer::prodid(Production * prod) {
	for (int i = 0; i < grammar->size(); i++) {
		if (prod->equalCore(grammar->at(i), true)) {
			return i;
		}
	}
	return -1;
}

vector<unordered_map<string, int>*>* GrammarAnalyzer::toParsingTable(vector<Production*>* augGrammar, vector<string>* tokenList) {
	grammar = augGrammar;
	//Merge token list and non-terminals
	symbols->insert(symbols->end(), tokenList->begin(), tokenList->end());
	num_tokens = tokenList->size();
	for (int h = 0; h < grammar->size(); h++) {
		if (find(symbols->begin(), symbols->end(), grammar->at(h)->left) == symbols->end()) {
			symbols->push_back(grammar->at(h)->left);
		}
	}

	//construct production map
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

	//Insert I0 state
	states->push_back(new State(0));
	Production* firstpr = grammar->at(0)->clone();
	firstpr->la->insert("$");
	states->at(0)->prods->push_back(firstpr);

	//Expand the FA
	for (int i = 0; i < states->size(); i++) {
		State* cur = states->at(i);
		//Inferred edges and new states (might be identical to existent state)
		unordered_map<string, vector<Production*>*>* bp = new unordered_map<string, vector<Production*>*>;
		for (int j = 0; j < cur->prods->size(); j++) {
			Production* pr = cur->prods->at(j);
			//If end of expression or a terminal is reached, no edge is created
			if (pr->dot == pr->right->size()||isTerminal(pr->right->at(pr->dot))) continue;
			string next = pr->right->at(pr->dot);
			Production* shifted = pr->clone();
			shifted->dot++;
			if (bp->find(next) == bp->end()) {
				vector<Production*>* np = new vector<Production*>;
				np->push_back(shifted);
				bp->insert(pair<string, vector<Production*>*>(next, np));
			}
			else {
				bp->at(next)->push_back(shifted);
			}
		}
		//Check identical states, create new ones
		for (unordered_map<string, vector<Production*>*>::iterator it = bp->begin(); it != bp->end(); it++) {
			bool found = false;
			for (int k = 0; k < states->size(); k++) {
				if (states->at(k)->equalTo(it->second)) {
					cur->edges->insert(pair<string, int>(it->first, k));
					found = true;
					break;
				}
			}
			if (!found) {
				State* created = newState();
				created->prods->insert(created->prods->end(), it->second->begin(), it->second->end());
				toClosure(created);
				cur->edges->insert(pair<string, int>(it->first, created->id));
			}
			else {
				for (int l = 0; l < it->second->size(); l++) {
					delete it->second->at(l);
				}
			}
			delete it->second;
		}
		delete bp;
	}

	vector<unordered_map<string, int>*>* pt = new vector<unordered_map<string, int>*>;
	for (int i = 0; i < states->size(); i++) {
		int found = -1;
		vector<Production*>* prodlist = states->at(i)->prods;
		for (int j = 0; j < prodlist->size(); j++) {
			if (prodlist->at(j)->dot == prodlist->at(j)->right->size()) {
				if (found < 0) {
					found = j;
				}
				else {
					cout << "Reduce-reduce conflict between " << prodlist->at(found)->toString() << " and " << prodlist->at(j)->toString();
				}
			}
		}
		unordered_map<string, int>* ptItem = new unordered_map<string, int>;
		unordered_map<string, int>* edges = states->at(i)->edges;
		if (found > -1) {
			for (unordered_set<string>::iterator it = prodlist->at(found)->la->begin(); it != prodlist->at(found)->la->end(); it++) {
				ptItem->insert(pair<string, int>(*it, -prodid(prodlist->at(found))));
			}
		}
		for (unordered_map<string, int>::iterator it = edges->begin(); it != edges->end(); it++) {
			ptItem->insert(pair<string, int>(it->first, it->second));
		}
	}
	return pt;

	//Code to generate code
	/*ostringstream os;
	os << "vector<unordered_map<string, int>> pt = { ";
	vector<unordered_map<string, int>> pt = { unordered_map<string, int>{{"expr", 2}, {"id", -2}}, unordered_map<string, int>{} };
	for (int i = 0; i < states->size(); i++) {
		os << " unordered_map<string, int>{";
		int found = -1;
		vector<Production*>* prodlist = states->at(i)->prods;
		for (int j = 0; j < prodlist->size(); j++) {
			if (prodlist->at(j)->dot == prodlist->at(j)->right->size()) {
				if (found < 0) found = j;
				else {
					cout << "Reduce-reduce conflict between " << prodlist->at(found)->toString() << " and " << prodlist->at(j)->toString();
				}
			}
		}

		unordered_map<string, int>* edges = states->at(i)->edges;
		int mapsize = 0;
		if (found > -1) {
			for (unordered_set<string>::iterator it = prodlist->at(found)->la->begin(); it != prodlist->at(found)->la->end(); it++) {
				os << "{\"" << *it << "\", " << -prodid(prodlist->at(found)) << "}";
				if (distance(prodlist->at(found)->la->begin(), it) < (int)(prodlist->at(found)->la->size() - 1)) os << ", ";
				else os << "}";
				mapsize++;
			}
		}
		for (unordered_map<string, int>::iterator it = edges->begin(); it != edges->end(); it++) {
			if (it == edges->begin() && mapsize) os << ", ";
			os << "{\"" << it->first << "\", " << it->second << "}";
			if (distance(edges->begin(), it) < (int)(edges->size() - 1)) os << ", ";
			else os << "}";
			mapsize++;
		}
		if (i < states->size() - 1) os << ", \n";
		else os << " };\n";
	}
	return os.str();*/
}

GrammarAnalyzer::GrammarAnalyzer() {
	states = new vector<State*>;
	prodmap = new unordered_map<string, vector<int>*>;
	grammar = NULL;
}

GrammarAnalyzer::~GrammarAnalyzer() {
	delete states;
	delete prodmap;
}

Parser::Parser(vector<Production*>* augGrammar, vector<unordered_map<string, int>*>* parsingTable) {
	grammar = augGrammar;
	pt = parsingTable;
}

Parser::~Parser() {
	delete grammar;
	delete pt;
}

void Parser::parse(vector<string>* tokens, ostream & out) {
	int state = 0, i = 0;
	tokens->push_back("$");
	stack<int>* stateStack = new stack<int>;
	stateStack->push(1);
	stack<string>* symbolStack = new stack<string>;
	while(1) {
		int action = pt->at(stateStack->top())->at(tokens->at(i));
		//shift
		if (action >= 0) {
			stateStack->push(action);
			symbolStack->push(tokens->at(i));
			i++;
		}
		//reduce
		else {
			action = -action - 1;
			int plen = grammar->at(action)->right->size();
			for (int j = 0; j < plen; j++) {
				string back = symbolStack->top();
				//Production to reduce by contradicts with token in stack
				if (back.compare(grammar->at(action)->right->at(plen - 1 - j)) != 0) {
					cout << "Parsing error at token #" << i + 1 << ": " << tokens->at(i);
					out << " Parsing error encountered.";
					return;
				}
				symbolStack->pop();
				stateStack->pop();
			}
			//AKA 'acc'
			if (action == 0) {
				out << "ACC" << endl;
				cout << "Parsing complete." << endl;
				return;
			}
			out << grammar->at(i)->toString() << endl;
			symbolStack->push(grammar->at(i)->left);
			//Perform goto
			action = pt->at(stateStack->top())->at(tokens->at(i));
			stateStack->push(action);
		}
	}
}
