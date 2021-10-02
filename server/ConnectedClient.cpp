#include <iostream>

#include <cstring>

#include <unistd.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <vector>
#include <filesystem>

#include "ChunkedDataSender.h"
#include "ConnectedClient.h"

using std::cout;
using std::vector;
using std::cerr;

namespace fs = std::filesystem;


ConnectedClient::ConnectedClient(int fd, ClientState initial_state) :
	client_fd(fd), sender(NULL), state(initial_state) {}

void ConnectedClient::send_response(int epoll_fd, uint32_t ID, std::vector<fs::path> file_path) {
	// Create a large array, just to make sure we can send a lot of data in
	// smaller chunks.
	//char *data_to_send = new char[CHUNK_SIZE*2000];
	//memset(data_to_send, 117, CHUNK_SIZE*2000); // 117 is ascii 'u'
	
	//std::string song_dir = "music/"// name of the directory with songs
	//std::string info_dir = // name of the directory with song info

	//ArraySender *array_sender = new ArraySender(data_to_send, CHUNK_SIZE*2000);
	//delete[] data_to_send; // The ArraySender creates its own copy of the data so let's delete this copy
	cout << "in send_response. state is " << this->state << "\n";
	ssize_t num_bytes_sent = 0;
	ssize_t total_bytes_sent = 0;

	// keep sending the next chunk until it says we either didn't send
	// anything (0 return indicates nothing left to send) or until we can't
	// send anymore because of a full socket buffer (-1 return value)
	/*
	while((num_bytes_sent = array_sender->send_next_chunk(this->client_fd)) > 0) {
		total_bytes_sent += num_bytes_sent;
	}
	cout << "sent " << total_bytes_sent << " bytes to client\n";
	*/
	FileSender *file_sender;
	bool init_file_sender = false;
	if (this->state == SENDING_SONG) {

		std::string song_name = file_path[ID];
		file_sender = new FileSender(song_name);
		init_file_sender = true;

		// set timeout length for the recv while sending chunks
		//set_timeout_length(20);	
		char recv_seg[sizeof(JBHeader)];
		/*if ((num_bytes_sent = file_sender->send_first_chunk(this->client_fd, ID)) < 0) {
			perror("send_first_chunk");
		}*/

		while (total_bytes_sent < fs::file_size(song_name)) {
			while ((num_bytes_sent = file_sender->send_next_chunk(this->client_fd)) > 0) {
				total_bytes_sent += num_bytes_sent;
				if (recv(this->client_fd, recv_seg, sizeof(JBHeader), 0) < 0) {
					if (errno == EAGAIN) {
						// timed out. Didn't receive a STOP message
						continue;
					}
					else {
						// some other error than a timeout
						perror("SENDING_SONG recv");
						exit(EXIT_FAILURE);
					}
				}
				else {
					// received a message from the client
					JBHeader* hdr = (JBHeader*)recv_seg;
					if (hdr->command == STOP) {
						// client wants to stop the song currently streaming. Stop
						// sending chunks and send an ACK
						send_ACK();
						break;
					}
					else {
						// received a different command. Handle later
					}
				}
			}
		}
		// done sending. go back to idle state
		this->state = RECEIVING;
	// make timeout length indefinite again
	set_timeout_length(0);
	}
	else if (this->state == SENDING_INFO) {
		cout << "the vector is \n";
		for (int i=0;i<file_path.size();i++)
			cout << file_path.at(i) << "\n";
		std::string info_file = file_path[ID];
		int bytes_read = 0;
		unsigned int buffer_size = 4096;	// Calculate a better size?
		char file_data[buffer_size];
		if (info_file != "") {
			
			std::ifstream file (info_file.c_str(), std::ios::binary);

			while(!file.eof()) {
				file.read(file_data, buffer_size);
				bytes_read = file.gcount();
				cout << "read " << bytes_read << " bytes from file\n";
			}
			file.close();
			// make sure file_size is 3 bytes long	
			std::string file_size;
			if (bytes_read < 10) {
				// prepend a 0 to make file_size 3 byte
				file_size = std::to_string(0) + std::to_string(0) + std::to_string(bytes_read);
			}
			else if (bytes_read < 100) {
				file_size = std::to_string(0) + std::to_string(bytes_read);
			}
			else {
				file_size = std::to_string(bytes_read);
			}
			// create a header to send back
			std::string msg = std::to_string(ID) + file_size + std::to_string(3);
			msg = msg + file_data;

			if ((num_bytes_sent = send(this->client_fd, msg.c_str(), msg.length(), 0)) < 0) {
				perror("SENDING_INFO send");
			}
		}
		else {
			std::string msg = "ID does not have a .info file.\n";
			std::string head = std::to_string(ID) + std::to_string(0) + std::to_string(46) + std::to_string(3);
			head = head + msg;
			
			if ((num_bytes_sent = send(this->client_fd, head.c_str(), head.length(), 0)) < 0) {
				perror("SENDING_INFO send ID error");
			}
		}
	}
	else if(this->state == SENDING_LIST) { //List function, maybe should be its own method
		/*
		std::string song_list = song_path[1]; // populate song list
		int list_length = song_list.length();
		char send_list[sizeof(JBHeader) + list_length];
		JBHeader *hdr_list = (JBHeader*) send_list;

		hdr_list->ID = 1; //the first song
		hdr_list->file_size = list_length;
		hdr_list->command = LIST;
		cerr << "song_list is " << song_list << "\n";
		//this->send_response(epoll_fd, send_list, sizeof(send_list));
		*/
		cout << "getting ready to send list\n";
		int index = 1;
		std::stringstream song_list;	//make song_list a stringstream
		for (auto& mp3file : file_path) {
			if (!mp3file.empty()) {
				song_list << index++ << " " << mp3file << "\n";
			}
		}
		cout << "past the for loop\n";	
		
		std::string file_size;
		std::string temp = std::to_string((song_list.str()).length());
		cout << "temp is " << temp << "\n";
		if (stoi(temp) < 10) {
			cout << "got into the first if\n";
			// prepend a 0 to make file_size 3 byte
			file_size = std::to_string(0) + std::to_string(0) + temp;
		}
		else if (stoi(temp) < 100) {
			cout << "the temp length is " << int(temp.length()) << "\n";
			file_size = std::to_string(0) + temp;
		}
		else {
			file_size = temp;
		}
		cout << "the temp is now " << temp << "the file_size is " << file_size << "\n";
		std::string msg = std::to_string(0) + file_size + std::to_string(2);
		msg = msg + song_list.str();
		cout << msg << "\n" << "size is " << file_size; 
		// need a header for the message sent back?
		if ((num_bytes_sent = send(this->client_fd, msg.c_str(), msg.length(), 0)) < 0) {
			perror("SENDING_LIST send");
		}
		cout << "got past the send\n";  
	}

	/*
	 * TODO: if the last call to send_next_chunk indicated we couldn't send
	 * anything because of a full socket buffer, we should do the following:
	 *
	 * 1. update our state field to be sending
	 * 2. set our sender field to be the ArraySender object we created
	 * 3. update epoll so that it also watches for EPOLLOUT for this client
	 *    socket (use epoll_ctl with EPOLL_CTL_MOD).
	 *
	 * WARNING: These steps are to be done inside of the following if statement,
	 * not before it.
	 */
	cout << "line 0. num_bytes_sent is " << num_bytes_sent << "\n";
	if (num_bytes_sent < 0) {
		// Fill this in with the three steps listed in the comment above.
		// WARNING: Do NOT delete array_sender here (you'll need it to continue
		// sending later).
		cout << "line 1\n";
		struct epoll_event event;
		event.events = EPOLLIN | EPOLLRDHUP | EPOLLOUT;
		cout << "line 2\n";
		//this->state = SENDING;
		this->sender = file_sender;
		event.data.fd = this->client_fd;
		cout << "line 3\n";
		if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, this->client_fd, &event) < 0) {
			perror("epoll error");
		}
	}
	else {
		// Sent everything with no problem so we are done with our FileSender
		// object. Delete if one was initialized
		if (init_file_sender) {
			cout << "Shutting down\n";
			//shutdown(this->client_fd, SHUT_RDWR);
			delete file_sender;
		}
	}
}

