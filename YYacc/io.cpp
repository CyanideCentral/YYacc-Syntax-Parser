#include"class.h"

vector<string>* split(const string& str, const string& delimiters) {
	string s = "";
	vector<string>* substrs = new vector<string>;
	for (int ptr = 0; ptr < str.length(); ptr++) {
		//One of delimiters found
		if (delimiters.find_first_of(str[ptr]) != string::npos) {
			if (s.length() > 0) {
				substrs->push_back(s);
				s = string();
			}
		}
		else {
			s.push_back(str[ptr]);
		}
	}
	if (s.length() > 0) {
		substrs->push_back(s);
	}
	return substrs;
}

bool isWhiteSpace(const string& line) {
	string ws = " \t\r\n";
	for (int i = 0; i < line.length(); i++) {
		if (ws.find_first_of(line[i]) == string::npos) return false;
	}
	return true;
}

void run() {
	ifstream ifs;
	ofstream ofs;
	string fname;
	int ln = 0;
	//Request .y file
	while (1) {
		cout << "Please enter the path of yacc file:" << endl;
		getline(cin, fname);
		ifs.open(fname, ifstream::in);
		if (ifs.is_open()) break;
		cout << "File not found." << endl;
	}

	vector<string>* tokenlist = new vector<string>;
	vector<Production*>* grammar = new vector<Production*>;
	string line;
	//Read tokens and start symbol
	while (1) {
		getline(ifs, line);
		ln++;
		if (isWhiteSpace(line)) {
			continue;
		}
		if (line.substr(0, 2).compare("%%") == 0) {
			break;
		}
		else if (line.substr(0, 6).compare("%token") == 0) {
			vector<string>* sublist = split(line.substr(6), "\t ");
			tokenlist->insert(tokenlist->end(), sublist->begin(), sublist->end());
			delete sublist;
		}
		else if (line.substr(0, 6).compare("%start") == 0) {
			if (grammar->size() > 0) {
				cout << "Warning: multiple start symbols" << endl;
			}
			else {
				//Add production S'->S
				vector<string>* sublist = split(line.substr(6), " \t");
				Production* first = new Production();
				first->left = "S'";
				first->right->push_back(sublist->at(0));
				grammar->push_back(first);
				delete sublist;
			}
		}
	}

	//Read Productions
	string current = "";
	while (1) {
		getline(ifs, line);
		ln++;
		if (ifs.eof()) break;
		//Delete comment section
		int comment = line.find("/*");
		if (comment != string::npos) {
			line = line.substr(0, comment);
		}
		if (isWhiteSpace(line)) continue;
		//New left symbol
		if (line[0] != '\t'&&line[0] != ' ') {
			vector<string>* strs = split(line, " \t");
			if (line.substr(0, 2).compare("%%") == 0) {
				break;
			}
			current = strs->at(0);
		}
		if (current.empty()) {
			cout << "Warning: wrong syntax at line " << ln << endl;
		}
		//End of this left symbol's productions
		if (line.substr(0, 2).compare("\t;") == 0) {
			current = "";
		}
		//New production
		if (line.substr(0, 2).compare("\t:") == 0 || line.substr(0, 2).compare("\t|") == 0) {
			Production* p = new Production();
			p->left = current;
			delete p->right;
			p->right = split(line.substr(2), " \t");
			grammar->push_back(p);
			//cout << p->toString();
		}
	}

	string codeHead = "";
	while (1) {
		getline(ifs, line);
		ln++;
		if (ifs.eof()) break;
		codeHead.append(line);
	}
	ifs.close();

	GrammarAnalyzer* analyzer = new GrammarAnalyzer();
	string pt = analyzer->toParsingTable(grammar, tokenlist);
	
	while (1) {
		cout << "Path of file to save source code of generated syntax analyzer:" << endl;
		getline(cin, fname);
		ofs.open(fname, ofstream::out | ofstream::trunc);
		if (ofs.is_open()) break;
		cout << "Invalid path." << endl;
	}

	//Code declaring production vector
	ofs << "#include<iostream>\n#include<sstream>\n#include<fstream>\n#include<string>\n#include<algorithm>\n#include<unordered_set>\n#include<unordered_map>\n#include<vector>\n#include<stack>\n\nusing namespace std;\n\nvector<string>* split(const string& str, const string& delimiters) {\n\tstring s = \"\";\n\tvector<string>* substrs = new vector<string>;\n\tfor (int ptr = 0; ptr < str.length(); ptr++) {\n\t\t//One of delimiters found\n\t\tif (delimiters.find_first_of(str[ptr]) != string::npos) {\n\t\t\tif (s.length() > 0) {\n\t\t\t\tsubstrs->push_back(s);\n\t\t\t\ts = string();\n\t\t\t}\n\t\t}\n\t\telse {\n\t\t\ts.push_back(str[ptr]);\n\t\t}\n\t}\n\tif (s.length() > 0) {\n\t\tsubstrs->push_back(s);\n\t}\n\treturn substrs;\n}\n\nbool isWhiteSpace(const string& line) {\n\tstring ws = \" \\t\\r\\n\";\n\tfor (int i = 0; i < line.length(); i++) {\n\t\tif (ws.find_first_of(line[i]) == string::npos) return false;\n\t}\n\treturn true;\n}\n\nclass Production {\npublic:\n\tstring left;\n\tvector<string>* right;\n\n\tProduction(string leftNT);\n\t~Production();\n\tstring toString();\n\tProduction* extendRight(string symbol);\n\tProduction* clone();\n};\n\nProduction::Production(string leftNT) {\n\tleft = leftNT;\n\tright = new vector<string>;\n}\n\nProduction::~Production() {\n\tdelete right;\n}\n\nstring Production::toString() {\n\tostringstream os;\n\tos << left << \" -> \";\n\tfor (int i = 0; i < right->size(); i++) {\n\t\tos << right->at(i) << \" \";\n\t}\n\treturn os.str();\n}\n\nProduction * Production::extendRight(string symbol) {\n\tright->push_back(symbol);\n\treturn this;\n}\n\nProduction * Production::clone() {\n\tProduction* copy = new Production(left);\n\tfor (int i = 0; i < right->size(); i++) {\n\t\tcopy->right->push_back((*right)[i]);\n\t}\n\treturn copy;\n}\n\nclass Parser {\nprivate:\n\tvector<Production*>* grammar;\n\tvector<unordered_map<string, int>*>* pt;\npublic:\n\tParser(vector<Production*>* augGrammar, vector<unordered_map<string, int>*>* parsingTable);\n\t~Parser();\n\tvoid parse(vector<string>* tokens, ostream& out);\n};\n\nParser::Parser(vector<Production*>* augGrammar, vector<unordered_map<string, int>*>* parsingTable) {\n\tgrammar = augGrammar;\n\tpt = parsingTable;\n}\n\nParser::~Parser() {\n\tdelete grammar;\n\tdelete pt;\n}\n\nvoid Parser::parse(vector<string>* tokens, ostream & out) {\n\tint state = 0, i = 0;\n\ttokens->push_back(\"'$'\");\n\tstack<int>* stateStack = new stack<int>;\n\tstateStack->push(0);\n\tstack<string>* symbolStack = new stack<string>;\n\ttry {\n\t\twhile (1) {\n\t\t\tint action = pt->at(stateStack->top())->at(tokens->at(i));\n\t\t\t//shift\n\t\t\tif (action >= 0) {\n\t\t\t\tstateStack->push(action);\n\t\t\t\tsymbolStack->push(tokens->at(i));\n\t\t\t\ti++;\n\t\t\t}\n\t\t\t//reduce\n\t\t\telse {\n\t\t\t\taction = -action - 1;\n\t\t\t\tint plen = grammar->at(action)->right->size();\n\t\t\t\tfor (int j = 0; j < plen; j++) {\n\t\t\t\t\tstring back = symbolStack->top();\n\t\t\t\t\t//Production to reduce by contradicts with token in stack\n\t\t\t\t\tif (back.compare(grammar->at(action)->right->at(plen - 1 - j)) != 0) {\n\t\t\t\t\t\tcout << \"Parsing error at token #\" << i + 1 << \": \" << tokens->at(i);\n\t\t\t\t\t\tout << \" Parsing error encountered.\";\n\t\t\t\t\t\treturn;\n\t\t\t\t\t}\n\t\t\t\t\tsymbolStack->pop();\n\t\t\t\t\tstateStack->pop();\n\t\t\t\t}\n\t\t\t\t//AKA 'acc'\n\t\t\t\tif (action == 0) {\n\t\t\t\t\tout << \"ACC\" << endl;\n\t\t\t\t\tcout << \"Parsing complete.\" << endl;\n\t\t\t\t\treturn;\n\t\t\t\t}\n\t\t\t\tout << grammar->at(action)->toString() << endl;\n\t\t\t\tsymbolStack->push(grammar->at(action)->left);\n\t\t\t\t//Perform goto\n\t\t\t\taction = pt->at(stateStack->top())->at(symbolStack->top());\n\t\t\t\tstateStack->push(action);\n\t\t\t}\n\t\t}\n\t}\n\tcatch (out_of_range e) {\n\t\tcout << \"Error caused by input.\" << endl;\n\t\treturn;\n\t}\n}\n\nint main() {\n";
	ofs << endl << "vector<Production*>* grammar = new vector<Production*>();" << endl;
	for (int i = 0; i < grammar->size(); i++) {
		ofs << "grammar->push_back(new Production(\"" << grammar->at(i)->toString() << "\"));";
	}

	ofs << pt;
	//main function
	ofs << "\n\tifstream ifs;\n\tofstream ofs;\n\tstring fname;\n\tParser* parser = new Parser(grammar, pt);\n\twhile (1) {\n\t\twhile (1) {\n\t\t\tcout << \"Path of tokens file (or -q to quit):\" << endl;\n\t\t\tgetline(cin, fname);\n\t\t\tif (fname.compare(\"-q\") == 0) return 1;\n\t\t\tifs.open(fname);\n\t\t\tif (ifs.is_open()) break;\n\t\t\tcout << \"Invalid path.\" << endl;\n\t\t}\n\t\tbool toConsole = false;\n\t\twhile (1) {\n\t\t\tcout << \"Path of file to save sequence of reductions (or -c to show in console):\" << endl;\n\t\t\tgetline(cin, fname);\n\t\t\tif (fname.compare(\"-c\") == 0) {\n\t\t\t\ttoConsole = true;\n\t\t\t\tbreak;\n\t\t\t}\n\t\t\tofs.open(fname, ofstream::out | ofstream::trunc);\n\t\t\tif (ofs.is_open()) break;\n\t\t\tcout << \"Invalid path.\" << endl;\n\t\t}\n\t\tvector<string>* input = new vector<string>;\n\t\twhile (1) {\n\t\t\tstring line;\n\t\t\tif (ifs.eof()) break;\n\t\t\tgetline(ifs, line);\n\t\t\tif (line.empty()) continue;\n\t\t\tinput->push_back(line);\n\t\t}\n\t\tparser->parse(input, toConsole ? cout : ofs);\n\t}\n\treturn 1;\n}";
	cout << "Source code generated successfully." << endl;
}