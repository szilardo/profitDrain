Description:
    Profit Drain is a simple tool that helps track the time spent waiting for builds to finish.

Usage:
    profitDrain -o=<timer database file> -x=<Operation To Execute>
       -x=<Operation to execute>
           "start <note>" - start timer
           "stop <exit code>" - stop timer
           stat - print build time statistics
           dump - dump raw data as text
       -o=<Timer database file name>
       -h Help

Usage examples:
    profitDrain -o=t.db -x=stat
    profitDrain -o=t.db -x=dump
    profitDrain -o=t.db -x=start
    profitDrain -o=t.db -x="start First build after integrating library xyz."
    profitDrain -o=t.db -x="stop 0"
    profitDrain -o=t.db -x="stop 32"

https://github.com/szilardo/profitDrain/blob/master/documentation/profitDrain_1.0.0.png

Motivation:
    Waiting for builds instead of actively working on solving problems is wasted time and can cause frustration,
loss of concentration, lower productivity, context switching, and many more issues. In case of a larger team,
the problems can become much worse.
    This tool should highlight the benefits of faster build times and short develop-build-test cycles, and that this is
something very much worth investing in. Fast development cycles are great for any project!
    Frustration and misery experienced with slow builds and joy felt working with well structured, fast builds has led
to the creation of this small tool. I hope someone else will find it useful.

Copyright:
    (c) 2017 Szilard Orban <devszilardo@gmail.com>. All Rights Reserved.

Support the work:
    If you feel that you are getting value from the software, please consider supporting my work by donating and
passing the word about the software to those that might find it useful.  Your help is much appreciated:
    https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=P9FRPKSN7J6WC&source=url


