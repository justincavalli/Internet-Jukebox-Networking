package edu.sandiego.comp375.jukebox;

public class JBHeader {

    private int ID;
    private int file_size;
    private String command; 

    public JBHeader(int ID, String command) {
        this.ID = ID;
        this.file_size = 0;
        this.command = command;
    }

    public int getFileSize() {
        return file_size;
    }
    public int getID() {
        return ID;
    }
    public String getCommand() {
        return command;
    }
}
