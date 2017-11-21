#pragma once
#include<iostream>
#include<sstream>
#include<string>
#include<algorithm>
#include<unordered_set>
#include<unordered_map>
#include<vector>

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
	bool equalCore(Production* p);
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

class Analyzer {
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
public:
	string toParsingTable(vector<Production*>* augGrammar, vector<string>* tokenList);

	Analyzer();
	~Analyzer();
};