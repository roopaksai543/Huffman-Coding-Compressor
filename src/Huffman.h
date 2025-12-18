#include <string>
#include <queue>
#include <fstream>
#include <sstream>
#include "Node.h"
#include "Storage/Storage.h"

#ifndef HUFFMAN_H
#define HUFFMAN_H

class Huffman {

    private:
    Node *root;
    Storage huffmanStorage;
    std::priority_queue<Node *, std::vector<Node *>, compareWeights> queueOfWeights;
    std::unordered_map<char, int> frequencyTable;
    std::unordered_map<char, std::string> codeTable;
    std::queue<Node*> decompressionQueue;
    /**
    * @brief A function that builds a frequency table
    * from the contents of an input file. This table
    * tracks the frequency of each character.
    * @param inputFileContent The content of the input file
    * used to build the frequency table.
    */
    void buildFrequencyTable(std::string inputFileContent);
    /**
    * @brief A function that builds a priority queue
    * based on the frequency table. The queue stores nodes
    * in order of frequency to help in Huffman coding.
    */
    void buildPriorityQueue();
    /**
    * @brief A function that constructs a Huffman tree
    * based on the priority queue from the earlier method.
    * The tree is built by merging nodes together into
    * a tree.
    */
    void buildHuffmanTree();
    /**
    * @brief A recursive function that assigns Huffman codes
    * to each leaf node of the Huffman tree while inserting them
    * into a codeTable map. The function uses preorder traversal
    * through the Huffman Tree and assigns a binary code to each
    * leaf node.
    * @param root The root node of the Huffman tree.
    * @param code The binary code that is manipulated through
    * recursion that is finally assigned to a character.
    */
    void assignCodes(Node* root, std::string code);
    /**
    * @brief The final output function that handles the output of
    * the compression process. It writes the compressed content to
    * a specified output file.
    * @param inputFileContent The content of the input file to be compressed.
    * @param out_file_name The name of the output file where the compressed
    * content will be stored.
    */
    void compressionOutput(std::string inputFileContent, std::string out_file_name);
    /**
    * @brief Builds the Huffman tree used for decompression from the
    * header information in the specified input file. It reads the
    * character-to-code mappings and reconstructs the tree based on
    * the Huffman codes stored in the file's header.
    * @param in_file_name The name of the input file from which the
    * decompression tree will be built.
    */
    void buildDecompressionHuffmanTree(std::string in_file_name);
    /**
    * @brief The final output function that handles the final output of
    * the decompression process. It writes the decompressed content to
    * a specified output file after decoding the compressed data using
    * the Huffman tree.
    * @param out_file_name The name of the output file where the decompressed
    * content will be stored.
    */
    void decompressionOutput(std::string out_file_name);

    public:
    /**
     * The only constructor for the Huffman class
     */
    Huffman();
    /**
    * @brief The "master" compress method that compresses an input
    * file chosen by the user. The compression is then outputted into a
    * selected output file.
    * @param in_file_name The name of the input file that will be compressed.
    * @param out_file_name The name of the output file where the compressed
    * content will be stored.
    */
    void compress(std::string &in_file_name, std::string &out_file_name);
    /**
    * @brief The "master" decompress method that decompresses an input
    * file chosen by the user. The decompression is then outputted into a
    * selected output file.
    * @param in_file_name The name of the input file that will be decompressed.
    * @param out_file_name The name of the output file where the decompressed
    * content will be stored.
    */
    void decompress(std::string &in_file_name, std::string &out_file_name);
};

#endif //HUFFMAN_H
