# Internet Jukebox
The goal of this project is to design and implement a protocol for a networked Jukebox (somewhat similar to Spotify).The protocol is defined as follows: </br>

The protocol follows a simple echo outline, in that the client sends a request to the server which echoes the request back as confirmation. </br>
The connection between the server and client should start with both in a ready/waiting state. The client, on receiving the user’s input, should send a message to the server indicating that it would like to connect. Upon receiving the connection request from the client, the server echoes back an ACK to confirm the connection status. If the client does not receive an ACK message back after timing out, it assumes that either the message was lost or the server was down, so it keeps trying to send a connect request until an ACK is finally reciprocated.</br> 
After a connection is established, the client will send one of three commands to the server, requesting some information or enacting some action. Each command will be packed with a header containing the appropriate information. The three commands are as follows:</br>
1. Sending a request for a specific song from a client to the server, and then having the server stream that song to the client.
    * Message information: A string with the command (play, stop)
    * Header information: Song ID, file size, bits of information
    * The server will respond back to the client by sending multiple chunks of information, with each chunk corresponding to a portion of the music file. This allows the server to stream the song instead of sending the entire file at once.
        * If the server receives a stop message while sending chunks (independent of any situation), the server should stop sending data immediately and revert back to the waiting state.
        * If the server receives a play message for the same song while sending chunks of the first song, the server ignores the command and keeps sending chunks of the file.
        * If the server receives a play message for a different song while sending chunks, the server stops sending the first song’s data and immediately starts sending the second song’s data to the client.

1. Sending a list of songs from the server to a client.
    * Message information: a string with the command (list) or list of songs 
    * Header information: List size, list ID, bits of information
    * The server will respond back to the client by sending a list (text file) of song titles from all available files in the source song directory. 
        * If there are no songs in the directory, the server will send a reply back stating that there are no songs available and to try again later.
        * There is no maximum number of songs that the server can list.
        * The user should be able to display previously created lists using the IDs.

1. Sending text info about a song (e.g. title, artist, album, and/or genre).
    * Message information: A string with the command (info)
    * Header information: Song ID, file size, bits of information 
    * The server will send information about a song with all metadata attached (specifically the corresponding .mp3.info file).
        * If the client tries to request information about a non-existent song, the server will respond with a song not found error message.
</br>
If an invalid message is received, the server will respond back to the client with an error explaining why it is invalid (eg. wrong parameters or file does not exist).
</br>
When the client eventually disconnects, the server transitions back to an idle state, again waiting for the client to send a connection request. 

## NOTE: Current Errors
* The program fails to successfuly pause when given the command.
* The program cannot execute some commands consecutively without needing to run again.