#pragma once
#include<iostream>
#include<sstream>
#include<string>
#include<algorithm>
#include<unordered_set>
#include<unordered_map>
#include<vector>
#include<stack>

using namespace std;

class Production {
public:
	string left;
	vector<string>* right;
	int dot;
	unordered_set<string>* la;

	Production(string leftNT);
	~Production();
	bool equalTo(Production* p);
	bool equalCore(Production* p, bool ignoreDot = false);
	string toString();
	Production* extendRight(string symbol);
	Production* clone();
};

class State {
public:
	int id;
	vector<Production*>* prods;
	unordered_map<string, int>* edges;

	State(int num);
	~State();
	bool equalTo(vector<Production*>* ns);
	bool insert(Production* prod, int dot = -1, string la = "");
};

class GrammarAnalyzer {
private:
	int num_tokens;
	vector<State*>* states;
	vector<Production*>* grammar;
	vector<string>* symbols;
	//Map from non-terminal to its productions
	unordered_map<string, vector<int>*>* prodmap;
	unordered_map<string, int>* symbolmap;

	State* newState();
	void toClosure(State* st);
	bool isTerminal(string name);
	int prodid(Production* prod);
public:
	vector<unordered_map<string, int>*>* toParsingTable(vector<Production*>* augGrammar, vector<string>* tokenList);

	GrammarAnalyzer();
	~GrammarAnalyzer();
};

class Parser {
private:
	vector<Production*>* grammar;
	vector<unordered_map<string, int>*>* pt;
public:
	Parser(vector<Production*>* augGrammar, vector<unordered_map<string, int>*>* parsingTable);
	~Parser();
	void parse(vector<string>* tokens, ostream& out);
};