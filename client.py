import socket
import datetime


def stamp(message):
    ct = datetime.datetime.now()
    print(message)
    with open("log.txt", "a") as file:
        file.write(f"{ct}: {message}\n")

def start_client():
    # Step 1: Create a socket object
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    
    # Step 2: Connect to the server (provide the host and port)
    host = '127.0.0.1'  # Server's IP address (localhost)
    port = 8080         # The same port as the server
    try:
        client_socket.connect((host, port))
        # message = f"Connected to server at {host}:{port}"
        stamp(f"Connected to server at {host}:{port}")
        
        # Step 3: Send data to the server
        id = input("ID: ")
        password = input("Password: ")
        combination = f"{id} {password}"
        client_socket.send(combination.encode())
        exit = input("Type 'shutdown' to exit: ")
        client_socket.send(exit.encode())

        

        # Step 4: Receive the server's response
        data = client_socket.recv(1024)  # Buffer size is 1024 bytes
        if not data:
            stamp("No response received from the server.")
            raise ValueError("No response received from the server.")
        stamp(f"Received from server: {data.decode()}")

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
