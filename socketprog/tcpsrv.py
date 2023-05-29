from socket import *

serverPort = 12000
serverSocket = socket(AF_INET,SOCK_STREAM)
serverSocket.bind(('',serverPort))

serverSocket.listen(1)
print ('The server is ready to receive')

while True:
	connectionSocket, addr = serverSocket.accept()
	print (addr)
	sentence = connectionSocket.recv(1024).decode()
	print(sentence)
	capitalizedSentence = sentence.upper()
	connectionSocket.send(capitalizedSentence.encode())
	connectionSocket.close()