void ConnectedClient::handle_input(int epoll_fd, std::vector<fs::path> song_path, std::vector<fs::path> info_path) {
	cout << "Ready to read from client " << this->client_fd << "\n";
	//char recv_seg[1024];

	JBHeader* hdr = recvHeader();
	//void *data = (void*)(recv_seg + sizeof(JBHeader));

	//ssize_t bytes_received = recv(this->client_fd, recv_seg, 1024, 0);
	//if (bytes_received < 0) {
		//perror("client_read recv");
		//exit(EXIT_FAILURE);
	//}

	/*
	cout << "Received data: ";
	for (int i = 0; i < bytes_received; i++)
		cout << recv_seg[i];

	cout << "\n";
	*/
	cout << "the header is: ID " << (int)hdr->ID << " size " << (int)hdr->file_size << " command " << (int)hdr->command << "\n";
	if (hdr->command == PLAY) {
		this->state = SENDING_SONG;
		this->send_response(epoll_fd, hdr->ID, song_path);
	}
	else if (hdr->command == STOP) {
		// send back an ACK
		send_ACK();
	}
	else if (hdr->command == 3) {
		this->state = SENDING_INFO;	
		this->send_response(epoll_fd, hdr->ID, info_path);

	}
	else {
		this->state = SENDING_LIST;
		this->send_response(epoll_fd, 0, song_path);
	}
	 

	// TODO: Eventually you need to actually look at the response and send a
	// response based on what you got from the client (e.g. did they ask for a
	// list of songs or for you to send them a song?)
	// For now, the following function call just demonstrates how you might
	// send data.
	//this->send_response(epoll_fd);
}


