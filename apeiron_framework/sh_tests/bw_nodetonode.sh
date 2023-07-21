echo '**************************'
echo 'Bandwidth Test (DDR) from node_0:port1 to node_1:port1' 
echo '***************************'

BITSTREAM='test.xclbin'
NEVENTS=1000000
PACKET_SIZE=$2
QUIET='-q'

make bandwidth_test
if [ $1 == 0 ]
then
	./bandwidth_test -b $BITSTREAM -n $NEVENTS -l $PACKET_SIZE -x 1 -s 0 -i 1 -m $QUIET
elif [ $1 == 1 ]
then	
   ./bandwidth_test -b $BITSTREAM -n $NEVENTS -l $PACKET_SIZE -x 0 -c -i 1 -m $QUIET  
else
	echo 'Only node 0 and node 1 available!! ERROR'
fi

