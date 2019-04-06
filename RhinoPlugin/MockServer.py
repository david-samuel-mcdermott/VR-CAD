import socket

HOST, PORT = '127.0.0.1', 25565
listen_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
listen_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
listen_socket.bind((HOST, PORT))
listen_socket.listen(1)
print 'Serving HTTP on port %s ...' % PORT
while True:
    client_connection, client_address = listen_socket.accept()
    request = client_connection.recv(1024)
    print request
    if request.decode().startswith("GET /vector"):
        http_response = """\
HTTP/1.1 200 OK

"""+raw_input('Enter the response: ')
    else:
        http_response = """\
HTTP/1.1 404 NOT FOUND

Only /vector/ works presently
"""
    client_connection.sendall(http_response)
    client_connection.close()
