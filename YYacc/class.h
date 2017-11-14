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

	Production();
	~Production();
};

class State {
public:
	int id;
	vector<Production*>* prods;

	State(int num);
	~State();
};