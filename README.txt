CS 252: Lab 3

Authors:
1.) Harshit Varma (190100055)
2.) Devansh Jain (190100044)
3.) Krushnakant Bhattad (190100036)

File Descriptions:
1.) sender.c   : Contains the code for the sender program
2.) receiver.c : Contains the code for the receiver program
3.) run.sh : Bash script that compiles both sender and receiver files and runs the experiment
    Usage: bash run.sh <Senders Port> <Receivers Port> <Number of packets> <Timeout> <Drop probability>
    Example: bash run.sh 8080 8000 100 5 0.5
    Running this script also creates sender.txt and receiver.txt which contains the required logs.