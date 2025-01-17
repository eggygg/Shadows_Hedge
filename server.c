#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h> 

// function that handles opening directory and actually affects stack
int view_directory(int socket)
{
    DIR * d;
    char buffer_dir[4096];
    struct dirent *dir;
    d = opendir("/home/lrapodaca/BCCD_Course/Networking_Server/Networking_server");
    if (d) {
    send(socket,"This is the current directory:\n",31,0);
    while ((dir = readdir(d)) != NULL) 
    {
      printf("%s\n", dir->d_name);
      snprintf(buffer_dir, sizeof(buffer_dir), "%s\n", dir->d_name);
      send(socket, buffer_dir, strlen(buffer_dir), 0);
    }
    
    closedir(d);
    }
    return(0);

}


 //log function that tracks messages that are passed to it

int log_message(char * message)
{
    FILE * flogptr;
    flogptr = fopen("server_log.txt","a");
    if (flogptr == NULL)
    {

        perror("ERROR UPDATING SERVER LOG");
        return -1;

    }
    time_t now;
    struct tm * timeinfo;
    char timestamp[20];

    time(&now);
    timeinfo = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);

    fprintf(flogptr,"[%s] %s\n", timestamp, message);
    fclose(flogptr);
    return 0;
}


// function that controls the user pass and login for the server
int user_pass (char * user_or_password, FILE * fptr)
{
    char comparison[1024];
    strcpy(comparison,user_or_password);
    char line[1024];
    
    if (fptr == NULL)
    {
        printf("Error Connecting to user data base!");
        log_message("Error Connecting to user data base!");
        return -1;
    }

    // fgets(line, sizeof(line), fptr);

    while (fgets(line, sizeof(line), fptr) != NULL)
    {

        // fun way to create .split in python.
        char * word;
        char ** lists = NULL;
        int list_count = 0;
        int counter2 = 0;
        word = strtok(line, ",");
        while (word != NULL)
        {

            lists = realloc(lists, (list_count + 1) * sizeof(char *));
            lists[list_count] = strdup(word);
            list_count++;
            word = strtok(NULL, ",");

        }
        // shout out to michael for fixing this for me
        for (counter2 = 0;counter2 < list_count;counter2++) {
            // printf("Word %s at count %d\n",lists[counter2],counter2);
            // printf("Searched word: %s\n",comparison);
            if (strcmp(comparison, lists[counter2]) == 0) 
            {
            fclose(fptr);
            return 0;
            }
        }
        for (int i = 0; i < list_count; i++) {
            free(lists[i]);
        }
        free(lists);
        
    }
    fclose(fptr);
    return -1;
}
// as the title suggests client action reception and prompting the client
int prompt_client_action_valid(int socket) {

    char prompt[256];

    snprintf(prompt, sizeof(prompt), "Username and password valid. What would you like to do next? (1) Upload (2) Download (3) Exit: (4) View Directory\n");
    log_message("Logged : Client is being prompted");

    if (send(socket,prompt, strlen(prompt),0) == -1) {
        perror("Send failed");
        log_message("Send Failed");
    }

    send(socket,prompt, strlen(prompt),0);
    return 0;
}
int prompt_client_action_invalid(int socket) {
    char prompt[256];

    snprintf(prompt, sizeof(prompt), "Invalid Login!");
    log_message("Invalid Login");

    if (send(socket,prompt, strlen(prompt),0) == -1) {
        perror("Send failed");
        log_message("Send failed");
    }

    send(socket,prompt, strlen(prompt),0);
    return 0;
}

// this function handles client upload and sending information to communicate back and forth with the client
void handle_client_upload(int socket) {
    char filename[256];
    FILE *file;
    ssize_t bytes_received;
    char buffer[1024];

    send(socket, "Please send the filename to upload:\n", 35,0);

    bytes_received = recv(socket,filename, sizeof(filename) - 1, 0);
    if (bytes_received <= 0) {
        perror("Error receiving filename");
        log_message("Error receiving filename");
        return;
    }
    filename[bytes_received] = '\0';

    file = fopen(filename, "wb");
    if (file == NULL) {
        perror("Error opening file for writing");
        log_message("Error opening file for writing");
        return;
    }

    // recieve the file content in parts
    while ((bytes_received = recv(socket,buffer,sizeof(buffer),0)) > 0)
    {
        fwrite(buffer, sizeof(char), bytes_received, file);
    }

    if (bytes_received == 0) {
        printf("File uploading complete.\n");
        log_message("File uploading complete");
    } else {
        perror("Error receiving file data");
        log_message("Error receiving file data");
    }

    fclose(file);
    // finish up uploading to client
}
// handles client download and sending downloaded file to the client
int handle_client_download(int socket) {
    char filename[256];
    FILE *file;
    ssize_t bytes_sent;
    char buffer[1024];

    //ask client for filename to download
    send(socket, "Please send the filename to download:\n", 36, 0);

    // get filename from the client
    ssize_t bytes_received = recv(socket, filename, sizeof(filename) -1,0);
    if (bytes_received <=0) {
        perror("Error receiving filename");
        return -1;
    }
    filename[bytes_received] = '\0';

    file = fopen(filename, "rb");
    if (file == NULL) {
        send(socket, "File not found.", 15, 0);
        perror("Error openeing file for reading");
        return -1;
    }

    // send file in parts
    while ((bytes_received = fread(buffer, sizeof(char), sizeof(buffer), file)) > 0) {
        bytes_sent = send(socket, buffer, bytes_received, 0);
        if (bytes_sent == -1) {
            perror("Error sending file data");
            break;
        } 
    }
    fclose(file);
    printf("File download complete.\n");
    log_message("File download complete");
    return 0;
}

