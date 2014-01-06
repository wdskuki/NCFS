Network coding filesystem (NCFS)

Version: 1.02
Date: June 2011

Authors: 
Lee Pak Ching (pclee@cse.cuhk.edu.hk)
Hu YuChong (yuchonghu@gmail.com)
Yu Chiu Man (cmyu@cse.cuhk.edu.hk)
Li Yan Kit (ykli7@cse.cuhk.edu.hk)
Kong Chun Ho (chkong8@cse.cuhk.edu.hk)
Zhu YunFeng (zyfl@mail.ustc.edu.cn)
Lui Chi Shing (cslui@cse.cuhk.edu.hk)


Package installation:
> sudo apt-get install libfuse-dev
> sudo apt-get install pkg-config

Quick installation:
> make clean; make

Quick setup:
> sudo ./setup.sh 4 1024
(This would setup four nodes on local loop devices.)
OR
> sudo ./setup_aoe.sh 4 1024
(This would setup four nodes on networked devices connected by ATA over Ethernet.)

YOu can modify the "raid_setting" file for different configurations.

Quick run (with debug mode):
> sudo ./ecfs -d roordir mountdir

Quick run (without debug mode):
> sudo ./ecfs rootdir mountdir

Unmount:
> sudo fusermount -u mountdir


Disk raid type:
Number := Type
100 := jbod
0 := raid0
1 := raid1
4 := raid4
5 := raid5
6 := raid6
1000 := mbr (exact minimum bandwidth regenerating code)
3000 := reed-solomon code

Disk status:
0 := normal
1 := fail (cannot open)

Operation mode:
0 := normal
1 := degraded (read only)
2 := incapable (cannot read and write)
