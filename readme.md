Ethernet packet sender/receiver usage instructions:

1. Compile the source file:
gcc -o eth_comm eth_send_recv.c

2. Run Mininet and build a network:
sudo mn -c
sudo mn --topo tree,depth=3,fanout=2

3. Open up two terminals:
xterm h1 h2

4. On the host you wish to receive from, prepare the following command:
./eth_comm Recv <host interface name>
Example: ./eth_comm Recv h1-eth0

5. On the host you with to send from, prepare the following command:
./eth_comm Send <host interface name> <dest MAC> <message>
Example: ./eth_comm Send h2-eth0 5e:6a:b4:ac:9e:aa 'Hello world!'

6. Run the command on the receive side, then on the send side immediately after


Extra:
If you wish to see the actual packets being sent, use Wireshark.

To run Wireshark, on each host enter the command:
wireshark &
