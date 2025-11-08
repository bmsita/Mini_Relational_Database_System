#  Mini Relational Database System

###  Author: **Rojalin Swain**

---

##  Overview

The **Mini Relational Database System** is a simplified database engine built in **C** that demonstrates how core DBMS components work internally.  
It provides a **command-line SQL interface** to execute queries like `SELECT`, `INSERT`, `UPDATE`, and `DELETE`, while internally using **B+ Trees** for indexing, **Write-Ahead Logging (WAL)** for durability, and a **Buffer Pool** for efficient memory management.

This project was developed to understand **how a database stores, retrieves, and manages data** at a low level — beyond what traditional SQL databases hide from us.

---

##  Project Objectives

- Design and implement a **mini relational database** from scratch.  
- Understand **data indexing** using **B+ Trees**.  
- Implement **durable file storage** using **WAL (Write-Ahead Logging)**.  
- Build a **Buffer Pool** to optimize page access.  
- Create a **SQL-like query parser** for interactive command execution.  

---

##  Features Implemented

✅ Interactive SQL CLI Interface  
✅ Basic CRUD Operations (INSERT, SELECT, UPDATE, DELETE)  
✅ Conditional Filtering using `WHERE`  
✅ Persistent Storage in `.db` files  
✅ B+ Tree-based indexing for faster search  
✅ Write-Ahead Logging (WAL) for data safety  
✅ Buffer Pool for optimized in-memory management  
✅ Graceful shutdown and data saving  

---

##  Architecture Overview

The system is designed around modular components that together form the database engine:

| Component | Description |
|------------|--------------|
| **B+ Tree** | Provides indexing and balanced search for fast record lookup. |
| **Buffer Pool** | Caches frequently accessed pages to minimize disk I/O. |
| **Write-Ahead Log (WAL)** | Logs all write operations for crash recovery and durability. |
| **Storage Manager** | Handles persistent data storage using memory-mapped files. |
| **SQL Parser** | Reads and interprets SQL commands entered by the user. |

---

## 🧩 Folder Structure

Mini_Relational_Database_System/
├── src/
│ ├── sql_main.c
│ ├── sql_parser.c
│ ├── bptree.c
│ ├── buffer_pool.c
│ ├── storage.c
│ ├── record.c
│ ├── wal.c
│ └── page.c
├── header/
│ ├── bptree.h
│ ├── buffer_pool.h
│ ├── storage.h
│ ├── sql_parser.h
│ ├── wal.h
│ └── record.h
├── students.db
├── wal.log
└── README.md



## 💻 Compilation and Execution

To compile and run the project using **GCC**:


# Compile
gcc src/sql_main.c src/sql_parser.c src/bptree.c src/buffer_pool.c src/page.c src/record.c src/wal.c src/storage.c -I header -o mini_db

# Run
./mini_db

Example Session:-

$ ./mini_db
MiniDB Starting......
[BufferPool] Initialized with 3 frames.
[WAL] Initialized: wal.log
[Page] Buffer pool set successfully
[B+Tree] Initialized (order=3)
[Storage] Records loaded from students.db
MiniDB SQL Engine Ready

SQL> INSERT INTO students VALUES (1, 'Rojalin', 95);
Record inserted: ID=1, Name=Rojalin, Marks=95

SQL> INSERT INTO students VALUES (2, 'Ambika', 89);
Record inserted: ID=2, Name=Ambika, Marks=89

SQL> SELECT * FROM students;
ID | Name       | Marks
-------------------------
1 | Rojalin    | 95
2 | Ambika     | 89

SQL> UPDATE students SET marks = 90 WHERE id = 2;
Updated record ID=2 -> Name=Ambika, Marks=90

SQL> DELETE FROM students WHERE id = 1;
Deleted record(s) with ID=1

SQL> EXIT;
[Storage] Records saved to students.db
MiniDB Shutdown complete. Data saved successfully.

## Learning Outcomes 

Through this project, I learned to:
Understand DBMS internal architecture and data flow.
Implement indexing structures like B+ Trees.
Manage data durability using WAL.
Handle buffer management and caching.
Build a basic SQL parser for user input processing.
Apply file I/O, memory management, and data structure optimization.

## Future Enhancements 

To make the project more complete and advanced:
Implement ORDER BY and LIMIT in SELECT queries.
Add JOIN support between two tables.
Introduce Transaction Management (BEGIN, COMMIT, ROLLBACK).
Improve syntax validation in the SQL parser.
Add Unit Testing for reliability.
Save and load B+ Tree index to disk for persistence.
