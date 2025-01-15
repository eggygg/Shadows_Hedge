import socket
import datetime

def stamp(message):
    ct = datetime.datetime.now()
    # print(message)
    with open("log.txt", "a") as file:
        file.write(f"{ct}: {message}\n")

def sendmessage(client_socket, data):
    # client_socket.send(combination.encode())
    client_socket.send(data.encode())
    stamp(f"Sent to server: {data}")

def upload(client_socket):
    try: 
        with open("upload_file.txt", "r") as file:
            while True:
                data = file.read() 
                if not data or data == "EOF": 
                    break
                client_socket.send(str(data).encode()) 
            stamp("Upload completed successfully.")
            print("Upload completed successfully.")
        

    except IOError as e: 
        stamp(f"Error during upload: {e}")
        print(f"Error during upload: {e}")


def download(client_socket):
    try:
        with open("downloaded_file.txt", "w") as file:
            while True:
                data = client_socket.recv(1024)
                if not data or data.decode() == "EOF":
                    break
                file.write(data)

            stamp("Download completed successfully.")
            print("Download completed successfully.")

    except Exception as e:
        stamp(f"Error during download: {e}")
        print(f"Error during download: {e}")


def recvmessage(client_socket):
    data = client_socket.recv(1024)  # Buffer size is 1024 bytes
    if not data:
        stamp("No response received from the server.")
        raise ValueError("No response received from the server.")
    print(data)
    stamp(f"Received from server: {data.decode()}\n")
    

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
            choice = input()
            # print(f"choice is {choice}")
            match int(choice):
                case 1:
                    sendmessage(client_socket, choice)
                    recvmessage(client_socket)
                    upload(client_socket)
                case 2:
                    sendmessage(client_socket, choice)
                    recvmessage(client_socket)
                    download(client_socket)
                case 3:
                    stamp(f"Disconnected from server at {host}:{port}")
                    break
                case _:
                    print("Invalid choice.")

        # Step 4: Receive the server's response

    except Exception as e:
        stamp(f"Error: {e}")
    except KeyboardInterrupt:
        stamp("Program interrupted by user.")
    finally:
        stamp(f"Disconnected from server at {host}:{port}")
        # Step 5: Close the connection
        client_socket.close()

if __name__ == "__main__":
    start_client()