//handle exit conditions
void handle_client_exit(int socket) {
    send(socket, "Exiting, have a good day!\n",18,0);
    printf("Client selected exit\n");
    log_message("Client selected exit");
    close(socket); // close client connection
}



 // handling selection from choices sent to client to respond to.

int handle_client_action(int socket) {
    char buffer[1024] = {0};

    ssize_t bytes_received = recv(socket,buffer, sizeof(buffer)-1,0);

    if (bytes_received == -1) {
        perror("Receive failed");
        log_message("Receive failed");
        return -1;
    }

    buffer[bytes_received] = '\0'; // Null terminate the string
    printf("Client has chosen: %s\n", buffer);
    log_message("Client Chose Option");
    

    if (strcmp(buffer, "1") == 0) {
        // handle file upload
        printf("Hits this at upload");
        handle_client_upload(socket);

    } else if (strcmp(buffer, "2") == 0) {
        // handle file download
        handle_client_download(socket);

    } else if (strcmp(buffer, "3") == 0) {
        // handle exit condition
        handle_client_exit(socket);

    } else if (strcmp(buffer, "4") == 0) {

        view_directory(socket);
    }
    
    else {
        send(socket, "Invalid choice. Please select 1, 2, or 3 or 4.\n", 41, 0);
        printf("Client made and invalid choice\n");
        return 0;
    }
    
}


// verificiation of reception of client information for logging in and comparing against files that store user name and passwords
int verify(int socket)
{
    char buffer[1024] = {0};
    ssize_t bytes_received = recv(socket, buffer, sizeof(buffer) - 1, 0);

    if (bytes_received == -1) 
    {
        perror("Receive failed");
        return -1;
    } 
    else 
    {
        buffer[bytes_received] = '\0'; // Null-terminate the string
        // more .split() from pyhton in c shenanigans
        char * token;
        char ** tokens = NULL;
        int token_count = 0;
        token = strtok(buffer, " ");
        int result;
        while (token != NULL)
        {
            tokens = realloc(tokens, (token_count  + 1) * sizeof(char *));
            tokens[token_count] = strdup(token);
            token_count++;
            token = strtok(NULL, " "); 
        }

        FILE *fptr_user = fopen("username.csv", "r");
        FILE *fptr_pass = fopen("password.csv", "r");

        if (fptr_user == NULL || fptr_pass == NULL) {
            perror("Error opening files");
            result = -1;
        }

        if(user_pass(tokens[0],fptr_user) == 0)
        {
            if(user_pass(tokens[1],fptr_pass) == 0)
            {
                printf("Username and password valid");
                log_message("Username and password valid");
                result = 1;

            }
            else 
            {

                printf("Invalid Password");
                log_message("Invalid Password");
                result = 2;

            }

        }
        else
        {
            printf("Invalid Username");
            log_message("Invalid Password");
            result = 0; 

        }
        for (int i = 0; i < token_count; i++) 
        {
            free(tokens[i]);
        }
        free(tokens);

        return result;
    }
}



// main file that handles socket creation execution and use
int main() {
    int sockfd, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    // Create a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Socket creation failed");
        return 1;
    }
    
    // Set up server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    
    // Bind the socket
    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        return 1;
    }
    
    // Listen for connections
    listen(sockfd, 5);
    printf("Server is listening on port 8080...\n");
    log_message("Server is listening on port 8080");
    
    // Accept a connection
    client_sock = accept(sockfd, (struct sockaddr*)&client_addr, &client_len);
    if (client_sock < 0) {
        perror("Accept failed");
        close(sockfd);
        return 1;
    }
    
    
    // Receive data
    int result = verify(client_sock);
    // switch case based on client side choices
    switch (result)
    {
        
    // Valid
    case 1:
        //
        for(;;)
        {
            prompt_client_action_valid(client_sock);
            for(;;)
            {
            handle_client_action(client_sock);
            break;
            }
            break;
        }
        break;
    
    // Not valid: invalid password
    case 2:
        prompt_client_action_invalid(client_sock);
        break;
    
    // Not valid: invalid username
    case 0:
        prompt_client_action_invalid(client_sock);
        break;
    
    default:
        break;
    }
     
    // Close sockets
    close(client_sock);
    close(sockfd);
    return 0;
}
