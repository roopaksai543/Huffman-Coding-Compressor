# Huffman Coding Compressor

Lossless text compression and decompression using Huffman coding. Built to demonstrate frequency tables, priority queues, binary trees, recursion, and file I/O.

## Description

The compression process begins by reading the entire input text file and building a frequency table that records how often each character appears. These frequencies are used to initialize a min-priority queue, where each node represents a character and its associated count. The algorithm repeatedly removes the two lowest-frequency nodes from the queue and merges them into a new parent node, gradually constructing a binary Huffman tree that optimizes for minimal average code length.

Once the Huffman tree is built, it is traversed recursively to assign binary prefix codes to each character. Moving left in the tree appends a `0` to the code, while moving right appends a `1`. Characters that appear more frequently in the input naturally receive shorter codes, while less common characters receive longer ones, resulting in efficient, lossless compression.

The compressed output file is written in two parts. First, a header is generated that stores the mapping between each character and its corresponding Huffman code. This metadata is required for accurate decompression. Following the header, the original file contents are encoded into a compact bitstream by replacing each character with its assigned binary code.

During decompression, the process is reversed. The header is read from the compressed file and used to reconstruct the exact Huffman tree that was created during compression. The encoded bitstream is then read bit-by-bit, traversing the tree from the root until a leaf node is reached. Each leaf corresponds to a decoded character, which is written to the output before resetting traversal back to the root. This continues until the entire bitstream has been decoded, reproducing the original input file exactly.

## How it Works

1. Read an input text file  
2. Build a frequency table from the input  
3. Insert nodes into a min-priority queue ordered by frequency  
4. Repeatedly merge the two lowest-frequency nodes to construct the Huffman tree  
5. Traverse the tree to assign optimal prefix codes  
   - left edge = `0`  
   - right edge = `1`  
6. Write a compressed file containing:
   - a **header** storing character → code mappings  
   - an encoded **bitstream**
7. For decompression:
   - read the header from the compressed file  
   - rebuild the Huffman tree using stored codes  
   - read the encoded bitstream bit-by-bit  
   - traverse the tree until a leaf is reached  
   - output the decoded character and reset to the root
  
## Example Demonstration

**Input Text**  
`banana`

**Character Frequencies**  
`b -> 1`  
`a -> 3`  
`n -> 2`

**Assigned Huffman Codes**  
`a -> 0`  
`b -> 10`  
`n -> 11`

**Encoded Bitstream**  
`banana -> 10 0 11 0 11 0 -> 100110110`

**Compressed File Contents**  
`[header] + 100110110`

## Project Structure

`src/Huffman.cpp` — compression + decompression pipeline implementation  
`src/Huffman.h` — class definition and helper methods  
`src/HuffmanDriver.cpp` — CLI driver (choose compress/decompress)  
`src/Node.h` — Huffman tree node representation  
`src/Storage/` — bit-level storage and header handling

## Key Concepts Demonstrated

- Greedy algorithms 
- Priority queues
- Binary trees
- Recursive traversal
- File streams and custom metadata headers

## Developer Notes

This program is designed for text-based input. It focuses on clarity and correctness over maximum compression ratio. It is implemented as an academic systems project.

Built by **Roopaksai Sivakumar**  
Computer Engineering @ UC Irvine
