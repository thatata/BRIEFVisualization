# Senior Design 2018 - Hatata & Magnan
# The George Washington University

# Import Karl API
# TODO

import socket

def Main():
    HOST = '127.0.0.1' # '' #specifies that the socket is reachable by any address the machine happens to have
    PORT = 5000

    # create and INET, ICP socket
    serversocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # Bind the socket to a public host, a well-known port
    serversocket.bind((HOST, PORT))

    # Initiate and become server socket
    serversocket.listen(2)
    print("Server Listening...")

    # Read/Write Loop
    while True:
        # accepts connections
        (connsocket, address) = serversocket.accept()
        print("Connection established with: ", address)

        # Read data
        while True:
            data = connsocket.recv(1024)
            if not data: break
            print("From connected user: ", data.decode()) # Decode from byte to string to desplay for proof of transfer

            # Do what needs to be done
            x = 1
            print("Data: ", data)
            eval(data)
            print("x= ", x)

            connsocket.send(b'Echo=>' + data) # Rencode the data into the byte stream and send back to client

        connsocket.close()

    serversocket.close()

if __name__ == '__main__':
    Main()
