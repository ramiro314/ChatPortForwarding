# ChatPortForwarding
Conssists of a terminal based chat server/client and a portforwarding software

# Usage
Start port forwarding software
```
./port-forwarding 4000 4001
```

Start chat server on port 21200
```
./chat 4000
```

Connect chat client to port 21201
```
./chat 4001 127.0.0.1
```
