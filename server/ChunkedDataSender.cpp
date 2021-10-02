#include <algorithm>

#include <cstring>
#include <cerrno>
#include <cstdio>

#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include <filesystem>
#include <fstream>	// std::ifstream

#include "ChunkedDataSender.h"

namespace fs = std::filesystem;

ArraySender::ArraySender(const char *array_to_send, size_t length) {
	this->array = new char[length];
	std::copy(array_to_send, array_to_send+length, this->array);
	this->array_length = length;
	this->curr_loc = 0;
}

ssize_t ArraySender::send_next_chunk(int sock_fd) {
	// Determine how many bytes we need to put in the next chunk.
	// This will be either the CHUNK_SIZE constant or the number of bytes left
	// to send in the array, whichever is smaller.
	size_t num_bytes_remaining = array_length - curr_loc;
	size_t bytes_in_chunk = std::min(num_bytes_remaining, CHUNK_SIZE);

	if (bytes_in_chunk > 0) {
		// Create the chunk and copy the data over from the appropriate
		// location in the array
		char chunk[CHUNK_SIZE];
		memcpy(chunk, array+curr_loc, bytes_in_chunk);

		ssize_t num_bytes_sent = send(sock_fd, chunk, bytes_in_chunk, 0);

		if (num_bytes_sent > 0) {
			// We successfully send some of the data so update our location in
			// the array so we know where to start sending the next time we
			// call this function.
			curr_loc += num_bytes_sent;
			return num_bytes_sent;
		}
		else if (num_bytes_sent < 0 && errno == EAGAIN) {
			// We couldn't send anything because the buffer was full
			return -1;
		}
		else {
			// Send had an error which we didn't expect, so exit the program.
			perror("send_next_chunk send");
			exit(EXIT_FAILURE);
		}
	}
	else {
		return 0;
	}
}

FileSender::FileSender(std::string file_name) : file(file_name, std::ios::binary){
	//FileSender::(file_name.c_str(), std::ios::binary);
	this->curr_loc = 0;
	this->file_length = fs::file_size(file_name);
}

ssize_t FileSender::send_next_chunk(int sock_fd) {

	// Determine how many bytes we need to put in the next chunk.
	// This will be either the CHUNK_SIZE constant or the number of bytes left
	// to send in the array, whichever is smaller.
	size_t num_bytes_remaining = this->file_length - this->curr_loc;
	size_t bytes_in_chunk = std::min(num_bytes_remaining, CHUNK_SIZE);
	char *chunk = NULL;

	if (!this->file.eof()) {
		// Create the chunk and copy the data over from the appropriate
		// location in the file 
		chunk = new char [CHUNK_SIZE];
		//this->file.seekg(0, file.cur);
		this->file.seekg(this->curr_loc);
		this->file.read(chunk, CHUNK_SIZE);

		ssize_t num_bytes_sent = send(sock_fd, chunk, bytes_in_chunk, 0);
		//std::cout << "num bytes: " << num_bytes_sent << "\n";

		if (num_bytes_sent < 0 && errno != EAGAIN) {
			// We couldn't send anything because the buffer was full
			//std::cout << "eagain error";
			return -1;
		}
		
		else if (num_bytes_sent > 0) {
			// We successfully send some of the data so update our location in
			// the file so we know where to start sending the next time we
			// call this function.
			delete[] chunk;
			this->curr_loc += num_bytes_sent;
			return num_bytes_sent;
		}
		/*else {
			// Send had an error which we didn't expect, so exit the program.
			this->file.close();
			perror("send_next_chunk send, error");
			exit(EXIT_FAILURE);
		}*/
	}
	else {
		this->file.close();
		return 0;
	}
}

ssize_t FileSender::send_first_chunk(int sock_fd, int ID) {

	// Determine how many bytes we need to put in the next chunk.
	// This will be either the CHUNK_SIZE constant or the number of bytes left
	// to send in the array, whichever is smaller.
	std::string msg = std::to_string(ID) + std::to_string(CHUNK_SIZE - 5) + std::to_string(1);
	
	size_t num_bytes_remaining = this->file_length - this->curr_loc;
	size_t bytes_in_chunk = std::min(num_bytes_remaining, CHUNK_SIZE - 5);

	if (bytes_in_chunk > 0) {
		// Create the chunk and copy the data over from the appropriate
		// location in the file 
		char chunk[CHUNK_SIZE - 5];
		this->file.seekg(0, file.cur);
		this->file.read(chunk, bytes_in_chunk);

		msg = msg + chunk;
		ssize_t num_bytes_sent = send(sock_fd, msg.c_str(), msg.length(), 0);

		if (num_bytes_sent > 0) {
			// We successfully send some of the data so update our location in
			// the file so we know where to start sending the next time we
			// call this function.
			this->curr_loc += num_bytes_sent - 5;
			return num_bytes_sent;
		}
		else if (num_bytes_sent < 0 && errno == EAGAIN) {
			// We couldn't send anything because the buffer was full
			return -1;
		}
		else {
			// Send had an error which we didn't expect, so exit the program.
			this->file.close();
			perror("send_next_chunk send");
			exit(EXIT_FAILURE);
		}
	}
	else {
		this->file.close();
		return 0;
	}
}
