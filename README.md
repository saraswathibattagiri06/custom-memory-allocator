# ⚙️ Custom Memory Allocator (C/C++)

A lightweight, low-level custom memory allocator that implements replacements for the standard `malloc()` and `free()` functions. This project manages a contiguous heap space requested from the operating system, utilizing a **Best-Fit allocation strategy** and a block header metadata system.


## 🚀 Key Technical Achievements

* **Low-Level Memory Management:** Interacts directly with the operating system via the `sbrk()` system call to request and expand contiguous heap space, reflecting deep virtual memory and OS-level comprehension.
* **Best-Fit Algorithm Implementation:** The `my_malloc()` function implements a Best-Fit search strategy (O(n) time complexity). It scans the block registry to select the smallest available free block that satisfies the requested size, drastically minimizing **external fragmentation**.
* **Implicit Free List Data Structure:** Utilizes an embedded, singly linked list architecture where each memory chunk is preceded by a `Header` struct metadata block tracking allocation states (`is_free`) and capacities.
* **Rigorous Pointer Arithmetic:** Leverages precise byte-level pointer manipulation to cleanly calculate offsets between metadata headers and the user-accessible payloads (e.g., matching alignment and evaluating boundaries via `(char*)block + HEADER_SIZE`).


## 🏗️ Architecture & Memory Layout

When a block is allocated, it is split into two distinct parts: a **Header** (metadata) and the **Payload** (usable memory returned to the user).
### The Best-Fit Strategy
1. Upon calling `my_malloc(size)`, the allocator traverses the linked list from the head.
2. It looks for all blocks where `is_free == true` and `block_size >= size`.
3. Out of all valid candidates, it selects the block that leaves the *smallest leftover fragment* of memory.
4. If a found block is significantly larger than requested, it is dynamically split into two smaller blocks to conserve space.
5. If no suitable block is found, `sbrk()` is invoked to safely grow the system heap.



## 🛠️ Technology Stack

* **Language:** C++ (utilizing foundational procedural C idioms)
* **OS Interface:** POSIX standard memory management (`sbrk()`)
* **Core Concepts:** Explicit Systems Programming, Memory Fragmentation Management, Linked Lists, Pointer Math Alignment.



## 💾 Installation & Compilation

### Prerequisites
* A Linux or macOS environment (or WSL on Windows) that supports the `sbrk()` system call.
* A GCC/G++ compiler.

### Compilation
Compile the source code using `g++`:

```bash
g++ -std=c++17 -o my_allocator my_malloc.cpp