// You likely should not need to modify this function.
void ConnectedClient::handle_close(int epoll_fd) {
	cout << "Closing connection to client " << this->client_fd << "\n";

	if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, this->client_fd, NULL) == -1) {
		perror("handle_close epoll_ctl");
		exit(EXIT_FAILURE);
	}

	close(this->client_fd);
}

void ConnectedClient::send_ACK() {
	//char send_seg[sizeof(JBHeader)];
	//JBHeader* hdr = (JBHeader*)send_seg;
	//hdr->command = 4;
	std::string ACK_hdr = std::to_string(0) + std::to_string(0) + std::to_string(0) + std::to_string(0) + std::to_string(0);
	
	if (send(this->client_fd, ACK_hdr.c_str(), ACK_hdr.length(), 0) < 0) {
		perror("send_ack send");
		exit(EXIT_FAILURE);
	}		
}

void ConnectedClient::continue_response(int epoll_fd) {

	ssize_t num_bytes_sent = 0;
	ssize_t total_bytes_sent = 0;

	// keep sending the next chunk until it says we either didn't send
	// anything (0 return indicates nothing left to send) or until we can't
	// send anymore because of a full socket buffer (-1 return value)
	while((num_bytes_sent = this->sender->send_next_chunk(this->client_fd)) > 0) {
		total_bytes_sent += num_bytes_sent;
	}
	cout << "sent " << total_bytes_sent << " bytes to client\n";

	if (num_bytes_sent < 0) {
		struct epoll_event event;
		event.events = EPOLLIN | EPOLLRDHUP;
		
		this->state = RECEIVING;
		event.data.fd = this->client_fd;

		if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, this->client_fd, &event) < 0) {
			perror("epoll error");
		}
	}
}

JBHeader* ConnectedClient::recvHeader() {
	cout << "got into recvHeader\n";
	char full_buffer[sizeof(JBHeader)];
	char temp_buffer[sizeof(JBHeader)];

	int buffer_position = 0;
	int num_bytes = 0;
	int total_bytes = 0;
	JBHeader* hdr = new JBHeader;
	int a;
	for (int i = 0; i < 3; i++) {
		if((num_bytes = recv(this->client_fd, temp_buffer, sizeof(int), 0)) < 0) {
			perror("recvHeader() recv");
		}
		a = int((unsigned char)(temp_buffer[0]) << 24 |
		            (unsigned char)(temp_buffer[1]) << 16 |
					(unsigned char)(temp_buffer[2]) << 8 |
					(unsigned char)(temp_buffer[3]));
		cout << a << "\n";	
		if (i == 0)
			hdr->ID = a;
		else if (i == 1)
			hdr->file_size = a;
		else
			hdr->command = a;
		
		cout << "got past recvHeader() recv" << "\n";
		total_bytes += num_bytes;
		// store the bytes just read at the next open spot in the buffer
		for(int i = 0; i < num_bytes; i++) {
			full_buffer[buffer_position++] = temp_buffer[i];
		}
		/*
		// Stop once the entire header is received
		if (total_bytes == sizeof(JBHeader)) {
			break;
		}*/
	}
	// Make a header for the bytes read in
	//JBHeader* hdr = (JBHeader*)full_buffer;
	return hdr;	
}

void ConnectedClient::set_timeout_length(uint32_t timeout_length_ms) {
	struct timeval timeout;
	msec_to_timeval(timeout_length_ms, &timeout);

	if (setsockopt(this->client_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(struct timeval)) < 0) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
}

void ConnectedClient::msec_to_timeval(int millis, struct timeval *out_timeval) {
	out_timeval->tv_sec = millis/1000;
	out_timeval->tv_usec = (millis%1000)*1000;
}
