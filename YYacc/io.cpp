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
				Production* first = new Production("S'");
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
			Production* p = new Production(current);
			delete p->right;
			p->right = split(line.substr(2), " \t");
			grammar->push_back(p);
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
	vector<unordered_map<string, int>*>* pt = analyzer->toParsingTable(grammar, tokenlist);
	Parser* parser = new Parser(grammar, pt);
	while (1) {
		while (1) {
			cout << "Path of tokens file (or -q to quit):" << endl;
			getline(cin, fname);
			if (fname.compare("-q") == 0) return;
			ifs.open(fname);
			if (ifs.is_open()) break;
			cout << "Invalid path." << endl;
		}
		bool toConsole = false;
		while (1) {
			cout << "Path of file to save sequence of reductions (or -c to show in console):" << endl;
			getline(cin, fname);
			if (fname.compare("-c") == 0) {
				toConsole = true;
				break;
			}
			ofs.open(fname, ofstream::out | ofstream::trunc);
			if (ofs.is_open()) break;
			cout << "Invalid path." << endl;
		}
		vector<string>* input = new vector<string>;
		while (1) {
			string line;
			if (ifs.eof()) break;
			getline(ifs, line);
			if (line.empty()) continue;
			input->push_back(line);
		}
		parser->parse(input, toConsole ? cout : ofs);
	}
}