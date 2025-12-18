#include "Huffman.h"

// private data members

void Huffman::buildFrequencyTable(std::string inputFileContent) {

    // If the frequency table is empty, initialize it with the first character from inputFileContent
    if (frequencyTable.size() == 0) {
        frequencyTable[inputFileContent[0]] = 1;
    }

    // Define an iterator for traversing the frequency table map
    std::unordered_map<char, int>::iterator mapTracer;

    // Loop through the remaining characters in inputFileContent, starting from index 1
    for (int i = 1; i < inputFileContent.length(); i++) {
        char selectedCharacter = inputFileContent[i];

        // Initialize iterator to the beginning of the frequency table for each new character
        mapTracer = frequencyTable.begin();

        // Search the frequency table for the current character
        while (mapTracer != frequencyTable.end()) {
            // If character is found in the table, increment its frequency count
            if (mapTracer->first == selectedCharacter) {
                mapTracer->second++;
                break; // Exit loop as character was found and updated
            }
            // If character is not in the table, insert it with an initial count of 1
            else {
                frequencyTable.insert({selectedCharacter, 1});
            }
            ++mapTracer;
        }
    }
}

void Huffman::buildPriorityQueue() {

    // For loop iterating through the frequency table and placing each
    // item in the priority queue
    for (auto i : frequencyTable) {
        Node* temp = new Node(i.first, i.second - 0);
        queueOfWeights.push(temp);
    }
}

void Huffman::buildHuffmanTree() {

    while(queueOfWeights.size() > 1) {

        // Initializing left Node
        Node* left = queueOfWeights.top();
        queueOfWeights.pop();

        // Initializing right Node
        Node* right = queueOfWeights.top();
        queueOfWeights.pop();

        // Initializing a tempRoot to build tree
        Node* tempRoot = new Node('*', (left->weight) + (right->weight), left, right);
        queueOfWeights.push(tempRoot); // Push tree onto priority queue
    }

    // Setting root to the final tempRoot on the priority queue
    root = queueOfWeights.top();
}

void Huffman::assignCodes(Node* root, std::string code) {

    // Base Case (do nothing if empty)
    if (root == nullptr) {
        return;
    }

    // If the node is a leaf node, insert it onto the codeTable map
    if (root->zero == nullptr && root->one == nullptr) {
        root->code = code;
        codeTable.insert({root->letter, root->code});
    }

    // Traverse the left subtree (add 0 to code if left node)
    assignCodes(root->zero, code + "0");

    // Traverse the right subtree (add 1 to code if right node)
    assignCodes(root->one, code + "1");

}

void Huffman::compressionOutput(std::string inputFileContent, std::string out_file_name) {

    // Attempt to open the output file for writing, and handle error if file cannot be opened
    if (!huffmanStorage.open(out_file_name, "write")) {
        std::cout << "There was an error opening the file." << std::endl;
    }

    // Iterator to traverse the code table that maps characters to their binary Huffman codes
    std::unordered_map<char, std::string>::iterator tracer = codeTable.begin();
    std::string tempHeader; // Header string to store character-code pairs for file metadata
    std::string temp; // Temporary string for storing binary codes of characters

    // Loop to create a header with all character-to-code mappings
    for (int i = 0; i < codeTable.size(); i++) {
        std::string tempString = std::string(1, tracer->first); // Convert character to string
        tempHeader += tempString;
        tempHeader += std::string(1, char(30)); // Use a delimiter (ASCII 30) to separate character-code pairs
        tempHeader += tracer->second; // Append binary code for the character
        tempHeader += std::string(1, char(30)); // Add delimiter after each code
        tracer++;
    }

    // Set the constructed header in huffmanStorage for file metadata
    huffmanStorage.setHeader(tempHeader);

    // Loop through each character in the input content to encode it using the code table
    for (int i = 0; i < inputFileContent.size(); i++) {
        char selectedCharacter = inputFileContent[i];

        // Check if the character exists in the code table
        if (codeTable.find(selectedCharacter) != codeTable.end()) {
            temp = codeTable[selectedCharacter]; // Get Huffman code for the character
            huffmanStorage.insert(temp); // Insert the code into the output file
        }
        else {
            // Output error if character not found in the code table
            std::cout << "Character not found in codeTable: " << selectedCharacter << std::endl;
        }
    }

    // Close the output file after writing all content
    huffmanStorage.close();
}

