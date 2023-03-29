# CS214-Project2
Ashley Kurnit: ank103
Zachary Goldring: ztg8

Our mysh implemented the directory wildcards and the home directory extensions from section 3 of the requirements. For our testing strategy, we created the data.txt and batch.txt files, along with the testcommands file. We then implemented the commands in batch.txt and testcommands file in the terminal to create the newdata.txt and sort.txt files.

We considered it important to check that we could list the files and directories using the ls command and then check that our pipe method worked correctly by piping newdata.txt to sort.txt and using the head command in sort.txt. Along with this, on our local machines, we tested that our home directory extension works when cd is called with no arguments by creating a subdirectory with the txt files in it, and calling cd to change the working directory to the home directory.
