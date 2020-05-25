import socketserver, re, json
from time import time, asctime, sleep
from threading import Thread
from sys import argv

z_dic = {}
X_dic = {}
VK_dic = {}
NONCE_dic = {}
global PARTICIPANTS
PARTICIPANTS = 0

global COMMANDS
COMMANDS = [b"setz:", b"setX:", b"xrddy", b"zrddy", b"plrdy", b"rcrdy", b"rcval", b"srec:", b"gzvl:", b"gxvl:",
            b"allid", b"numpa", b"myidn", b"snmsg", b"pubvk", b"gtvk:", b"gtnc:", b"pubnc"]


def get_command(r):
    for c in COMMANDS:
        if r.startswith(c):
            return c


class ClientHandler(socketserver.BaseRequestHandler):
    global z_dic, X_dic, PARTICIPANTS, VK_dic, NONCE_dic

    def __init__(self, request, client_address, server):
        self.buffer = []
        self.reading = False
        super().__init__(request, client_address, server)

    def setup(self):
        super().setup()
        self.server.add_client(self)
    
    def handle(self):

        while True:
            print("Expected participants: {}. Current participants: {}. Number of Z-vals: {}. Number of X-vals: {}".format(PARTICIPANTS, len(self.server.clients), len(z_dic), len(X_dic)))

            r = self.request.recv(5)
            print(r)
            cmd = get_command(r)

            if cmd is None: break

            print("Received command: {}".format(cmd))
            id_num = self.client_address[1]

            if cmd == b'myidn':
                id_num = self.client_address[1]
                self.send_to(self.client_address, "{}\x00\n".format(id_num).encode('utf-8'))
            elif cmd == b'snmsg':
                data = self.request.recv(1088)
                self.server.broadcast(self.request, data)

            elif cmd == b'numpa':
                self.send_to(self.client_address, "{}\x00\n".format(PARTICIPANTS).encode('utf-8'))
            elif cmd == b'allid':
                ids = []
                for c in self.server.clients:
                    ids.append(str(c.client_address[1]))
                ids = sorted(ids)
                out = ','.join(ids)
                self.send_to(self.client_address, "{}\x00\n".format(out).encode('utf-8'))

            elif cmd == b'gzvl:':
                id = self.request.recv(5).strip(b"\n")
                self.send_to(self.client_address, z_dic[int(id.decode())])

            elif cmd == b'gxvl:':
                id = self.request.recv(5).strip(b"\n")
                self.send_to(self.client_address, X_dic[int(id.decode())])

            elif cmd == b'gtvk:':
                id = self.request.recv(6).strip(b"\n")
                self.send_to(self.client_address, VK_dic[int(id.decode())])

            elif cmd == b'gtnc:':
                id = self.request.recv(6).strip(b"\n")
                self.send_to(self.client_address, NONCE_dic[int(id.decode())])

            elif cmd == b'srec:':
                rec = self.request.recv(824, 0x100)
                self.server.REC = rec
            
            elif cmd == b'rcval':
                self.send_to(self.client_address, self.server.REC)

            elif cmd == b'rcrdy':
                response = "no"
                if len(self.server.REC) > 0:
                    response = "yes"

                self.send_to(self.client_address, "{}\x00\n".format(response).encode('utf-8'))

            elif cmd == b'plrdy':
                response = "no"
                if len(self.server.clients) == PARTICIPANTS:
                    response = "yes"

                self.send_to(self.client_address, "{}\x00\n".format(response).encode('utf-8'))

            elif cmd == b'zrddy':
                response = "no"
                if len(z_dic) == PARTICIPANTS:
                    response = "yes"
                
                self.send_to(self.client_address, "{}\x00\n".format(response).encode('utf-8'))
            
            elif cmd == b'xrddy':
                response = "no"
                if len(X_dic) == PARTICIPANTS:
                    response = "yes"

                self.send_to(self.client_address, "{}\x00\n".format(response).encode('utf-8'))
            
            elif cmd == b'setz:':
                val = self.request.recv(5816, 0x100)
                z_dic[id_num] = val
            elif cmd == b'setX:':
                val = self.request.recv(5816, 0x100)
                X_dic[id_num] = val
            elif cmd == b"pubvk":
                val = self.request.recv(899, 0x100)
                VK_dic[id_num] = val
            elif cmd == b"pubnc":
                val = self.request.recv(4, 0x100)
                NONCE_dic[id_num] = val
            else:
                print("Invalid command!")
#                self.buffer.append((self.client_address,recv + b"\n"))

#           print(z_dic)
            self.empty_buffers()

    def empty_buffers(self):
        if self.reading:
            return

        self.reading = True
        for client, data in self.buffer:
            self.server.broadcast(self, data)

        self.buffer = []
        self.reading = False

    def send_to(self, recipient, payload):
        self.request.sendto(payload, recipient)


class ThreadedTCPServer(socketserver.ThreadingMixIn, socketserver.TCPServer):
    allow_reuse_address = True

    def __init__(self, server_addr, request_handler):
        super().__init__(server_addr, request_handler)
        self.clients = set()
        self.REC = b""

    def add_client(self, client):
        self.clients.add(client)

    def broadcast(self, source, data):
        for client in tuple(self.clients):
            if client is not source:
                client.send_to(client.client_address, data)                

    def remove_client(self, client):
        self.clients.remove(client)


if __name__ == '__main__':
    print(argv)
    print("Starting a server. Expecting {} participants.".format(argv[1]))
    PARTICIPANTS = int(argv[1])
    HOST = '0.0.0.0'
    PORT = int(argv[2])

    server = ThreadedTCPServer((HOST, PORT), ClientHandler)
    server.serve_forever()


