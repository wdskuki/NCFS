losetup -d /dev/loop0
rm -f file00.img
rm -f test_read.dat
dd if=/dev/zero of=file00.img bs=1k count=10000
losetup /dev/loop0 file00.img
