/*
# Program Operation Summary

1. **FIFO Initialization**: The program starts by creating two FIFO pipes,
which will be used for communication between two users (User1 and User2). 
The FIFO is created with the appropriate permissions to allow both users to access it.

2. **Opening FIFO**: Next, the program opens both FIFO pipes in read and write mode,
enabling two-way communication between the users.

3. **Thread Creation**: The program creates a new thread that runs concurrently
and is responsible for receiving messages from User2.
It uses the `receive_messages` function to handle this communication.

4. **Sending and Receiving Messages**: The main thread of the program runs in a loop,
where the user (User1) can input messages. These messages are sent to User2 via the FIFO.
Meanwhile, the receiving thread continuously checks,
if User2 has sent any messages and displays them on the screen.

5. **Ending Communication**: The user can end the conversation by typing "end".
The program then sends a disconnection message to User2,
exits the message-sending loop, and stops the receiving thread.

6. **Cleanup**: After the communication ends, the program closes the FIFO file descriptors
and removes the created pipes to free up system resources.

# Summary

The program implements two-way communication between two users using FIFO pipes and threads,
 allowing for parallel receiving and sending of messages.
*/


// All libraries needed for input/output, file handling, and other program functionalities
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>

// Definition of FIFO names for communication between two users
// It helps make managing constant values easier
#define FIFO_USER1_TO_USER2 "fifo_user1_to_user2" // FIFO for data from User1 to User2
#define FIFO_USER2_TO_USER1 "fifo_user2_to_user1" // FIFO for data from User2 to User1
#define BUFFER_SIZE 1024 // Buffer size for message transmission

// Function to create two separate FIFOs (pipes) if they don't already exist,
// and set read and write permissions for everyone
void setup_fifo() {
    // Create FIFO for User1 to User2
    if (mkfifo(FIFO_USER1_TO_USER2, 0666) == -1 && errno != EEXIST) {
        perror("mkfifo FIFO_USER1_TO_USER2"); // Display error if creation fails
        exit(EXIT_FAILURE); // Exit the program with an error
    }
    // Create FIFO for User2 to User1
    if (mkfifo(FIFO_USER2_TO_USER1, 0666) == -1 && errno != EEXIST) {
        perror("mkfifo FIFO_USER2_TO_USER1");
        exit(EXIT_FAILURE);
    }
}

// Function in a thread to receive messages from User2
// The * symbol means we're using pointers, which are variables that store memory addresses,
// and allow access to the values stored at those addresses
void* receive_messages(void* arg) {
    int fd_from_user2 = *(int*)arg; // File descriptor for receiving messages from FIFO
    char readbuf[BUFFER_SIZE]; // Buffer to store received messages

    // Infinite loop to receive messages
    while (1) {
        int read_bytes = read(fd_from_user2, readbuf, sizeof(readbuf) - 1); // Read data from FIFO
        if (read_bytes > 0) { // If something was received
            readbuf[read_bytes] = '\0';
			// Add a null terminator so the program knows where the string ends
            // Display the message from User2
            printf("\rUser2: %s\nUser1: ", readbuf);
			// By using '\r', we can easily remove the current line from the console,
			// allowing us to show "User1:" as the start message. If User2 sends a message before us,
			// the start message is cleared, and we can write what we want.
            fflush(stdout); // Clear the output buffer for displaying the received message
        }
    }
    return NULL; // In a void function, you don't normally return anything, but since this runs in a thread,
    // we return NULL to avoid any conflicting return types during program execution
}

int main() {
    setup_fifo(); // Call the function that sets up the FIFO (pipes)

    char writebuf[BUFFER_SIZE]; // Buffer for sending messages
    int fd_to_user2, fd_from_user2; // Declaration of file descriptors for FIFOs

	// Open the FIFO for communicating with User2 in read and write mode
	fd_to_user2 = open(FIFO_USER1_TO_USER2, O_RDWR);
    if (fd_to_user2 == -1) {
        perror("open FIFO_USER1_TO_USER2");
        exit(EXIT_FAILURE);
    }

    fd_from_user2 = open(FIFO_USER2_TO_USER1, O_RDWR);
    if (fd_from_user2 == -1) {
        perror("open FIFO_USER2_TO_USER1");
        exit(EXIT_FAILURE);
    }

    // Display welcome message to the user and inform them about the 'end' command to close the chat
    printf("Admin: Welcome User1 in Two-Way communication via FIFO-pipe. Type 'end' to close chat.\n");

    // Create a new thread that receives messages from User2
	// In short, we run the receive_messages function in a separate thread,
	// allowing us to receive messages in parallel while the main program continues
	// The function takes a few important arguments:
	// 1. &recv_thread – address of the recv_thread variable, where the ID of the new thread will be stored
	// 2. NULL – thread attributes, in this case, we don't pass any special attributes
	// 3. receive_messages – the function that will run in the new thread (our function from the top of the program)
	// 4. &fd_from_user2 – argument passed to the receive_messages function.
	// This is a pointer to the file descriptor used for receiving messages.
    pthread_t recv_thread;
    pthread_create(&recv_thread, NULL, receive_messages, &fd_from_user2);

    // Loop for sending messages
    while (1) {
        printf("User1: "); // Display the start message for User1
        fflush(stdout); // Clear the output buffer

        // Get the message from the user via the keyboard
		fgets(writebuf, sizeof(writebuf), stdin);
        // Remove the newline character at the end of the input text
        writebuf[strcspn(writebuf, "\n")] = '\0';

        // If User1 types "end", stop the communication
        if (strcmp(writebuf, "end") == 0) {
            write(fd_to_user2, "Disconnected o7", 15); // Send a disconnection message to User2
            printf("Communication ended.\n");
            break; // Exit the loop
        }

        // Send the message to User2
        write(fd_to_user2, writebuf, strlen(writebuf) + 1);
    }

    // Stop the thread that receives messages
    pthread_cancel(recv_thread);

    // Close the FIFO file descriptors
    close(fd_to_user2);
    close(fd_from_user2);

    // Remove the FIFO files after communication ends
    remove(FIFO_USER1_TO_USER2);
    remove(FIFO_USER2_TO_USER1);

    return 0;
}