# Book: Unix Network Programming

# Author: W.R. Stevens

## Description

The repo contains implementation of various code fragments provided in the text. Each Program can be run individually by navigating to the directory and preparing the executable.

## Understanding the Code 

The code contains the comments from the text as well as my intrepretation of the code and its behavior. Most sub-directories has a `usage.txt` file which shows how to run the program. Those which lack this file has the usage described in one of the source file. Along with the usage, the code is written to be as self-explainable as possible. 

## Some Changes

Some changes made by me to the code fragment from the text includes:

1. In [multiple buffer implementation](./Chapter%203%20-%20Interprocess%20Communication/11_multi_buffer/), the client program faced issued when the server finished writing the content of the file in the buffer and exits shortly. This created an issue as the semaphores used to keep track of the resources contained the `SEM_UNDO` flag which made the kernel adjust the semaphore values although the client process would still be using it. To fix this issue, some modification were made, as described in the [client.c file](./Chapter%203%20-%20Interprocess%20Communication/11_multi_buffer/client.c) where the second one of the listed approach was used.

## About the Book

The book `Unix Network Programming` provides various case studies that showcase ~15,000 source code written in C (along with comments). The book is well crafted and describes various functionality provided by System V as well as 4.3BSD for network programming.