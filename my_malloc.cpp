#include <iostream>
#include <windows.h>// For sbrk (Linux/macOS). Use <windows.h> and VirtualAlloc on Windows.
#include <cstring>  // For memcpy, memset

// --- 1. CONFIGURATION ---
// Define a reasonable block size for initial memory allocation (e.g., 1MB)
const size_t INITIAL_HEAP_SIZE = 1024 * 1024; // 1MB

// --- 2. DATA STRUCTURE (Metadata) ---

// Header struct defines the metadata for each block of memory
struct Header {
    size_t size;          // Size of the memory block (excluding the header itself)
    bool is_free;         // Status: true if free, false if allocated
    Header *next;         // Pointer to the next block in the list (used for traversal)
};

// Pointer to the start of the managed heap
void* base_heap = nullptr; 
// Pointer to the first block's header (the head of our linked list of blocks)
Header* free_list_head = nullptr;

// Size of the Header struct itself
const size_t HEADER_SIZE = sizeof(Header);

// --- 3. HELPER FUNCTIONS ---

// Function to get more memory from the OS if needed
// New (Windows)
Header* request_memory(size_t size) {
    Header* header;
    
    // VirtualAlloc reserves and commits a region of pages.
    void* block = VirtualAlloc(
        NULL,                             // System chooses where to allocate
        size + HEADER_SIZE,               // Total size to allocate
        MEM_COMMIT | MEM_RESERVE,         // Reserve the address space AND commit physical storage
        PAGE_READWRITE                    // Set memory protection to read/write
    ); 

    if (block == NULL) { // VirtualAlloc returns NULL on failure
        return nullptr; 
    }

    header = (Header*)block;
    header->size = size;
    header->is_free = true;
    header->next = nullptr;
    
    return header;
}

// --- 4. THE ALLOCATOR: my_malloc() ---

void* my_malloc(size_t size) {
    if (size == 0) return nullptr;

    Header* current = free_list_head;
    Header* best_fit = nullptr;
    size_t min_size = INITIAL_HEAP_SIZE + 1; // Start with a size larger than the heap

    // 4.1. Traverse the list to find the BEST-FIT block (O(n) search)
    // The BEST-FIT algorithm minimizes fragmentation by choosing the smallest possible free block.
    while (current) {
        if (current->is_free && current->size >= size) {
            if (current->size < min_size) {
                min_size = current->size;
                best_fit = current;
            }
        }
        current = current->next;
    }

    // 4.2. Handle No Block Found (Either initialize or extend the heap)
    if (!best_fit) {
        // First allocation OR we need more memory
        best_fit = request_memory(size);
        if (!best_fit) return nullptr; // Failed to get memory

        // If this is the very first block, set it as the head
        if (!free_list_head) {
            free_list_head = best_fit;
        } else {
            // Append the new block to the end of the list (simplest for demonstration)
            current = free_list_head;
            while (current->next) {
                current = current->next;
            }
            current->next = best_fit;
        }
    }

    // 4.3. Split the Block (If the best-fit block is significantly larger)
    if (best_fit->size > (size + HEADER_SIZE + 8)) { // 8 bytes minimum for leftover chunk
        // Calculate the address for the new header for the leftover chunk
        Header* new_block = (Header*)((char*)best_fit + HEADER_SIZE + size);
        
        // Initialize the new header
        new_block->size = best_fit->size - size - HEADER_SIZE;
        new_block->is_free = true;
        new_block->next = best_fit->next;

        // Update the current best_fit block
        best_fit->size = size;
        best_fit->next = new_block;
    }

    // 4.4. Mark the Block as Allocated
    best_fit->is_free = false;

    // Return the memory area, which is HEADER_SIZE bytes past the Header struct
    return (void*)((char*)best_fit + HEADER_SIZE);
}

// --- 5. THE DEALLOCATOR: my_free() ---

void my_free(void* block) {
    if (!block) return;

    // Get the header address by moving backward from the block pointer
    Header* header = (Header*)((char*)block - HEADER_SIZE);

    // Basic check: is the block already free?
    if (header->is_free) {
        std::cerr << "Warning: Attempted to free an already free block." << std::endl;
        return;
    }

    // Mark the block as free
    header->is_free = true;
    
    // NOTE: This implementation skips the complex step of COALESCING adjacent free blocks for simplicity,
    // but coalescing is crucial for reducing external fragmentation.
}

// --- 6. DEMONSTRATION (main function) ---

int main() {
    std::cout << "--- Custom Best-Fit Allocator Demo ---" << std::endl;

    // Allocate 100 bytes
    int* p1 = (int*)my_malloc(100 * sizeof(int)); 
    if (p1) {
        p1[0] = 42;
        std::cout << "1. Allocated 100 bytes at address: " << p1 << std::endl;
    }

    // Allocate 200 bytes
    char* p2 = (char*)my_malloc(200); 
    if (p2) {
        p2[0] = 'A';
        std::cout << "2. Allocated 200 bytes at address: " << (void*)p2 << std::endl;
    }

    // Free the first block
    my_free(p1);
    std::cout << "3. Freed 100-byte block (p1)." << std::endl;

    // Allocate a small block (Best-Fit Test)
    // The allocator should ideally reuse the 100-byte free chunk from p1.
    short* p3 = (short*)my_malloc(32); 
    if (p3) {
        std::cout << "4. Allocated 32 bytes (p3). Best-Fit should reuse p1's slot. Address: " << p3 << std::endl;
    }
    
    // Clean up
    my_free(p2);
    my_free(p3);
    
    std::cout << "--- Demo Complete ---" << std::endl;
    
    return 0;
}