void Huffman::buildDecompressionHuffmanTree(std::string in_file_name) {

    // Open the input file for reading
    if (!huffmanStorage.open(in_file_name, "read")) {
        std::cout << "There was an error opening the file." << std::endl;
        exit(0);
    };

    std::istringstream header(huffmanStorage.getHeader());
    std::string tempString;

    // Read the header and build nodes for the decompression queue
    while (std::getline(header, tempString, char(30))) {

        Node *temp = new Node(tempString[0], 0);  // Create a new node with the character from the header
        std::getline(header, tempString, char(30));   // Get the Huffman code for the current character
        temp->code = tempString;   // Set code with the code from header
        decompressionQueue.push(temp);  // Push the node into the decompression queue
    }

    Node* tempRoot = new Node('*', 0); // Root node of the decompression Huffman tree

    // Process each node from the queue to build the Huffman tree
    while (!decompressionQueue.empty()) {
        Node* currentNode = decompressionQueue.front();
        decompressionQueue.pop();

        std::string code = currentNode->code; // Huffman code for the current node
        char letter = currentNode->letter;    // Character associated with the Huffman code
        Node* current = tempRoot;             // Start from the root node of the tree

        // Traverse the Huffman code to insert the node into the tree
        for (int i = 0; i < code.size(); i++) {
            char bit = code[i];

            // Move left for '0' and right for '1'
            if (bit == '0') {
                if (!current->zero) {
                    current->zero = new Node('*', 0); // Create left child if it doesn't exist
                }
                current = current->zero; // Move to left child
            } else if (bit == '1') {
                if (!current->one) {
                    current->one = new Node('*', 0); // Create right child if it doesn't exist
                }
                current = current->one; // Move to right child
            }
        }

        // Set the letter at the leaf node
        current->letter = letter;
    }

    root = tempRoot; // Set the tree's root node after construction
}

void Huffman::decompressionOutput(std::string out_file_name) {

    // Initializing a few strings to use through decompression process
    std::string compressedString; // Compressed string with 1's and 0's
    std::string decompressedString; // Decompressed string
    std::string tempString; // Temporary string used for extraction

    while(huffmanStorage.extract(tempString)) {
        compressedString += tempString;
    }

    // Tracer node pointer used to traverse the Huffman tree
    Node* tracer = root;

    // Loop through each bit in the compressed string
    for (int i = 0; i < compressedString.size(); i++) {

        // If we reach a leaf node, add the character to the decompressed string and reset the tracer
        if (tracer->zero == nullptr && tracer->one == nullptr) {
            decompressedString += tracer->letter;
            tracer = root; // Reset tracer to the root node for the next character
        }

        // Move to the left child if the bit is '0'
        if (compressedString[i] == '0' && tracer->zero != nullptr) {
            tracer = tracer->zero;
        }
        // Move to the right child if the bit is '1'
        else if (compressedString[i] == '1' && tracer->one != nullptr) {
            tracer = tracer->one;
        }
    }

    // Open the file with std::ofstream in write mode
    std::ofstream file(out_file_name);

    // Check if the file opened successfully
    if (!file) {
        std::cerr << "Error opening file: " << out_file_name << std::endl;
        return;
    }

    // Write the entire decompressed String into the file
    file << decompressedString;
    file.close(); // Close file

}


// public data members

Huffman::Huffman() {

}

void Huffman::compress(std::string &in_file_name, std::string &out_file_name) {

    // Initializing input file
    std::ifstream inputFile(in_file_name);
    if (!inputFile.is_open()) {
        std::cerr << "Could not open the file!" << std::endl;
    }

    // Initializing strings to get file content
    std::string inputFileContent;
    std::string tempInputString;

    // Inputting content from input file to string
    while(std::getline(inputFile, tempInputString)) {
        inputFileContent += (tempInputString + '\n');
    }
    inputFile.close();

    // Series of functions to complete compression (more information in header file)
    buildFrequencyTable(inputFileContent);
    buildPriorityQueue();
    buildHuffmanTree();
    assignCodes(root, "");
    compressionOutput(inputFileContent, out_file_name);

}

void Huffman::decompress(std::string &in_file_name, std::string &out_file_name) {

    // Series of functions to complete decompression (more information in header file)
    buildDecompressionHuffmanTree(in_file_name);
    decompressionOutput(out_file_name);

}
