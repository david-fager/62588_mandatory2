> 62588 Operativsystemer
> Mandatory 2
> 23/10/2019
> readme.txt

Get started:
# Download the .zip file, unpack it at your preferred location and open a unix terminal.
# Navigate in the unix terminal, to the location and compile the code with the following
# command: 'gcc main.c -lpthread'. Then run the program by writing command: './a.out'.
# Enjoy.

The application idea:
# This program mimics the card came 'war' otherwise known as 'krig'. The implemented rules
# are as follows: each player gets half a card deck and takes turn revealing a card from
# their deck. The one with the higher value card, where 2 is the lowest and ace is the
# highest, gets a point. Different from the real card game, when player draw cards of
# equal value, this game gives no points, when in the actual game a 'war' starts.
# The game runs and writes the result of each turn into a file called BattleLog.txt.
# This means that BattleLog.txt acts as the view for the audience.

Multithreading & mutual exclusion:
# Multithreading is the concept of allowing parts of a program, to run simultaneously,
# So that while the program has one thing computing, another part of the program can run
# independently from that computation, and do its own. This can be done with 'endless'
# amount of threads running at the same time. Threads are also much lighter on the system
# than processes for instance. Threads share global resources which comes with issues.
# These issues are for instance that one thread read and uses a resource, which another
# thread after the first one read, then changes. This can corrupt the data that the
# first thread is computing, and this is partly the reason for locking important resources
# until a thread is finished with it. This locking is called mutex (mutual exclusion)
# which denies (blocks) threads access to modify resources, when another thread has
# acquired the lock.

Program environment & background execution:
# This program is made with pthreads, which are POSIX calls. Therefore the program must
# be run within a UNIX environment to function properly, if at all. This is done by
# either running the program in a terminal on a UNIX system or in a UNIX terminal on
# another system (windows for instance).
# Creating threads in programs does not by default alert the user of it happening.
# Therefore the creation and running of threads are done in the background, without
# the users knowledge.

The need for multithreading & mutual exclusion:
# It is not strictly necessary for this program to be split into two threads, however
# it is convenient for the point of the game, which is that two players independent
# of each other reveals their card. For this game to need multithreading, it would have
# to be a client-server based game, so the server has two threads each handling a client's
# input. The implemented game ensures each player reveals a card each round, by the use
# of signals.
# With multithreading this game is in need of mutual exclusion, because player two needs
# to read what player one revealed, from the battlelog and then add variable information
# depending on what was revealed. Therefore a lock on reading and writing to the file
# is needed, and signals are required to ensure this is done in the right order.

Testing:
# The program was tested by removing (commenting out) the mutual exclusion. The program
# did once out of a total of five tests, function properly with regards to printing to
# the battlelog. However the remaining four times, the battlelog had missing/wrong
# formatting, making it difficult to judge if the end result was trustworthy.