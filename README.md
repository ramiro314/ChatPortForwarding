# ChatPortForwarding
Conssists of a terminal based chat server/client and a portforwarding software

# Usage
Start chat server on port 21200
```
./chat 21200
```
Start port forwarding software (ports 21200<->20201 hardcoded right now)
```
./port-forwarding
```
Connect chat client to port 21201
```
./chat 21201 127.0.0.1
```
