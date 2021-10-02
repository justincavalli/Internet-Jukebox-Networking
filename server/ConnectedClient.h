#ifndef CONNECTEDCLIENT_H
#define CONNECTEDCLIENT_H

/**
 * Represents the state of a connected client.
 */
enum ClientState { RECEIVING, SENDING_SONG, SENDING_INFO, SENDING_LIST };

// Type of Commands received from the client
//enum JBCommandType: uint8_t { PLAY, STOP, INFO, LIST, ACK };
#define PLAY 1
#define STOP 4
#define INFO 3
#define LIST 2
#define ACK 0

namespace fs = std::filesystem;

// Format for the header of a segment sent by the client
struct JBHeader {
	uint32_t ID;
	uint32_t file_size;
	uint32_t command;
};

/**
 * Class that models a connected client.
 * 
 * One object of this class will be created for every client that you accept a
 * connection from.
 */
class ConnectedClient {
  public:
	// Member Variablesa (i.e. fields)
	int client_fd;
	ChunkedDataSender *sender;
	ClientState state;

	// Constructors
	/**
	 * Constructor that takes the client's socket file descriptor and the
	 * initial state of the client.
	 */
	ConnectedClient(int fd, ClientState initial_state);

	/**
	 * No argument constructor.
	 */
	ConnectedClient() : client_fd(-1), sender(NULL), state(RECEIVING) {}


	// Member Functions (i.e. Methods)
	
	/**
	 * Sends a response to the client.
	 * Note that this is just to demonstrate sending to the client: it doesn't
	 * send anything intelligent.
	 *
	 * @param epoll_fd File descriptor for epoll.
	 */
	void send_response(int epoll_fd, uint32_t ID, std::vector<fs::path> file_path);

	/**
	 * Handles new input from the client.
	 *
	 * @param epoll_fd File descriptor for epoll.
	 */
	void handle_input(int epoll_fd, std::vector<fs::path> song_path, std::vector<fs::path> info_path);

	/**
	 * Sends an ACK to the client after receiving a STOP command
	 */
	void send_ACK();

	/**
	 * Handles a close request from the client.
	 *
	 * @param epoll_fd File descriptor for epoll.
	 */
	void handle_close(int epoll_fd);

	/**
	 * Continues connection with client.
	 *
	 * @param epoll_fd File descriptor for epoll.
	 */
	void continue_response(int epoll_fd);

	/**
	 * Receives a message from the client, reading it into a JBHeader
	 * returns a pointer to the header
	 */
	JBHeader* recvHeader();

	/**
	 * Sets the timeout length of this connection
	 *
	 * @param timeout_length_ms Length of timeout period in milliseconds
	 */
	void set_timeout_length(uint32_t timeout_length_ms);

	/**
	 * creates a timeval struct that represents the given number of
	 * milliseconds.
	 *
	 * @param millis The number of milliseconds to convert.
	 * @param out_timeval Pointer to timeval that will be filled in based on
	 * millis.
	 */
	void msec_to_timeval(int millis, struct timeval *out_timeval);
};

#endif
