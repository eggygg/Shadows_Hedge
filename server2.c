#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>


int user_pass (char * user_or_password, FILE * fptr)
{
    char comparison[1024];
    strcpy(comparison,user_or_password);
    char line[1024];
    
    if (fptr == NULL)
    {
        printf("Error Connecting to user data base!");
        return -1;
    }

    // fgets(line, sizeof(line), fptr);

    while (fgets(line, sizeof(line), fptr) != NULL)
    {

        
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
        for (counter2 = 0;counter2 < list_count;counter2++) {
            // printf("Word %s at count %d\n",lists[counter2],counter2);
            // printf("Searched word: %s\n",comparison);
            if (strcmp(comparison, lists[counter2]) == 0) 
            {
            printf("Found in the string\n");
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

void prompt_client_action(int socket, int result) {
    char prompt[256];

    switch (result)
    {
    case 1:
        snprintf(prompt, sizeof(prompt), "Username and password valid. What would you like to do next? (1) Upload (2) Download (3) Exit:");
        break;
    case 2: 
        snprintf(prompt, sizeof(prompt), "Invalid Password");
        break;
    case 3:
        snprintf(prompt, sizeof(prompt), "Invalid Username");
        break;   
    default:
        snprintf(prompt, sizeof(prompt), "Invalid action");
        break;
    }

    if (send(socket,prompt, strlen(prompt),0) == -1) {
        perror("Send failed");
    }

    send(socket,prompt, strlen(prompt),0);
}

void handle_client_upload(int socket) {
    char filename[256];
    FILE *file;
    ssize_t bytes_received;
    char buffer[1024];

    send(socket, "Please send the filename to upload:", 35,0);

    bytes_received = recv(socket,filename, sizeof(filename) - 1, 0);
    if (bytes_received <= 0) {
        perror("Error receiving filename");
        return;
    }
    filename[bytes_received] = '\0';

    file = fopen(filename, "wb");
    if (file == NULL) {
        perror("Error opening file for writing");
        return;
    }

    // recieve the file content in parts
    while ((bytes_received = recv(socket,buffer,sizeof(buffer),0)) > 0)
    {
        fwrite(buffer, sizeof(char), bytes_received, file);
    }

    if (bytes_received == 0) {
        printf("File uploading complete.\n");
    } else {
        perror("Error receiving file data");
    }

    fclose(file);
    
}

void handle_client_download(int socket) {
    char filename[256];
    FILE *file;
    ssize_t bytes_sent;
    char buffer[1024];

    //ask client for filename to download
    send(socket, "Please send the filename to download:", 36, 0);

    // get filename from the client
    ssize_t bytes_received = recv(socket, filename, sizeof(filename) -1,0);
    if (bytes_received <=0) {
        perror("Error receiving filename");
        return;
    }
    filename[bytes_received] = '\0';

    file = fopen(filename, "rb");
    if (file == NULL) {
        send(socket, "File not found.", 15, 0);
        perror("Error openeing file for reading");
        return;
    }

    // send file in parts
    while ((bytes_received = fread(buffer, sizeof(char), sizeof(buffer), file)) >0) {
        bytes_sent = send(socket, buffer, bytes_received, 0);
        if (bytes_sent == -1) {
            perror("Error sending file data");
            break;
        } 
    }

    printf("File download complete.\n");    
}

void handle_client_exit(int socket) {
    send(socket, "Exiting, have a good day!",18,0);
    printf("Client selected exit\n");
    close(socket); // close client connection
}

 
void handle_client_action(int socket) {
    char buffer[1024] = {0};

    ssize_t bytes_received = recv(socket,buffer, sizeof(buffer)-1,0);

    if (bytes_received == -1) {
        perror("Receive failed");
        return;
    }

    buffer[bytes_received] = '\0'; // Null terminate the string
    printf("Client chose: %s\n", buffer);

    if (strcmp(buffer, "1") == 0) {
        // handle file upload
        handle_client_upload(socket);
    //    send(socket, "You selected upload. Please send the file.",41,0);
    //    printf("Client selected upload.\n");
    } else if (strcmp(buffer, "2") == 0) {
        // handle file download
        handle_client_download(socket);
    //    send(socket, "You selected download. Please wait.",41,0);
    //    printf("Client selected download.\n");
    } else if (strcmp(buffer, "3") == 0) {
        // handle exit condition
        handle_client_exit(socket);
        // send(socket, "Exiting, Goodbye!", 18,0);
        // printf("Client selected exit\n");
    } else {
        send(socket, "Invalid choice. Please select 1, 2, or 3.", 41, 0);
        printf("Client made and invalid choice\n");
    }
}



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
                result = 1;

            }
            else 
            {

                printf("Invalid Password");
                result = 2;

            }

        }
        else
        {
            printf("Invalid Username");
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
    
    // Accept a connection
    client_sock = accept(sockfd, (struct sockaddr*)&client_addr, &client_len);
    if (client_sock < 0) {
        perror("Accept failed");
        close(sockfd);
        return 1;
    }
    
    
    // Receive data
    int result = verify(client_sock);
    switch (result)
    {
        
    // Valid
    case 1:
        //
        while (1)
        {
            prompt_client_action(client_sock, result);
            handle_client_action(client_sock);

            char buffer[1024] = {0};
            ssize_t bytes_recieved = recv(client_sock,buffer, sizeof(buffer) -1, 0);

            if (bytes_recieved == -1) {
                perror("Recieve failed");
                break;
            }

            buffer[bytes_recieved] = '\0';
            if (strcmp(buffer, "3") == 0) {
                printf("Client selected exit\n");
                break;
            }
        }
        break;
    
    // Not valid: invalid password
    case 2:
        prompt_client_action(client_sock, result);
        break;
    
    // Not valid: invalid username
    case 0:
        prompt_client_action(client_sock, result);
        break;
    
    default:
        break;
    }
     
    // Close sockets
    close(client_sock);
    close(sockfd);
    return 0;
}
