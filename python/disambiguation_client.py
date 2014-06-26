import socket, struct
from disambiguation_request_pb2 import *


class DisambiguationClient:

    def __init__(self, host="localhost", port=8888):
        """
        Initialise new client using the given host and port.
        """
        self.host = host
        self.port = port
        
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.connect((host,port))

    def sendRequest(self, req):
        data = req.SerializeToString()
        size = len(data)

        print "sending request of %d bytes\n" % size

        self.socket.send(struct.pack("!L",size))
        self.socket.sendall(data)

        chunk = self.socket.recv(4)

        resp_len = struct.unpack("!L",chunk)[0]

        print "receiving a response of %d bytes\n" % resp_len


        chunks = []
        bytes_recd = 0;

        while bytes_recd < resp_len:
            chunk = self.socket.recv(min(resp_len - bytes_recd, 2048))
            if chunk == b'':
                raise RuntimeError("socket connection broken")

            chunks.append(chunk)
            bytes_recd = bytes_recd + len(chunk)

        resp_msg = b''.join(chunks)

        resp = DisambiguationRequest()
        resp.ParseFromString(resp_msg)
        return resp

    def close(self):
        self.socket.close()
