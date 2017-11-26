#include"class.h"

Production::Production() {
	right = new vector<string>;
}

Production::Production(string pr) {
	vector<string>* items = split(pr, " \t\r\n");
	left = items->at(0);
	items->erase(items->begin(), items->begin()+2);
	right = items;
}

Production::~Production() {
	delete right;
}

string Production::toString() {
	ostringstream os;
	os << left<<" -> ";
	for (int i = 0; i < right->size(); i++) {
		os << right->at(i) << " ";
	}
	return os.str();
}

Production * Production::extendRight(string symbol) {
	right->push_back(symbol);
	return this;
}

Production * Production::clone() {
	Production* copy = new Production();
	copy->left = copy->left;
	for (int i = 0; i < right->size(); i++) {
		copy->right->push_back((*right)[i]);
	}
	return copy;
}

SimpleProd::SimpleProd() {
	la = new set<string>;
	dot = 0;
}

SimpleProd::~SimpleProd() {
	delete la;
}

bool SimpleProd::equalTo(SimpleProd * p) {
	if (id != p->id) return false;
	if (dot != p->dot) return false;
	/*for (set<string>::iterator it = la->begin(); it != la->end(); it++) {
		if (find(p->la->begin(), p->la->end(), *it) == p->la->end()) return false;
	}*/
	return (*la) == (*(p->la));
}

bool SimpleProd::equalCore(SimpleProd * p, bool ignoreDot) {
	if (id != p->id) return false;
	if (!ignoreDot && dot != p->dot) return false;
	return true;
}

string SimpleProd::toString() {
	ostringstream os;
	os << id << ", ";
	for (set<string>::iterator it = la->begin(); it != la->end(); it++) {
		os << *it;
		if (distance(it, la->end())>1) os << "|";
	}
	return os.str();
}

SimpleProd * SimpleProd::clone() {
	SimpleProd* copy = new SimpleProd();
	copy->dot = dot;
	copy->la->insert(la->begin(), la->end());
	copy->id = id;
	return copy;
}

State::State(int num) {
	id = num;
	prods = new vector<SimpleProd*>;
	edges = new unordered_map<string, int>;
}

State::~State() {
	for (int i = 0; i < prods->size(); i++) {
		delete prods->at(i);
	}
	delete prods;
	delete edges;
}

bool State::equalTo(vector<SimpleProd*>* ns) {
	if (prods->size() != ns->size()) return false;
	for (int i = 0; i < ns->size(); i++) {
		//If no same production is found in this state
		/*if (none_of(prods->begin(), prods->end(), [ns, i](SimpleProd* p) {return ns->at(i)->equalTo(p); })) {
			return false;
		}*/
		int flag = 0;
		for (int j = 0; j < prods->size(); j++) {
			if (ns->at(i)->equalTo(prods->at(j))) {
				flag = 1;
				break;
			}
		}
		if (!flag) return false;
	}
	return true;
}

State * GrammarAnalyzer::newState() {
	State* ns = new State(states->size());
	states->push_back(ns);
	return ns;
}

unordered_set<string>* GrammarAnalyzer::first(string symbol) {

	unordered_set<string>* fs = new unordered_set<string>;
	if (isTerminal(symbol)) {
		fs->insert(symbol);
		return fs;
	}
	unordered_set<string>* vts = new unordered_set<string>;
	vts->insert(symbol);
	while (1) {
		int size = vts->size();
		for (int i = 0; i < grammar->size(); i++) {
			if (vts->find(grammar->at(i)->left) != vts->end() && !isTerminal(grammar->at(i)->right->at(0))) {
				vts->insert(grammar->at(i)->right->at(0));
			}
		}
		if (vts->size() == size) break;
	}
	for (int i = 0; i < grammar->size(); i++) {
		if (vts->find(grammar->at(i)->left) != vts->end() && isTerminal(grammar->at(i)->right->at(0))) {
			fs->insert(grammar->at(i)->right->at(0));
		}
	}
	return fs;
}

void GrammarAnalyzer::toClosure(vector<SimpleProd*>* st) {
	int change = 1;
	while (change) {
		change = 0;
		unordered_map<string, unordered_set<string>*>* lamap = new unordered_map<string, unordered_set<string>*>;
		for (int i = 0; i < st->size(); i++) {
			SimpleProd* spr = (*st)[i];
			Production* pr = grammar->at(spr->id);
			if (spr->dot == pr->right->size()) continue;
			string reach = pr->right->at(spr->dot);
			//calculate First()
			if (!isTerminal(reach)) {
				if (lamap->find(reach) == lamap->end()) {
					lamap->insert(pair<string, unordered_set<string>*>(reach, new unordered_set<string>));
				}
				if (spr->dot + 1 == pr->right->size()) {
					(*lamap)[reach]->insert(spr->la->begin(), spr->la->end());
				}
				else {
					string next = pr->right->at(spr->dot + 1);
					if (isTerminal(next)) {
						(*lamap)[reach]->insert(next);
					}
					else {
						unordered_set<string>* fs = firstmap->at(next);
						(*lamap)[reach]->insert(fs->begin(), fs->end());
					}
				}
			}
		}
		for (unordered_map<string, unordered_set<string>*>::iterator it = lamap->begin(); it != lamap->end(); it++) {
			vector<int>* head = prodmap->at(it->first);
			for (int j = 0; j < head->size(); j++) {
				SimpleProd* pr = new SimpleProd();
				pr->id = head->at(j);
				bool found = false;
				for (int k = 0; k < st->size(); k++) {
					//Same production found, add lookaheads
					if ((*st)[k]->equalCore(pr)) {
						int oldsize = (*st)[k]->la->size();
						(*st)[k]->la->insert(it->second->begin(), it->second->end());
						found = true;
						if ((*st)[k]->la->size()>oldsize) change++;
						break;
					}
				}
				//Insert production to this state
				if (!found) {
					SimpleProd* copy = pr->clone();
					copy->la->insert(it->second->begin(), it->second->end());
					st->push_back(copy);
					change++;
				}
			}
			delete it->second;
		}
		delete lamap;
	}
}

