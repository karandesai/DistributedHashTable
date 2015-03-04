g++ client.cpp -o client
g++ mult_serv.cpp -o mult_serv
g++ sis_serv.cpp -o sis_serv
echo "usage commands:"
echo "to run primary server : ./mult_serv portno 		Eg:- ./mult_serv 50000"
echo "to run client :         ./client portno 			Eg:- ./client 50000"
echo "to run sister server :  ./sis_serv portno 		Eg:- ./sis_serv 50000"
