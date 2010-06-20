set terminal png small
set size 1,0.75
set output "/var/www/statip.png"
set xdata time
set timefmt "%b %d %Y"
set xrange ["Mar 6 2006":"Dec 31 2006"]
set xtics "Mar 6 2006",2419200,"Dec 31 2006"
set format x "%m/%d"
set timestamp
set key top left Left reverse
set grid
plot "/home/bbs/ftphome/root/boards/bbslists/statip.txt" u 11:1 title "09:30 edu" with line 1,\
     "/home/bbs/ftphome/root/boards/bbslists/statip.txt" u 11:2 title "09:30 total" with line 2,\
     "/home/bbs/ftphome/root/boards/bbslists/statip.txt" u 11:3 title "13:30 edu" with line 3,\
     "/home/bbs/ftphome/root/boards/bbslists/statip.txt" u 11:4 title "13:30 total" with line 4,\
     "/home/bbs/ftphome/root/boards/bbslists/statip.txt" u 11:5 title "18:30 edu" with line 5,\
     "/home/bbs/ftphome/root/boards/bbslists/statip.txt" u 11:6 title "18:30 total" with line 6,\
     "/home/bbs/ftphome/root/boards/bbslists/statip.txt" u 11:7 title "22:30 edu" with line 7,\
     "/home/bbs/ftphome/root/boards/bbslists/statip.txt" u 11:8 title "22:30 total" with line 8,\
     "/home/bbs/ftphome/root/boards/bbslists/statip.txt" u 11:9 title "day edu" with line 9,\
     "/home/bbs/ftphome/root/boards/bbslists/statip.txt" u 11:10 title "day total" with line 10

