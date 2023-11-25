#ifndef MINIBINHEAP_H
#define MINIBINHEAP_H

// node-oriented heap size
// 30 means depth=4
#define HEAPSIZE 31
#define HEAPLEAF 30 //depth 4 first node

// tree pair  (unsigned int, unsigned int)

// order operand
// '<' means root = min
#define HEAPORDER <


class miniBINheap{

public:
    miniBINheap();

    // father at i=0,2,4...
    // left son at 2*i+2
    // right son at 2*i+4
    unsigned int heapArray[2*HEAPSIZE]; //={0}; C++11 for static init

    void replaceRootAndHeapify(unsigned int, unsigned int);

};

#endif
