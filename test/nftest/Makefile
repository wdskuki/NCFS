all:
	g++ -o nftest nftest.c -lnetfilter_queue
	g++ -o nftest_packet_loss nftest_packet_loss.c -lnetfilter_queue

clean:
	@rm -f nftest
	@rm -f nftest_packet_loss
