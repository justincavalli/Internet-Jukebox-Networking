package edu.sandiego.comp375.jukebox;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.net.Socket;
import java.util.Scanner;

/**
 * Class representing a client to our Jukebox.
 * 	Header: uint32_t ID, uint32_t file_size; int command;
 */
public class AudioClient {
	public static void main(String[] args) throws Exception {
		int BUFF = 5000;
		Scanner s = new Scanner(System.in);
		Thread player = null;

		int ID;
		int file_size;
		int command_header;

		Socket socket;

		BufferedInputStream input;
		BufferedOutputStream output;

		byte header[];
		byte test[];
		byte buff[];

		//Input parameters - must specify 127.0.0.1 6666 as params for now
		String ip_address = args[0];
		int port_number = Integer.parseInt(args[1]);
		//String directory = args[2];

		System.out.println("Client: Connecting to localhost (127.0.0.1) port 6666");

		while (true) {
			System.out.print(">> ");
			String command = s.nextLine();
			String command_parameters[] = command.split(" ");

			//try {
				socket = new Socket(ip_address, port_number);
				input = new BufferedInputStream(socket.getInputStream(), 2048);
				output = new BufferedOutputStream(socket.getOutputStream(), 2048);

			//} catch (IOException e) {
				//e.printStackTrace();
			//}
			buff = new byte[BUFF];
			header = new byte[BUFF];
			test = new byte[BUFF];
			
			//Test Command (delete after)
			if (command_parameters[0].equals("test")) {
				//for (int i = 0; i < 10; i++) {
					byte[] array = new byte[4];
					int i = 9;
					array[0] = (byte)(i >> 24);
					array[1] = (byte)(i >> 16);
					array[2] = (byte)(i >> 8);
					array[3] = (byte)i;
				//}
				System.out.print("Client: sending 10 bytes");
				//for (int i = 0; i < 10; i++) {
					//System.out.print(buff[i] + " ");
				//}
				output.write(array, 0, 4);
				output.flush();

			}
			//Command - play a specified song (1)
			else if (command_parameters[0].equals("play")) {
				if (command_parameters.length < 2) {
					System.out.println("Please specify a song to play.");
				}

				try {
					//Pack and send header
					ID = Integer.parseInt(command_parameters[1]);
					file_size = 0;
					command_header = 1; //play = 1

					byte[] array = new byte[4];
					byte[] array2 = new byte[4];
					byte[] array3 = new byte[4];
										
					array[0] = (byte)(ID >> 24);
					array[1] = (byte)(ID >> 16);
					array[2] = (byte)(ID >> 8);
					array[3] = (byte)ID;
										
					array2[0] = (byte)(file_size >> 24);
					array2[1] = (byte)(file_size >> 16);
					array2[2] = (byte)(file_size >> 8);
					array2[3] = (byte)file_size;

					
					array3[0] = (byte)(command_header >> 24);
					array3[1] = (byte)(command_header >> 16);
					array3[2] = (byte)(command_header >> 8);
					array3[3] = (byte)command_header;
					
					output.write(array[0]);
					output.write(array[1]);
					output.write(array[2]);
					output.write(array[3]);
					output.write(array2[0]);
					output.write(array2[1]);
					output.write(array2[2]);
					output.write(array2[3]);
					output.write(array3[0]);
					output.write(array3[1]);
					output.write(array3[2]);
					output.write(array3[3]);
					output.flush();

					if (socket.isConnected()) {
						//read in socket input as a byte array of 3
						//socket.close();
						//socket = new Socket(ip_address, port_number);
						input = new BufferedInputStream(socket.getInputStream(), 2048); //reset input stream
						
						/*//Read back header
						int count;
						count = input.read(header, 0, 1);
						String converted_ID = new String(header, StandardCharsets.UTF_8).split("\0")[0];;
						System.out.println(converted_ID);

						count = input.read(header, 0, 3);
						String converted_size = new String(header, StandardCharsets.UTF_8).split("\0")[0];;
						System.out.println(converted_size);

						count = input.read(test, 0, 1);
						String converted_cmd = new String(test, StandardCharsets.UTF_8).split("\0")[0];;
						System.out.println(converted_cmd);*/

						//Check for correct ACK and if the requested song is playing
						//if (converted_cmd.equals("1") && converted_ID.equals(String.valueOf(ID))) { 
						if (true) {
							if (player != null) {
								//If there is a currently playing thread, stop it and play the song with a new thread
								player.interrupt();
								player.join();
								player = new Thread(new AudioPlayerThread(input));
								player.start();
							} else {
								//If no thread present, spin up new one
								player = new Thread(new AudioPlayerThread(input));
								player.start();
							}
						} else {
							System.out.println("Not play file, error");
						}
					}
				}
				catch (Exception e) {
					System.out.println(e);
				}
			}

			//Command - list all songs (2)
			else if (command_parameters[0].equals("list")) {
				try {
					//Pack and send header
					ID = 0;
					file_size = 0;
					command_header = 2; //list = 2

					//buff[0] = (byte) ID;
					//buff[1] = (byte) file_size;
					//buff[2] = (byte) command_header;
					byte[] array = new byte[4];
					byte[] array2 = new byte[4];
					byte[] array3 = new byte[4];
										
					array[0] = (byte)(ID >> 24);
					array[1] = (byte)(ID >> 16);
					array[2] = (byte)(ID >> 8);
					array[3] = (byte)ID;
										
					array2[0] = (byte)(file_size >> 24);
					array2[1] = (byte)(file_size >> 16);
					array2[2] = (byte)(file_size >> 8);
					array2[3] = (byte)file_size;

					
					array3[0] = (byte)(command_header >> 24);
					array3[1] = (byte)(command_header >> 16);
					array3[2] = (byte)(command_header >> 8);
					array3[3] = (byte)command_header;
					
					output.write(array[0]);
					output.write(array[1]);
					output.write(array[2]);
					output.write(array[3]);
					output.write(array2[0]);
					output.write(array2[1]);
					output.write(array2[2]);
					output.write(array2[3]);
					output.write(array3[0]);
					output.write(array3[1]);
					output.write(array3[2]);
					output.write(array3[3]);
					output.flush();

					if (socket.isConnected()) {
						//read in socket input as a byte array of 3
						input = new BufferedInputStream(socket.getInputStream(), 2048); //reset input stream
						
						//Read back header
						int count;
						count = input.read(header, 0, 1);
						String converted_ID = new String(header, StandardCharsets.UTF_8).split("\0")[0];;
						System.out.println(converted_ID);

						count = input.read(header, 0, 3);
						String converted_size = new String(header, StandardCharsets.UTF_8).split("\0")[0];
						System.out.println(converted_size);

						count = input.read(test, 0, 1);
						String converted_cmd = new String(test, StandardCharsets.UTF_8).split("\0")[0];
						System.out.println(converted_cmd);

						//Assuming a string as byte array
						file_size = Integer.parseInt(converted_size);
						byte list[] = new byte[file_size];
						while (input.available() > 0) {
							count = input.read(list, 0, file_size);
						}

						if (converted_cmd.equals("2")) { //Check for list command
							String out = new String(list);
							System.out.println(out);
						}
						else {
							System.out.println("Did not receive correct ACK back, error");
						}
					}
				}

				catch (Exception e) {
					System.out.println(e);
				}
			}
			
			//Command - Song info request (3)
			else if (command_parameters[0].equals("info")) {
				if (command_parameters.length < 2) {
					System.out.println("Please specify a song to list.");
				}
				try {
					//Pack and send header
					ID = Integer.parseInt(command_parameters[1]);
					file_size = 0;
					command_header = 3; //info = 3

					byte[] array = new byte[4];
					byte[] array2 = new byte[4];
					byte[] array3 = new byte[4];
										
					array[0] = (byte)(ID >> 24);
					array[1] = (byte)(ID >> 16);
					array[2] = (byte)(ID >> 8);
					array[3] = (byte)ID;
										
					array2[0] = (byte)(file_size >> 24);
					array2[1] = (byte)(file_size >> 16);
					array2[2] = (byte)(file_size >> 8);
					array2[3] = (byte)file_size;

					
					array3[0] = (byte)(command_header >> 24);
					array3[1] = (byte)(command_header >> 16);
					array3[2] = (byte)(command_header >> 8);
					array3[3] = (byte)command_header;
					
					output.write(array[0]);
					output.write(array[1]);
					output.write(array[2]);
					output.write(array[3]);
					output.write(array2[0]);
					output.write(array2[1]);
					output.write(array2[2]);
					output.write(array2[3]);
					output.write(array3[0]);
					output.write(array3[1]);
					output.write(array3[2]);
					output.write(array3[3]);
					output.flush();

					if (socket.isConnected()) {
						//read in socket input as a byte array of 3
						input = new BufferedInputStream(socket.getInputStream(), 2048); //reset input stream
						
						//Read back header
						int count;
						count = input.read(header, 0, 1);
						String converted_ID = new String(header, StandardCharsets.UTF_8).split("\0")[0];;
						System.out.println(converted_ID);

						count = input.read(header, 0, 3);
						String converted_size = new String(header, StandardCharsets.UTF_8).split("\0")[0];;
						System.out.println(converted_size);

						count = input.read(test, 0, 1);
						String converted_cmd = new String(test, StandardCharsets.UTF_8).split("\0")[0];;
						System.out.println(converted_cmd);

						//Assuming a string as byte array
						file_size = Integer.parseInt(converted_size);
						byte list[] = new byte[file_size];
						while (input.available() > 0) {
							count = input.read(list, 0, file_size);
						}

						//Handle info command like list (not sure if can handle the same)
						if (converted_cmd.equals("3") && converted_ID.equals(String.valueOf(ID))) { //Check for info command
							String out = new String(list);
							System.out.println(out);
						}
					}
				}

				catch (Exception e) {
					System.out.println(e);
				}
			}
			
			//Command - Stop playing the song (4) 
			else if (command_parameters[0].equals("stop")) {
				try {
					//Pack and send header
					ID = 0;
					file_size = 0;
					command_header = 4; //stop = 4

					byte[] array = new byte[4];
					byte[] array2 = new byte[4];
					byte[] array3 = new byte[4];
										
					array[0] = (byte)(ID >> 24);
					array[1] = (byte)(ID >> 16);
					array[2] = (byte)(ID >> 8);
					array[3] = (byte)ID;
										
					array2[0] = (byte)(file_size >> 24);
					array2[1] = (byte)(file_size >> 16);
					array2[2] = (byte)(file_size >> 8);
					array2[3] = (byte)file_size;

					
					array3[0] = (byte)(command_header >> 24);
					array3[1] = (byte)(command_header >> 16);
					array3[2] = (byte)(command_header >> 8);
					array3[3] = (byte)command_header;
					
					output.write(array[0]);
					output.write(array[1]);
					output.write(array[2]);
					output.write(array[3]);
					output.write(array2[0]);
					output.write(array2[1]);
					output.write(array2[2]);
					output.write(array2[3]);
					output.write(array3[0]);
					output.write(array3[1]);
					output.write(array3[2]);
					output.write(array3[3]);
					output.flush();

					if (socket.isConnected()) {
						//read in socket input as a byte array of 3
						input = new BufferedInputStream(socket.getInputStream(), 2048); //reset input stream
						
						/*//Read back header
						int count;
						count = input.read(header, 0, 1);
						String converted_ID = new String(header, StandardCharsets.UTF_8).split("\0")[0];;
						System.out.println(converted_ID);

						count = input.read(header, 0, 3);
						String converted_size = new String(header, StandardCharsets.UTF_8).split("\0")[0];;
						System.out.println(converted_size);

						count = input.read(test, 0, 1);
						String converted_cmd = new String(test, StandardCharsets.UTF_8).split("\0")[0];;
						System.out.println(converted_cmd);

						if (converted_cmd.equals("0")) { //ACK is 0*/
						if (player != null) {
							player.interrupt();
							player.join();
							player = null;
							socket.close();
							input.close();
							output.close();
							s.close();
							System.out.println("Stopping player");
						} else {
							System.out.println("There is no currently playing song");
						}
					}
				}
				catch (Exception e) {
					System.out.println(e);
				}
			}

			//Command - Exit out of the program (5)
			else if (command_parameters[0].equals("exit")) {
				try {
					//Pack and send header
					ID = 0;
					file_size = 0;
					command_header = 4; //stop = 4 (send a stop header when exiting)

					byte[] array = new byte[4];
					byte[] array2 = new byte[4];
					byte[] array3 = new byte[4];
										
					array[0] = (byte)(ID >> 24);
					array[1] = (byte)(ID >> 16);
					array[2] = (byte)(ID >> 8);
					array[3] = (byte)ID;
										
					array2[0] = (byte)(file_size >> 24);
					array2[1] = (byte)(file_size >> 16);
					array2[2] = (byte)(file_size >> 8);
					array2[3] = (byte)file_size;

					
					array3[0] = (byte)(command_header >> 24);
					array3[1] = (byte)(command_header >> 16);
					array3[2] = (byte)(command_header >> 8);
					array3[3] = (byte)command_header;
					
					output.write(array[0]);
					output.write(array[1]);
					output.write(array[2]);
					output.write(array[3]);
					output.write(array2[0]);
					output.write(array2[1]);
					output.write(array2[2]);
					output.write(array2[3]);
					output.write(array3[0]);
					output.write(array3[1]);
					output.write(array3[2]);
					output.write(array3[3]);
					output.flush();
				}
				catch (Exception e) {
					System.out.println(e);
				}

				//Should we receive back an ACK when exiting?
				if (socket.isConnected()) {
					if (player != null) { //If there is a currenly playing song, stop it immediately
						player.interrupt();
						player.join();
						player = null;
					}
					socket.close();
					input.close();
					output.close();
					s.close();

					System.out.println("Goodbye!");
					System.exit(0);
					break;
				}
			}
			else {
				System.err.println("ERROR: unknown command");
			}
		}
		System.out.println("Client: Exiting");
	}
}
