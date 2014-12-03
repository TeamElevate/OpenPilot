import socket
import mraa

def readClient(client, size, rev=False):
	bytes_recd = 0
	chunks = []
	while bytes_recd < size:
		c = client.recv(min(size-bytes_recd, 2048))
		chunks.append(c)
		bytes_recd += len(c)
	ret = ''.join(chunks)
	if rev == True:
		ret = ret[::-1]
	return ret.encode('hex')

def sendClient(client, data):
	bytes_sent = 0
	while bytes_sent < len(data):
		sent = client.send(data[bytes_sent:])
		bytes_sent += sent

def getRaw(data):
	return data.decode('hex')

def readShort(obj, read_fxn):
	d = read_fxn(obj,2,True)
	return int(d,16), getRaw(d)[::-1].encode('hex')
	#return socket.ntohs(int(d,16)), d

def readLong(obj, read_fxn):
	d = read_fxn(obj,4,True)
	return int(d,16), getRaw(d)[::-1].encode('hex')
	#d = readClient(client,4)
	#return socket.ntohl(int(d,16)), d

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

def getHead(obj, read_fxn):
	while(True):
		b = read_fxn(obj,1) 
		if  b == '3c':
			break
		else:
			print b
	raw_data = '3c'
	ts = None
	rt = read_fxn(obj,1)
	raw_data += rt
	objType = getType(rt)
	length, rt  = readShort(obj, read_fxn)
	raw_data += rt
	lengthToRead = length - 10 + 1 #1 is for checksum
	objId, rt   = readLong(obj, read_fxn)
	raw_data += rt
	instId, rt  = readShort(obj, read_fxn)
	raw_data += rt
	if objType['ts']:
		ts, rt  = readShort(obj, read_fxn)
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

def i2c_send(i2c, msg):
	while len(msg) > 0:
		to_send = min(len(msg),32)
		i2c.write(msg[:to_send])
		msg = msg[to_send:]

def i2c_rcv(i2c, size, rev = False):
	chunks = []
	while size > 0:
		c = i2c.read(size)	
		chunks.append(c)
		size -= len(c)
		if len(c) == 0:
			break
	ret = (''.join(chunks))
	if rev == True:
		ret = ret[::-1]
	return ret.encode('hex')

#def i2c_rcv_head(i2c):


def setupSocket(host):
	serversocket = socket.socket(
			socket.AF_INET, socket.SOCK_STREAM)
	#bind the socket to a public host,
	# and a well-known port
	serversocket.bind((host, 8080))
	#become a server socket
	serversocket.listen(5)
	return serversocket

def setupI2c(port, addr):
	i2c = mraa.I2c(port)
	i2c.address(addr)
	return i2c
	

def handle_client(cs, i2c):
	while 1:
		head = getHead(cs, readClient)
		print head['type'], head['objId']
		body = readClient(cs, head['len'])
		raw_packet = head['raw']+getRaw(body)
		#sendClient(cs,raw_packet)
		i2c_send(i2c, raw_packet)
		i2c_head = getHead(i2c, i2c_rcv)
		i2c_body = i2c_rcv(i2c, i2c_head['len'])
		i2c_raw  = i2c_head['raw'] + getRaw(i2c_body)
		print i2c_head
		print i2c_body
		#d = i2c_rcv(i2c, 16)
		#d = getRaw(d)
		sendClient(cs, i2c_raw)
		'''
		print len(d)
		print d
		for i, c in enumerate(d):
			if c == '<':
				print i, d[i:].encode('hex')
				cs.send(d[i:])
				break
		if head['type'] == 'OBJ_REQ':
			c_head = i2c_rcv_head(i2c)
			c_body = (i2c_rcv_body(i2c, c_head['len']
			cs.send(getRaw(c_head) + getRaw(c_body))
		'''
			

def start():
	#host = socket.gethostname()
	host = '0.0.0.0'
	serversocket = setupSocket(host)
	i2c = setupI2c(6, 0)
	print host
	while 1:
		#accept connections from outside
		(clientsocket, address) = serversocket.accept()
		handle_client(clientsocket, i2c)
		#data = clientsocket.recv(
		#now do something with the clientsocket
		#in this case, we'll pretend this is a threaded server
		#ct = client_thread(clientsocket)
		#ct.run()

