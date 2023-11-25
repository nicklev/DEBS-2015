#include "miniBINheap.h"



miniBINheap::miniBINheap()
{
    for(int i=0; i<2*HEAPSIZE; i++)
        heapArray[i]=0;
}


void miniBINheap::replaceRootAndHeapify( unsigned int newKey, unsigned int newValue )
{
    // smaller means HEAPORDER is true
    // greater/equal means HEAPORDER is not true


    // heapify

    int i=0;
    int j;
    while(i<HEAPLEAF)
    {
        j=2*i+2;
        //find smaller child
        if(heapArray[j+2] HEAPORDER heapArray[j])
        {
            // right child is smaller than left child
            j+=2;
        }

        if(heapArray[j] HEAPORDER newKey)
        {
            // child is smaller than father, replace and continue heapify
            heapArray[i] = heapArray[j];
            heapArray[i+1] = heapArray[j+1];
            i=j;
        }
        else
        {
            // father is smaller than child, end of hipify
            heapArray[i] = newKey;
            heapArray[i+1] = newValue;
            return;
        }
    }
    // write newKey and newValue on leaf
    heapArray[i] = newKey;
    heapArray[i+1] = newValue;

}


















//
//    //level 1 - root
//    if(heapArray[2] HEAPORDER heapArray[4])
//    {
//        // left child is smaller than right child
//        if(heapArray[2] HEAPORDER newKey)
//        {
//            // left child is smaller than root, replace and continue heapify
//            heapArray[0] = heapArray[2];
//            heapArray[1] = heapArray[3];
//
//            //level 2, 3 and 4
//            int i=2;
//            int j;
//            while(i<32)
//            {
//                j=2*i+4;
//                //find smaller child
//                if(heapArray[j-2] HEAPORDER heapArray[j])
//                {
//                    // left child is smaller than right child
//                    j-=2;
//                }
//                if(heapArray[j] HEAPORDER newKey)
//                {
//                    // child is smaller than father, replace and continue heapify
//                    heapArray[i] = heapArray[j];
//                    heapArray[i+1] = heapArray[j+1];
//                }
//                else
//                {
//                    // root is smaller than left child, end of hipify
//                    heapArray[i] = newKey;
//                    heapArray[i+1] = newValue;
//                    return;
//                }
//                i=j;
//            }
//        }
//        else
//        {
//            // root is smaller than left child, end of hipify
//            heapArray[0] = newKey;
//            heapArray[1] = newValue;
//            return;
//        }
//    }
//    else
//    {
//        // right child is smaller/equal than/to left child
//        if(heapArray[4] HEAPORDER newKey)
//        {
//            //right child is smaller than root, replace and continue heapify
//            heapArray[0] = heapArray[4];
//            heapArray[1] = heapArray[5];
//
//            //level 2, 3 and 4
//            int i=4;
//            int j;
//            while(i<48)
//            {
//                j=2*i+4;
//                //find smaller child
//                if(heapArray[j-2] HEAPORDER heapArray[j])
//                {
//                    // left child is smaller than right child
//                    j-=2;
//                }
//                if(heapArray[j] HEAPORDER newKey)
//                {
//                    // child is smaller than father, replace and continue heapify
//                    heapArray[i] = heapArray[j];
//                    heapArray[i+1] = heapArray[j+1];
//                }
//                else
//                {
//                    // root is smaller than left child, end of hipify
//                    heapArray[i] = newKey;
//                    heapArray[i+1] = newValue;
//                    return;
//                }
//                i=j;
//            }
//        }
//        else
//        {
//            // root is smaller than right child, end of hipify
//            heapArray[0] = newKey;
//            heapArray[1] = newValue;
//            return;
//        }
//    }



