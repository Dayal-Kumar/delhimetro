//
// Created by Dayal Kumar on 02/11/23.
//
#include "delhi_metro.h"
#include "imgui/imgui.h"
#include "trie.h"
#include <iostream>
#include <string.h>
#include <cstddef>
#include <vector>
#include <queue>
#include "sha256_literal/sha256_literal.h"
#include "sha256_literal/sha256.h"

#define PASSWORD_LENGTH 128


constexpr sha256::HashType password_hash = "delhimetro4k*o8h$Ji7#&TKL$ctem"_sha256;

#define INFTY 1e9

float window_x;
float window_y;

struct MetroGraph {
	std::size_t count;
	std::vector<std::vector<int >> adj;

	MetroGraph(std::size_t count) {
		this->count = count;
		this->adj.resize(count, std::vector<int> (count, INFTY));
	}

	void AddStation() {
		for(int i = 0; i < count; i++) {
			adj[i].push_back(INFTY);
		}
		count++;
		adj.emplace_back(count, INFTY);
	}

	void AddLink(std::size_t first, std::size_t second, int distance_between) {
		this->adj[first][second] = distance_between;
		this->adj[second][first] = distance_between;
	}

	std::vector<std::pair<std::size_t, int>> GetRoute(std::size_t source, std::size_t destination) {
		std::vector<int> distance_to(count, INFTY);
		std::vector<std::size_t> predecessor(count, -1);
		std::priority_queue<std::pair<int, std::size_t>, std::vector<std::pair<int, std::size_t>>, std::greater<std::pair<int, std::size_t>>> pq;
		distance_to[source] = 0;
		pq.push({0, source});
		while(!pq.empty()) {
			const auto [current_distance, index] = pq.top();
			pq.pop();
			if(current_distance > distance_to[index]) continue;
			for(std::size_t i = 0; i < count; i++) {
				if(adj[index][i] >= INFTY) continue;
				if(current_distance + adj[index][i] < distance_to[i]) {
					predecessor[i] = index;
					distance_to[i] = current_distance + adj[index][i];
					pq.push({distance_to[i], i});
				}
			}
		}
		std::vector<std::pair<std::size_t, int>> route;
		std::size_t current = destination;
		while(predecessor[current] != -1) {
			route.emplace_back(current, distance_to[current]);
			current = predecessor[current];
		}
		std::reverse(route.begin(), route.end());
		return route;
	}
};

struct ComboState {
	std::size_t item_current_idx;
	std::vector<CharArray> items;
	bool show_listbox;
	char buffer[NAME_LENGTH];
	int turn;
	struct Trie* trie;
	ComboState(struct Trie* trie) {
		static int turn_counter = 0;
		turn = turn_counter++;
		this->trie = trie;
		this->item_current_idx = -1;
		this->buffer[0] = '\0';
		this->show_listbox = false;
	}
	static int ComboCallback(ImGuiInputTextCallbackData* data) {
		struct ComboState* state = (struct ComboState*) data->UserData;
		if(data->EventFlag == ImGuiInputTextFlags_CallbackEdit)
		{
			state->item_current_idx = -1;
			state->show_listbox = true;
			state->items = state->trie->Get(data->Buf);
		}
		if(data->EventFlag == ImGuiInputTextFlags_EscapeClearsAll) {
			state->item_current_idx = -1;
			state->show_listbox = false;
			state->buffer[0] = '\0';
			state->items = state->trie->Get("");
		}
		if(data->EventFlag == ImGuiInputTextFlags_EnterReturnsTrue) {
			state->show_listbox = false;
		}
		return 0;
	}
};

