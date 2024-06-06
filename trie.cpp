//
// Created by Dayal Kumar on 02/11/23.
//
#include "trie.h"
#include <string.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <cstddef>


CharArray::CharArray(const char* string) {
	strncpy(array, string, NAME_LENGTH);
}


TrieNode::TrieNode() {
	memset(child, -1, sizeof(child));
	word_pointer = -1;
}

Trie::Trie() {
	node_list.emplace_back();
	root = 0;
	node_list[root].word_pointer = -1;
}

Trie::Trie(const char** items, std::size_t n) {
	node_list.emplace_back();
	root = 0;
	node_list[root].word_pointer = -1;
	for(std::size_t i = 0; i < n; i++) {
		Insert(items[i]);
	}
}

void Trie::Insert(const char* query) {
	Trie::TrieInsert(0, query, 0);
}

std::size_t  Trie::GetIndex(const char* query) {
	return this->Find(0, query, 0);
}

std::size_t Trie::Find(std::size_t current, const char* query, std::size_t position) {
	if(query[position] == '\0'){
		if(strcmp(query, name_list[node_list[current].word_pointer].array) == 0) return node_list[current].word_pointer;
		else return -1;
	} else {
		if(node_list[current].child[query[position]] != -1)
			return this->Find(node_list[current].child[query[position]], query, position + 1);
		else return -1;
	}
}

std::vector<CharArray> Trie::Get(const char* query) {
	std::vector<CharArray> result;
	Trie::ListInsert(0, result, query, 0);
	return result;
}

void Trie::ListInsert(std::size_t current_node, std::vector<CharArray>& items, const char* query, std::size_t position) {
	if(node_list[current_node].word_pointer != -1) {
		items.emplace_back(name_list[node_list[current_node].word_pointer]);
	}
	if(query[position] == '\0') {
		for(int c = 0; c < 128; c++) {
			if (node_list[current_node].child[c] != -1)
				ListInsert(node_list[current_node].child[c], items, query, position);
		}
	}
	else if(node_list[current_node].child[query[position]] != -1) {
		ListInsert(node_list[current_node].child[query[position]], items, query, position + 1);
	}
	return;
}

void Trie::TrieInsert(std::size_t current_node, const char* query, std::size_t position) {
	if(query[position] == '\0') {
		if(node_list[current_node].word_pointer == -1) {
			name_list.emplace_back(query);
			node_list[current_node].word_pointer = name_list.size() - 1;
			return;
		} else return;
	}
	if(node_list[current_node].child[query[position]] == -1) {
		node_list.emplace_back();
		node_list[current_node].child[query[position]] = node_list.size() - 1;
	}
	TrieInsert(node_list[current_node].child[query[position]], query, position + 1);
	return;
}
