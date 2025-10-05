#include "HashMap.h"
#include <iostream>
#include <fstream>
#include <regex>
#include <chrono>
#include <string>
#include <algorithm>
#include <vector>
#include <unordered_map>

// helper to clean a token (keep original comments near the logic)
static std::string cleanWord(const std::string& raw) {
	// une regex qui reconnait les caractères anormaux (négation des lettres)
	static const std::regex re( R"([^a-zA-Z])");
	// élimine la ponctuation et les caractères spéciaux
	std::string w = std::regex_replace(raw, re, "");
	// passe en lowercase
	std::transform(w.begin(), w.end(), w.begin(), ::tolower);
	return w;
}

int main(int argc, char** argv) {
	using namespace std;
	using namespace std::chrono;

	// Allow filename as optional first argument, default to project-root/WarAndPeace.txt
	// Optional second argument is mode (e.g. "count" or "unique").
	string filename = "../WarAndPeace.txt";
	string mode = "freqstd";
	if (argc > 1) filename = argv[1];
	if (argc > 2) mode = argv[2];

	ifstream input(filename);
	if (!input.is_open()) {
		cerr << "Could not open '" << filename << "'. Please provide a readable text file as the first argument." << endl;
		cerr << "Usage: " << (argc>0?argv[0]:"TME2") << " [path/to/textfile]" << endl;
		return 2;
	}
	cout << "Parsing " << filename << " (mode=" << mode << ")" << endl;
	
	auto start = steady_clock::now();
	
	// prochain mot lu
	string word;

	if (mode == "count") {
		size_t nombre_lu = 0;
	
		// default counting mode: count total words
		while (input >> word) {
			// élimine la ponctuation et les caractères spéciaux
			word = cleanWord(word);

			// word est maintenant "tout propre"
			if (nombre_lu % 100 == 0)
				// on affiche un mot "propre" sur 100
				cout << nombre_lu << ": "<< word << endl;
			nombre_lu++;
		}
	input.close();
	cout << "Finished parsing." << endl;
	cout << "Found a total of " << nombre_lu << " words." << endl;

	} else if (mode == "unique") {
		// skeleton for unique mode
		// before the loop: declare a vector "seen"
		vector <string> vecteur;
		size_t i = 0;

		while (input >> word) {
			// élimine la ponctuation et les caractères spéciaux
			word = cleanWord(word);
			// add to seen if it is new
			bool present = false;
			for(i =0 ; i < vecteur.size(); i++){
				if (vecteur[i] == word){
					present = true;
					break;
				}
			}
			if (!present){
				cout << vecteur.size() << ": "<< word << endl;
				vecteur.push_back(word);
			}
			
		}
		cout << vecteur.size() << " mots differents"<< endl;
	input.close();
	}else if (mode == "freq") {
		vector <pair<string, int>> paires;
		pair<string, int> ajout;
		size_t i = 0;
		while (input >> word) {
			// élimine la ponctuation et les caractères spéciaux
			word = cleanWord(word);
			bool present = false;
			for(i =0 ; i < paires.size(); i++){
				if (paires[i].first == word){
					present = true;
					paires[i].second ++;
					break;
				}
			}
			if (!present){
				ajout.first = word;
				ajout.second = 1;
				paires.push_back(ajout);
			}
			
		}
		/*
		string cible;
		for (int j = 0 ; j < 3 ; j ++){
			if (j == 0){
				cible = "war";
			}
			else if(j == 1){
				cible = "peace";
			}else{
				cible = "toto";
			}
			auto it = find_if(paires.begin(), paires.end(), [&] (const auto& elt) {return elt.first == cible;} );
			if( it != paires.end()){
				//trouvé
				cout << cible << "trouve avec : " << paires[std::distance(paires.begin(), it)].second << " elements " << endl;
			}else{
				cout << cible << " pas trouve (frimousse triste)" << endl;
			}
		}
		*/
		std::sort(paires.begin(), paires.end(), [] (const pair<string, int> & a, const pair<string, int> & b) 
		{ return a.second > b.second ;});
		for (int i = 0 ; i < 10 ; ++i) {
			cout <<  paires[i].first << " : " << paires[i].second << endl;
		}

	} else if (mode == "freqhash") {
		size_t initial_size = 10000;//100, 1024, 10000
		HashMap<string, int> freq_map(initial_size);
		while (input >> word) {
			word = cleanWord(word);
			int* value = freq_map.get(word);
			if (value != nullptr){
				(*value)++;
			}else{
				freq_map.put(word, 1);
			}
		}
		auto entrees = freq_map.toKeyValuePairs();
		std::sort(entrees.begin(), entrees.end(), [] (const pair<string, int> & a, const pair<string, int> & b) 
		{ return a.second > b.second ;});
		for (int i = 0 ; i < 10 ; ++i) {
			cout <<  entrees[i].first << " : " << entrees[i].second << endl;
		}
}
	else if (mode == "freqstd") {
		size_t initial_size = 10000;
		unordered_map<string, int> freq_map;
		freq_map.reserve(initial_size);
		while (input >> word) {
			word = cleanWord(word);
			freq_map[word]++;
		}
		vector<pair<string, int>> entries;
		for (const auto& entry : freq_map) {
			entries.push_back(entry);
		}
		//std::sort(entries.begin(), entries.end(), [](const pair<string, int>& a, const std::pair<std::string, int>& b)
		//{ return a.second > b.second ;});
		//for (int i = 0; i < 10 ; ++i) {
		//	cout << entries[i].first << " : " << entries[i].second << endl;
		//}
	}

	else {
			// unknown mode: print usage and exit
		cerr << "Unknown mode '" << mode << "'. Supported modes: count, unique" << endl;
		input.close();
		return 1;
	}

	// print a single total runtime for successful runs
	auto end = steady_clock::now();
	cout << "Total runtime (wall clock) : " << duration_cast<milliseconds>(end - start).count() << " ms" << endl;

	return 0;
}