void ComboBox(const char* label, const char* hint, struct ComboState* state) {
	static int turn = 0;
	ImGui::InputTextWithHint(label, hint, state->buffer, NAME_LENGTH, ImGuiInputTextFlags_CallbackEdit | ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll, state->ComboCallback, state);
	if(ImGui::IsItemActive()) {
		if(state->buffer[0] == '\0') {
			state->items = state->trie->Get("");
			state->show_listbox = true;
		}
		turn = state->turn;
	}
	if (state->show_listbox) {
		if(turn == state->turn) {
			if (ImGui::BeginListBox(label)) {
				for (std::size_t i = 0; i < state->items.size(); i++) {
					const bool is_selected = (state->item_current_idx == i);
					if (ImGui::Selectable(state->items[i].array, is_selected)) {
						state->item_current_idx = i;
						state->show_listbox = false;
						strncpy(state->buffer, state->items[state->item_current_idx].array, NAME_LENGTH);
					}
					// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndListBox();
			}
		}
	}
}

void GetRoute(struct MetroGraph* graph, struct Trie* trie) {
	static struct ComboState source(trie), destination(trie);
	static bool invalid_stations = false;
	ComboBox("##source", "Source Station", &source);
	ComboBox("##destination", "Destination Station", &destination);
	static bool show_route = false;
	static std::vector<std::pair<std::size_t, int>> route{};
	if(ImGui::Button("Get Route")) {
		std::size_t source_index, destination_index;
		source_index = trie->GetIndex(source.buffer);
		destination_index = trie->GetIndex(destination.buffer);
		if((source_index != -1) && (destination_index != -1)) {
			invalid_stations = false;
			route = graph->GetRoute(source_index, destination_index);
			show_route = true;
		} else {
			invalid_stations = true;
		}
	}
	if(show_route){
		if(route.size() > 0) {
			for(std::size_t i = 0; i < route.size(); i++) {
				ImGui::Text("%d Minutes : %s", route[i].second, trie->name_list[route[i].first].array);
			}
		} else {
			ImGui::Text("No route found");
		}
	}
	if(invalid_stations) {
		ImGui::Text("Invalid stations");
	}
}

void AddLink(struct MetroGraph* graph, struct Trie* trie) {
	static bool show_addlink_window = false;
	static bool invalid_stations = false;
	if (ImGui::Button("Add Link")) {
		show_addlink_window = true;
	}
	if (show_addlink_window){
		auto io = ImGui::GetIO();
		ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x / 2, io.DisplaySize.y / 2));
		if (!ImGui::Begin("Add/Update Link", &show_addlink_window)) {
			ImGui::End();
			return;
		}
		static struct ComboState source(trie), destination(trie);
		ComboBox("##new_source", "Source Station", &source);
		ComboBox("##new_destination", "Destination Station", &destination);
		static char distance_between[8] = "";
		ImGui::InputTextWithHint("##distance_between", "Distance Between", distance_between, 8,
		                         ImGuiInputTextFlags_CharsDecimal);
		if (ImGui::Button("Add/Update Link")) {
			std::size_t source_index, destination_index;
			source_index = trie->GetIndex(source.buffer);
			destination_index = trie->GetIndex(destination.buffer);
			if((source_index != -1) && (destination_index != -1)) {
				graph->AddLink(source_index, destination_index, atoi(distance_between));
				show_addlink_window = false;
				invalid_stations = false;
			} else {
				invalid_stations = true;
			}
		}
		if(invalid_stations)
			ImGui::Text("Invalid Stations");
		ImGui::End();
	}
}

void AddStation(struct MetroGraph* graph, struct Trie* trie) {
	static bool show_addstation_window = false;
	if (ImGui::Button("Add Station")) {
		show_addstation_window = true;
	}
	if(show_addstation_window) {
		auto io = ImGui::GetIO();
		ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x / 2, io.DisplaySize.y / 2));
		if (!ImGui::Begin("Add Station", &show_addstation_window)) {
			ImGui::End();
			return;
		}
		static char new_station[NAME_LENGTH] = "";
		ImGui::InputTextWithHint("##new_station", "New Station Name", new_station, NAME_LENGTH);
		if (ImGui::Button("Add New Station")) {
			graph->AddStation();
			trie->Insert(new_station);
			show_addstation_window = false;
		}
		ImGui::End();
	}
}

void AdminLogin(bool* admin_logged) {
	const std::string salt = "4k*o8h$Ji7#&TKL$ctem";
	static bool show_login_window = false;
	if(*admin_logged) {
		if(ImGui::Button("Admin Logout")){
			*admin_logged = false;
		}
	} else {
		if(ImGui::Button("Admin Login")) {
			show_login_window = true;
		}
	}
	if(show_login_window) {
		auto io = ImGui::GetIO();
		ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x / 2, io.DisplaySize.y / 2));
		if(!ImGui::Begin("Admin Login", &show_login_window)){
			ImGui::End();
		}
		static char buffer[PASSWORD_LENGTH] = "";
		static bool wrong_password = false;
		ImGui::Text("Please enter your admin password");
		ImGui::InputTextWithHint("##password", "<Password>", buffer, PASSWORD_LENGTH, ImGuiInputTextFlags_Password);
		if(ImGui::Button("Login")) {
			std::string input(buffer);
			std::string input_with_salt = input + salt;
			if(password_hash == sha256::compute((const uint8_t*)input_with_salt.c_str(), input_with_salt.length())) {
				*admin_logged = true;
				show_login_window = false;
			} else {
				wrong_password = true;
			}
			buffer[0] = '\0';
		}
		if(wrong_password) {
			ImGui::Text("Wrong Password");
		}
		ImGui::End();
	}
}

void DelhiMetro::render_app(bool* p_open) {
	auto io = ImGui::GetIO();
	ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y));
	ImGui::SetNextWindowPos(ImVec2(0,0));
	ImGui::Begin("Delhi Metro", p_open);
	const char* items[] = { };

	static bool admin_logged = false;
	static struct Trie trie(items, 0);
	static struct MetroGraph graph(trie.name_list.size());

	if(admin_logged) {
		AddLink(&graph, &trie);
		ImGui::SameLine();
		AddStation(&graph, &trie);
		ImGui::SameLine();
	}
	AdminLogin(&admin_logged);
	GetRoute(&graph, &trie);
	ImGui::End();

}
