import socket
#TODO: add size check
def readClient(client, size):
	bytes_recd = 0
	chunks = []
	while bytes_recd < size:
		c = client.recv(min(size-bytes_recd, 2048))
		chunks.append(c)
		bytes_recd += len(c)
	return (''.join(chunks)).encode('hex')

def sendClient(client, data):
	bytes_sent = 0
	while bytes_sent < len(data):
		sent = client.send(data[bytes_sent:])
		bytes_sent += sent

def getRaw(data):
	return data.decode('hex')

def readShort(client):
	d = readClient(client,2)
	return socket.ntohs(int(d,16)), d

def readLong(client):
	d = readClient(client,4)
	return socket.ntohl(int(d,16)), d

def getType(hex_str):
	t = hex_str[1]
	objType = None
	if t == '0':
		objType = 'OBJ'
	elif t == '1':
		objType = 'OBJ_REQ'
	elif t == '2':
		objType = 'OBJ_ACK'
	elif t == '3':
		objType = 'ACK'
	elif t == '4':
		objType = 'NACK'
	hb = int(hex_str[0],16)
	hasTS = None
	if hb & 8:
		hasTS = True
	return {
		'type' : objType,
		'ts'   : hasTS,
	}

def getHead(client):
	while(True):
		b = readClient(client,1) 
		if  b == '3c':
			break
		else:
			print b
	raw_data = '3c'
	ts = None
	rt = readClient(client,1)
	raw_data += rt
	objType = getType(rt)
	length, rt  = readShort(client)
	raw_data += rt
	lengthToRead = length - 10 + 1 #1 is for checksum
	objId, rt   = readLong(client)
	raw_data += rt
	instId, rt  = readShort(client)
	raw_data += rt
	if objType['ts']:
		ts, rt  = readShort(client)
		raw_data += rt
		lengthToRead -= 2
	return {
		'type' : objType['type'],
		'len'  : lengthToRead,
		'objId': hex(objId),
		'instId': hex(instId),
		'ts'    : ts,
		'raw'   : getRaw(raw_data),
	}



'''

def getPacket(client):
'''
def setupSocket(host):
	serversocket = socket.socket(
			socket.AF_INET, socket.SOCK_STREAM)
	#bind the socket to a public host,
	# and a well-known port
	serversocket.bind((host, 8080))
	#become a server socket
	serversocket.listen(5)
	return serversocket

def handle_client(cs):
	while 1:
		head = getHead(cs)
		print head['type'], head['objId']
		body = readClient(cs, head['len'])
		raw_packet = head['raw']+getRaw(body)
		sendClient(cs,raw_packet)
		'''
		i2c_send(getRaw(head)+getRaw(body))
		if head['type'] == 'OBJ_REQ':
			c_head = i2c_rcv_head()
			c_body = (i2c_rcv_body(c_head['len']
			cs.send(getRaw(c_head) + getRaw(c_body))
		'''
			

def start():
	#host = socket.gethostname()
	host = '127.0.0.1'
	serversocket = setupSocket(host)
	print host
	while 1:
		#accept connections from outside
		(clientsocket, address) = serversocket.accept()
		handle_client(clientsocket)
		#data = clientsocket.recv(
		#now do something with the clientsocket
		#in this case, we'll pretend this is a threaded server
		#ct = client_thread(clientsocket)
		#ct.run()

