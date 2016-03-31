#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <atomic>
#include <exception>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include <dirent.h>

using namespace std;

atomic<bool> documents_reader;
atomic<bool> queries_executor;
int documents_counter = 1;
int queries_counter = 1;

unordered_map <string, unordered_set<int>> documents_map;
ofstream dictionary_output;
ofstream results_output;
ofstream measurements_output;

void read_documents(string documents_directory, int num_of_documents) {
	int local_counter;
	string file_name;
	ifstream streamer;
	string content;
	unordered_set <string> temp_set;

	while (documents_reader) {
		
		__transaction_atomic {
			local_counter = documents_counter;
			documents_counter = documents_counter + 1;
		}
		if (local_counter <= num_of_documents) {
			file_name = documents_directory + to_string(local_counter) + ".txt";
			streamer.open(file_name.c_str());

			while (streamer >> content) {
				temp_set.insert(content);
			}
			streamer.close();
			
			for (auto temp_iter : temp_set) {
			
				__transaction_relaxed {
					auto it = documents_map.find(temp_iter);
					if (it == documents_map.end()) {
						unordered_set<int> help_set;
						help_set.insert(local_counter);
						documents_map.insert(pair<string, unordered_set<int>>(temp_iter, help_set));
					}
					else {
						(it -> second).insert(local_counter);
					}
				}
			}
			temp_set.clear();
		}
		else {
			documents_reader = false;
		}
	}
}

void execute_queries(int thread_id, string queries_directory, int num_of_queries) {
	int local_counter;
	string file_name;
	ifstream streamer;
	string content;
	unordered_map <int, int> keys_values_map;
	multimap <int, int, greater<int>> values_keys_map;

	while (queries_executor) {
		
		__transaction_atomic {
			local_counter = queries_counter;
			queries_counter = queries_counter + 1;
		}
		if (local_counter <= num_of_queries) {
			file_name = queries_directory + to_string(local_counter) + ".txt";
			streamer.open(file_name.c_str());
			
			while (streamer >> content) {
				auto doc_iter = documents_map.find(content);

				if (doc_iter != documents_map.end()) {

					for (int value : doc_iter -> second) {
						auto kv_iter = keys_values_map.find(value);

						if (kv_iter == keys_values_map.end()) {
							keys_values_map.insert(pair<int, int>(value, 1));
						}
						else {
							int new_value = (kv_iter -> second) + 1;
							kv_iter -> second = new_value;
						}
					}
				}
			}
			streamer.close();

			for (auto kv_iter : keys_values_map) {
				values_keys_map.insert(pair<int, int>(kv_iter.second, kv_iter.first));
			}
			__transaction_relaxed {
				results_output << "Thread: " << thread_id << " - Query: " << local_counter << endl;
				int num_of_results = 0;
				for (auto vk_iter : values_keys_map) {
					if (num_of_results == 20) {
						break;
					}
					results_output << vk_iter.second << ' ' << vk_iter.first << endl;
					num_of_results++;
				}
				results_output << "---------------------" << endl;
			}
			keys_values_map.clear();
			values_keys_map.clear();
		}
		else {
			queries_executor = false;
		}
	}
}

int count_files(string directory) {
	struct dirent *dirp;
    DIR *dir = opendir(directory.c_str());
    
    if (!dir) {
        cout << "Directory: " << directory << " - opendir() failed. \n";
        exit(1);
    }

    int count = 0;
    while (dirp = readdir(dir)) {
    	string str(dirp -> d_name);
    	if (str.find(".txt") != string::npos) {
    		count++;
    	}
    }
    closedir(dir);
    return count;
}

void directions(char *message) {
  cout << "Usage: " << message << " documents_directory queries_directory output_directory num_of_readers num_of_executors" << std::endl;
  exit(1);
}

int main(int argc, char *argv[]) {
	
	if (argc != 6) {
		directions(argv[0]);
	}
	
	string documents_directory;
	string queries_directory;
	string output_directory;
	int num_of_readers;
	int num_of_executors;

	try {
		documents_directory = argv[1];
		queries_directory = argv[2];
		output_directory = argv[3];
		num_of_readers = atoi(argv[4]);
		num_of_executors = atoi(argv[5]);
	}
	catch (exception) {
		directions(argv[0]);
	}
	
	int num_of_documents = count_files(documents_directory);
	int num_of_queries = count_files(queries_directory);
	
	dictionary_output.open(output_directory + "dictionary.txt");
	documents_reader = true;
	chrono::time_point<chrono::system_clock> start = chrono::high_resolution_clock::now();
	thread *readers = new thread[num_of_readers];
	for (int i = 0; i < num_of_readers; i++) {
		readers[i] = thread(read_documents, documents_directory, num_of_documents);
	}
	for (int i = 0; i < num_of_readers; i++) {
		readers[i].join();
	}
	delete[] readers;
	chrono::time_point<chrono::system_clock> end = chrono::high_resolution_clock::now();
	auto elapsed_time_readers = chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

	for (auto doc_iter : documents_map) {
		dictionary_output << doc_iter.first << ' ';

		for (auto val_iter : doc_iter.second) {
			dictionary_output << val_iter << ' ';
		}
		dictionary_output << endl;
	}
	dictionary_output.close();
	
	results_output.open(output_directory + "results.txt");
	queries_executor = true;
	start = chrono::high_resolution_clock::now();
	thread *executors = new thread[num_of_executors];
	for (int i = 0; i < num_of_executors; i++) {
		executors[i] = thread(execute_queries, i, queries_directory, num_of_queries);
	}
	for (int i = 0; i < num_of_executors; i++) {
		executors[i].join();
	}
	delete[] executors;
	results_output.close();
	end = chrono::high_resolution_clock::now();
	auto elapsed_time_executors = chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	
	measurements_output.open(output_directory + "measurements.txt");
	measurements_output << "Readers elapsed time (ms): " << elapsed_time_readers << endl << "Executors elapsed time (ms): " 
	<< elapsed_time_executors << endl;
	measurements_output.close();
	
	return 0;
}
