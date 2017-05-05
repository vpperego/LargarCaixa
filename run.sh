make clean
pkill -9 dropboxServer 
pkill -9 dropboxClient
make all
./dropboxServer 
