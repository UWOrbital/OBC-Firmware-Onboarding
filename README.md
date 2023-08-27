# C Programming Challenge - Level 1

This mini-challenge is intended to test your knowledge of C programming. There are 13 C programming questions that can be found in the `challenge.c` file. Your solution to this challenge will be verified automatically on a standard Linux machine once you make a pull request.

To test your solutions locally, you can use the provided Dockerfile to build a Docker image that will run the tests for you.

If you do not have Docker installed on your machine, you can follow the instructions <a href="https://www.notion.so/uworbital/How-To-Docker">here</a>.
 
To build the Docker image, run:
```
docker build -t fw-onboarding-level1 .
```

You can then run the container with:
```
docker run -it --rm -v $(pwd):/app fw-onboarding-level1 /bin/bash
```

This will open a bash shell inside the container. From there, you can begin working on the challenge. You can stay in the container as long as you want, and you can exit the container by typing `exit`.
In order to test your solutions, you'll need to compile the challenge.c file. You can do this by running the following commands in the bash shell:
```
make clean
make all
./build/challenge
```
This will use the provided Makefile to compile the `challenge.c` file and run the resulting executable.

If you've answered all the questions correctly, you'll pass all the test cases. There should be zero build errors/warnings.

