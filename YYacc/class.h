#pragma once
#include<iostream>
#include<string>
#include<unordered_set>
#include<unordered_map>
#include<vector>

using namespace std;

class Production {
public:
	string left;
	vector<string>* right;
	int dot;
	string la;

	Production(string leftNT);
	~Production();
	bool equalTo(Production* p);
	bool equalCore(Production* p);
	Production* extendRight(string symbol);
};

class State {
public:
	int id;
	vector<Production*>* prods;
	unordered_map<string, int>* edges;

	State(int num);
	~State();
};

class Analyzer {
private:
	vector<State*>* states;
	vector<Production*>* grammar;
public:
	vector<unordered_map<string, int>*> toParsingTable(vector<Production*>* augGrammar);

	Analyzer();
	~Analyzer();
};