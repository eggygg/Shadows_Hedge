#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>


int user_pass (char * user_or_password, FILE * fptr)
{

    char line[1024];
    char *split;
    char *tokens[10];
    
    if (fptr == NULL)
    {
        printf("Error Connecting to user data base!");
        return -1;
    }

    fgets(line, sizeof(line), fptr);

    while (fgets(line, sizeof(line), fptr) != NULL)
    {

        split = strtok(line, ",");

        int counter = 0;

        tokens[counter] = split;

        while (split != NULL)
        {
            if (strcmp(user_or_password, tokens[counter]) != NULL) 
            {
            printf("Found in the string.%s\n", split[counter]);
            fclose(fptr);
            return 0;
            }
            counter++;
            
        }
        fclose(fptr);
        return -1;
    }
}

void prompt_client_action(int socket) {
    char prompt[] = "Username and password valid. What would you like to do next? (1) Upload (2) Download:";
    send(socket,prompt, sizeof(prompt),0);
}

int verify(int socket)
{
    char buffer[1024] = {0};
    FILE * fptr;
    ssize_t bytes_received = recv(socket, buffer, sizeof(buffer) - 1, 0);

    if (bytes_received == -1) 
    {
        perror("Receive failed");
    } 
    else 
    {
        buffer[bytes_received] = '\0'; // Null-terminate the string
        char * token;
        char ** tokens = NULL;
        int token_count = 0;
        token = strtok(buffer, " ");
        while (token != NULL)
        {
            tokens = realloc(tokens, (token_count  + 1) * sizeof(char *));

            tokens[token_count] = strdup(token);
            token_count++;

            token = strtok(NULL, " ");

        }
        if(user_pass(tokens[0],fptr = fopen("username.csv","r")) == 0)
        {

            if(user_pass(tokens[1],fptr = fopen("password.csv","r")) == 0)
            {
                printf("Username and password valid");
                prompt_client_action(socket);

            }
            else 
            {

                printf("Invalid Password");

            }

        }
        else
        {

            printf("Invalid Username");

        }
        for (int i = 0; i < token_count; i++) 
        {
        free(tokens[i]);
        }
        free(tokens);
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
   verify(client_sock);
     
    // Close sockets
    close(client_sock);
    close(sockfd);
    return 0;
}
