#pragma once
#include<iostream>
#include<sstream>
#include<fstream>
#include<string>
#include<algorithm>
#include<unordered_set>
#include<set>
#include<unordered_map>
#include<vector>
#include<stack>

using namespace std;

class Production {
public:
	string left;
	vector<string>* right;

	Production();
	Production(string pr);
	~Production();
	string toString();
	Production* extendRight(string symbol);
	Production* clone();
};

class SimpleProd {
public:
	int id;
	int dot;
	set<string>* la;

	SimpleProd();
	~SimpleProd();
	bool equalTo(SimpleProd* p);
	bool equalCore(SimpleProd* p, bool ignoreDot = false);
	string toString();
	SimpleProd* clone();
};

class State {
public:
	int id;
	vector<SimpleProd*>* prods;
	unordered_map<string, int>* edges;

	State(int num);
	~State();
	bool equalTo(vector<SimpleProd*>* ns);
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
	unordered_map<string, unordered_set<string>*>* firstmap;

	State* newState();
	void toClosure(vector<SimpleProd*>* st);
	bool isTerminal(string name);
	unordered_set<string>* first(string symbol);
	//int prodid(Production* prod);
public:
	string toParsingTable(vector<Production*>* augGrammar, vector<string>* tokenList);

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

vector<string>* split(const string& str, const string& delimiter);
bool isWhiteSpace(const string& line);
void run();