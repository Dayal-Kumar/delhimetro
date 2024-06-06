//
// Created by Dayal Kumar on 02/11/23.
//

#ifndef DELHIMETRO_TRIE_H
#define DELHIMETRO_TRIE_H

#include <vector>
#include <unordered_map>
#include <cstddef>

#define NAME_LENGTH 64

struct CharArray {
	char array[NAME_LENGTH];
	CharArray(const char* string);
};

struct TrieNode {
	std::size_t word_pointer;
	std::size_t child[128];
	TrieNode();
};

struct Trie {
	std::size_t root;
	std::vector<TrieNode> node_list;
	std::vector<CharArray> name_list;
	Trie();
	Trie(const char** items, std::size_t n);
	void Insert(const char* query);
	std::vector<CharArray> Get(const char* query);
	std::size_t GetIndex(const char* query);
	std::size_t Find(std::size_t current, const char* query, std::size_t position);
	void ListInsert(std::size_t current_index, std::vector<CharArray>& items, const char* query, std::size_t position);
	void TrieInsert(std::size_t current_index, const char* query, std::size_t position);
};

#endif //DELHIMETRO_TRIE_H
