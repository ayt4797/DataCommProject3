# #!/usr/bin/bash
n=1
last=100
while [ $n -le 100 ]
do
	echo "tst"
	./TigerC <<< "
	tconnect 127.0.0.1 user pass
	tget tst$n.txt
	tput tst$n.txt
	tlist
	tcwd ../
	tlist 
	tlist
	tcwd ../
	tget tst$n.txt
	tput tst$n.txt
	exit"
	n=$(($n+1))
done
