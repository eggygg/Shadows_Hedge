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
        for (int i = 0; i < list_count; i++) 
        {
        free(lists[i]);
        }
        free(lists);
        
    }
    fclose(fptr);
    return -1;
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
    char tokener[1024] = "gong";
    FILE * fptr = fopen("username.csv","r");
    int sockfd, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    // Create a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Socket creation failed");
        return 1;
    }
    
    //Set up server address
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
