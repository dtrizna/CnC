TODO:
   - [DONE] kill agent
   - [DONE] correct exit from shell
   - [DONE] help messages of Cmd 
 
  High priority:
   - add commands:
        - getpid [DONE]
        - file operations
            - b64 functions C++ [DONE]
            - i/o operations C++ [DONE]
            - upload file [DONE] 
               (implemented as b64 version and certutil.exe)
               - make direct transfer without b64 functions as per?:
               https://stackoverflow.com/questions/25634483/send-binary-file-over-tcp-ip-connection
               https://codereview.stackexchange.com/questions/43914/client-server-implementation-in-c-sending-data-files
        - migrate > inject itself
            - sRDI + TikiTorch
               - make a DLL payload [DONE]
               - convert to SC
               - weaponize
            - PE injection (sevagas)
            - native hollowing (xpn's evade blogpost)
  
  Low priority:
   - add HTTP transport
      - may be worth to consider this HTTP client:
         https://github.com/djhohnstein/CPPWebClient
   - write .NET agent (C#, boolang)
   - implement prompt display via queue (if response queue emptry - show)
   - list commands in agent mode!
   - verification if agent is alive?
   - add more transports:
      - DNS
      - DropBox
      - Slack
   - write Python agent
