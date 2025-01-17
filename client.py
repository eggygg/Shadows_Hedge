import socket
import datetime
#logging 
def stamp(message):
    ct = datetime.datetime.now()
    # print(message)
    with open("BCCD_Course/Networking_Project/log.txt", "a") as file:
        file.write(f"{ct}: {message}\n")
#handling sent bytes
def sendmessage(client_socket, data):
    # client_socket.send(combination.encode())
    client_socket.send(data.encode())
    stamp(f"Sent to server: {data}")
# function for uploading files properly to the server
def upload(client_socket, filename):
   
    try: 
        with open(f"BCCD_Course/Networking_Project/{filename}", "r") as file:
            while True:
                data = file.read() 
                print(f"Data: {data}")
                if not data or data == "EOF": 
                    print("No more data")
                    break
                client_socket.send(str(data).encode()) 
            stamp("Upload completed successfully.")
            print("Upload completed successfully.")
        

    except IOError as e: 
        stamp(f"Error during upload: {e}")
        print(f"Error during upload: {e}")

# download files from the server
def download(client_socket, filename):
    
    try:
        with open(f"BCCD_Course/Networking_Project/{filename}", "w") as file:
            while True:
                data = client_socket.recv(1024)
                if not data:
                    print(f"Data: {data}")
                    break
                decoded_data = data.decode()
                file.write(decoded_data)
    
            stamp("Download completed successfully.")
            print("Download completed successfully.")

    except Exception as e:
        stamp(f"Error during download: {e}")
        print(f"Error during download: {e}")


#receiving messages from server.
def recvmessage(client_socket):
    data = client_socket.recv(1024)  # Buffer size is 1024 bytes
    if not data:
        stamp("No response received from the server.")
        raise ValueError("No response received from the server.")
    print(data)
    stamp(f"Received from server: {data.decode()}\n")
    
#starts the socket
def start_client():
    # Step 1: Create a socket object
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    
    # Step 2: Connect to the server (provide the host and port)
    host = '127.0.0.1'  # Server's IP address (localhost)

    port = 8080       # The same port as the server

    try:
        client_socket.connect((host, port))
        # message = f"Connected to server at {host}:{port}"
        stamp(f"Connected to server at {host}:{port}")
        
        # authenticate
        id = input("ID: ")
        password = input("Password: ")
        combination = f"{id} {password}"
        # send
        sendmessage(client_socket, combination)
        # receive
        recvmessage(client_socket)

        while True:
            choice = input("What would you like to do?")
            # print(f"choice is {choice}")
            match int(choice):
                case 1:
                    sendmessage(client_socket, choice)
                    recvmessage(client_socket)
                    filename = input()
                    print(f"filename you entered: {filename}")
                    sendmessage(client_socket, filename)
                    upload(client_socket, filename)
                    # recvmessage(client_socket)
                    print("underneath upload")
                    
                case 2:
                    sendmessage(client_socket, choice)
                    recvmessage(client_socket)
                    filename = input()
                    print(f"filename you entered: {filename}")
                    sendmessage(client_socket, filename)
                    download(client_socket, filename)
                    recvmessage(client_socket)
                    print("underneath download")
                    
                case 3:
                    stamp(f"Disconnected from server at {host}:{port}")
                    print("Disconnecting from server...")
                    break

                case 4:
                    sendmessage(client_socket, choice)
                    recvmessage(client_socket)
                    print("Directory recieved")
                case _:
                    print("Invalid choice.")

        # Step 4: Receive the server's response
        
    except Exception as e:
        stamp(f"Error: {e}")
    except KeyboardInterrupt:
        stamp("Program interrupted by user.")
    finally:
        # recvmessage(client_socket)
        stamp(f"Disconnected from server at {host}:{port}")
        print("Close out of program")
        # Step 5: Close the connection
        client_socket.close()

if __name__ == "__main__":
    start_client()