bool GrammarAnalyzer::isTerminal(string name) {
	if ((find(symbols->begin(), symbols->end(), name) - symbols->begin()) < num_tokens) return true;
	if (name.length() == 3 && name[0] == '\'' && name[2] == '\'') return true;
	return false;
}

string GrammarAnalyzer::toParsingTable(vector<Production*>* augGrammar, vector<string>* tokenList) {
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
	for (int i = 0; i < grammar->size(); i++) {
		Production* p = (*grammar)[i];
		if (prodmap->find(p->left) != prodmap->end()) {
			(*prodmap)[p->left]->push_back(i);
		}
		else {
			(*prodmap)[p->left] = new vector<int>;
			(*prodmap)[p->left]->push_back(i);
		}
	}

	//construct First() map
	for (int i = 0; i < symbols->size(); i++) {
		firstmap->insert(pair<string, unordered_set<string>*>(symbols->at(i), first(symbols->at(i))));
	}

	//Insert I0 state
	states->push_back(new State(0));
	SimpleProd* firstpr = new SimpleProd();
	firstpr->id = 0;
	firstpr->la->insert("'$'");
	states->at(0)->prods->push_back(firstpr);
	toClosure(states->at(0)->prods);

	//Expand the FA
	for (int i = 0; i < states->size(); i++) {
		State* cur = states->at(i);
		//Inferred edges and new states (might be identical to existent state)
		unordered_map<string, vector<SimpleProd*>*>* bp = new unordered_map<string, vector<SimpleProd*>*>;
		for (int j = 0; j < cur->prods->size(); j++) {
			SimpleProd* pr = cur->prods->at(j);
			//If end of expressionis reached, no edge is created
			if (pr->dot == grammar->at(pr->id)->right->size()) continue;
			string next = grammar->at(pr->id)->right->at(pr->dot);
			SimpleProd* shifted = pr->clone();
			shifted->dot++;
			if (bp->find(next) == bp->end()) {
				vector<SimpleProd*>* np = new vector<SimpleProd*>;
				np->push_back(shifted);
				bp->insert(pair<string, vector<SimpleProd*>*>(next, np));
			}
			else {
				bp->at(next)->push_back(shifted);
			}
		}
		//Check identical states, create new ones
		for (unordered_map<string, vector<SimpleProd*>*>::iterator it = bp->begin(); it != bp->end(); it++) {
			bool found = false;
			toClosure(it->second);
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
				cur->edges->insert(pair<string, int>(it->first, created->id));
				cout << "New state: " << created->id << endl;
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
		vector<SimpleProd*>* prodlist = states->at(i)->prods;
		for (int j = 0; j < prodlist->size(); j++) {
			if (prodlist->at(j)->dot == grammar->at(prodlist->at(j)->id)->right->size()) {
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
			for (set<string>::iterator it = prodlist->at(found)->la->begin(); it != prodlist->at(found)->la->end(); it++) {
				ptItem->insert(pair<string, int>(*it, - prodlist->at(found)->id - 1));
			}
		}
		for (unordered_map<string, int>::iterator it = edges->begin(); it != edges->end(); it++) {
			ptItem->insert(pair<string, int>(it->first, it->second));
		}
		pt->push_back(ptItem);
	}

	//Code to generate code
	ostringstream os;
	os << "vector<unordered_map<string, int>*>* pt = new vector<unordered_map<string, int>*>(); " << endl;
	for (int i = 0; i < pt->size(); i++) {
		os << endl << "pt->push_back(new unordered_map<string, int>{";
		unordered_map<string, int>* edges = pt->at(i);
		int j = 0;
		for (unordered_map<string, int>::iterator it = edges->begin(); it != edges->end(); it++) {
			os << "{\"" << it->first << "\", " << it->second << "}";
			if (j < edges->size() - 1) os << ", ";
			else os << "} ";
			j++;
		}
		os << ");" << endl;
	}
	return os.str();
}

GrammarAnalyzer::GrammarAnalyzer() {
	states = new vector<State*>;
	symbols = new vector<string>;
	prodmap = new unordered_map<string, vector<int>*>;
	firstmap = new unordered_map<string, unordered_set<string>*>;
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
	int state = 0, i = 0, action = 0;
	tokens->push_back("'$'");
	stack<int>* stateStack = new stack<int>;
	stateStack->push(0);
	stack<string>* symbolStack = new stack<string>;
	try {
		while (1) {
			action = pt->at(stateStack->top())->at(tokens->at(i));
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
				out << grammar->at(action)->toString() << endl;
				symbolStack->push(grammar->at(action)->left);
				//Perform goto
				action = pt->at(stateStack->top())->at(symbolStack->top());
				stateStack->push(action);
			}
		}
	}
	catch (out_of_range e) {
		cout << "Error caused by token " << i + 1 << ": " << tokens->at(i) << endl << "action: " << action << endl;
		while (!symbolStack->empty()) {
			cout << symbolStack->top()<<" ";
			symbolStack->pop();
		}
		cout << endl;
		while (!stateStack->empty()) {
			cout << stateStack->top() << " ";
			stateStack->pop();
		}
		return;
	}
}
