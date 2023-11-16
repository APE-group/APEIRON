
BITSTREAM='test.xclbin'
NEVENTS=1000000
QUIET='-q'
BRAM='-m'

make bandwidth_test
for ((s=0; s<=3; s=s+1))
do
	START_PORT=$s
	for ((e=0; e<=3; e=e+1))
	do
		END_PORT=$e
		if [ $((START_PORT - END_PORT)) != 0 ] 
		then
		echo '**************************'
		echo 'Bandwidth Test (BRAM)' $START_PORT 'to' $END_PORT
		echo '***************************'

		for (( i=16; i <= 4096; i=i*2 ))
		do
			PACKET_SIZE=$i
    	./bandwidth_test -b test.xclbin -n $NEVENTS -l $PACKET_SIZE -i $END_PORT -s $START_PORT $BRAM $QUIET
		done
		fi

	done
done
