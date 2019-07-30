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
            - upload/download PoC [IN PROGRESS]
               - error handling - errors kill agent when:
                  - incorrect base64 data
                  - incorrect filename
               - server logics with command:
                   upload srcfile dstfile
                 to put on agent:
                   upload dstfile srcfile_b64
        - migrate > inject itself
            - sRDI + TikiTorch
               - make a DLL payload [DONE]
               - convert to SC
               - weaponize
            - PE injection (sevagas)
            - native hollowing
  
  Low priority:
   - add HTTP transport
   - write .NET agent (C#, boolang)
   - implement prompt display via queue (if response queue emptry - show)
   - list commands in agent mode!
   - verification if agent is alive?
   - add more transports:
      - DNS
      - DropBox
      - Slack
   - write Python agent
