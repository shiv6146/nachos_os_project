# nachos_os_project
Extending NachOS by adding primitive OS features like user threads, virtual memory, running multiple processes, filesystem

# Nachos - User programs workflow

![Nachos User programs workflow](/images/userprog_workflow.png)

# Build
1. sudo bash README.Debian
2. cd code/ && make

# Running examples
1. All NachOS user programs / tests are inside the test/ dir and are executed by a simulated MIPS processor
2. cd code/build
3. ./nachos-step<step_num> -x <userprog bin>

# Step 2
This step provides features like running multiple processes, user threads and virtual memory. Any user program binary can be run by this executable.

# Step 5
This step exclusively implements a whole new user filesystem simulated by a single DISK file.
1. ./nachos-step5 -f => To format the filesystem
2. ./nachos-step5 -cp <source_file_linux> <destination_file_NachOS> => To copy file from linux space into NachOS space
3. ./nachos-step5 -mkdir <dir_name> => To create a new dir inside NachOS user filesystem
