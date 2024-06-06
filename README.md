# 🎵 MP3 Defragmenter using Threads 🧩

Welcome to the MP3 Defragmenter project! This project is an exciting exploration of file I/O, multithreading, and synchronization techniques in C. The goal is to create a program that traverses a directory tree, finds fragmented pieces of an MP3 file, and concatenates them in the correct order to form a complete MP3 file.

![Threads](https://i.imgur.com/jLFgG8A.gif)

## 🎓 Learning Outcomes
By completing this project, you will gain hands-on experience with:
- Writing a Makefile to compile a C program
- Using file stream functions for binary file I/O
- Utilizing the thread library to create and join threads
- Implementing a mutex to share data structures between threads
- Applying memory allocator functions effectively

## 🗂️ Repository Structure
- `defrag.c`: The main source code file containing the implementation of the MP3 defragmenter
- `Makefile`: The Makefile used to compile the program
- `dirs/`: A sample directory tree containing fragmented MP3 pieces for testing

## 🚀 Getting Started
To get started with the MP3 Defragmenter project, follow these steps:
1. Clone the repository:
        
        ```
        git clone https://github.com/danb127/mp3-defragmenter.git
        ```
2. Navigate to the project directory:
        ```
        cd mp3-defragmenter
        ```
3. Compile the program using the provided Makefile:
        ```
        make
        ```
4. Run the program with the desired directory tree and output file:
        ```
        ./defrag dirs music.mp3
        ```
## 🛠️ Implementation Details
The MP3 Defragmenter program adheres to the following specifications:
- Traverses the given directory tree, searching for MP3 fragments with filenames in the format `<number>.bin`
- Starts a thread for each top-level directory to enable asynchronous traversal
- Uses a mutex to synchronize access to shared data structures when appending file contents
- Concatenates the MP3 fragments in the correct order based on their numbered filenames
- Outputs the defragmented MP3 file with the specified name

## 📝 Note
- The program must be compiled using the flags `std=gnu11`, `-Werror`, and `-Wall`
- The critical section protected by the mutex should be as small as possible to maximize concurrency
- Symbolic links in the directory tree should be ignored

## 🤝 Contributing
Contributions to the MP3 Defragmenter project are welcome! If you have any ideas, suggestions, or bug fixes, please open an issue or submit a pull request.

## 📧 Contact
For any questions or inquiries, please contact danielbakerr127@gmail.com.

Happy defragmenting and enjoy your reconstructed MP3 files! 🎧
