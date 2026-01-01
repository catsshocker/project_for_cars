#ifndef MY_QUEUE_H
#define MY_QUEUE_H

template <typename T>
class Queue {
private:
    T* queue;
    int front; 
    int end;   
    int count; 
    int size;  

public:
    Queue(int size) : size(size), front(0), end(0), count(0) {
        queue = new T[size];
    }
    
    ~Queue() {
        delete[] queue;
    }
    
    int push(T data) {
        if (count < size) {
            queue[end] = data;
            end = (end + 1) % size;
            count++;
            return 0;
        }
        return -1;
    }
    
    T pop() {
        if (count > 0) {
            T data = queue[front];
            front = (front + 1) % size;
            count--;
            return data;
        }
        return T();
    }
    
    bool isEmpty() const {
        return count == 0;
    }
    
    int getSize() const {
        return count;
    }
    
    void clear() {
        front = 0;
        end = 0;
        count = 0;
    }
};

#endif