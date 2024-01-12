//Made by Zakk Loveall
#ifndef QUEUE_ARRAY_H
#define QUEUE_ARRAY_H

#include <cstdlib> //This is for the exit command.
#include <iostream>
#include <queue> //STL Queue
using namespace std;

template <class T>
class QueueArray {
public:
  QueueArray(int);
  ~QueueArray();
  int Asize();
  T Dequeue();
  int Enqueue(const T &item, const int index);
  int QAsize();
  int Qsize(int index);
  T *Qstate(int index);

private:
  int size;        // size of the array
  queue<T> *array;
  int totalItems; // total number of items stored in the queues
};

// Constructor for the queue array.  It sets the default size
// to 10, initializes the private variables size and totalItems
template <class T>
QueueArray<T>::QueueArray(int sz) : size(sz), totalItems(0) {
  size = sz;
  totalItems = 0;
  array = new queue<T>[size];
  if (array == NULL) {
    cout << "Not enough memory to create the array" << endl;
    exit(-1);
  }
}

template <class T>
QueueArray<T>::~QueueArray() {
  delete[] array;
}

template <class T>
int QueueArray<T>::Asize() {
  return size;
}

template <class T>
int QueueArray<T>::Enqueue(const T &item, const int index) {
  if (index < size && index >= 0) {
    array[index].push(item);
    totalItems++;
    return 1;
  } else {
    return -1;
  }
  return 0;
}

template <class T>
T QueueArray<T>::Dequeue() {
  for (int i = 0; i < size; i++) {
    if (array[i].size() > 0) {
      T firstItem = array[i].front();
      array[i].pop();
      totalItems--;
      return firstItem;
    }
  }
  return 0;
}

template <class T>
int QueueArray<T>::QAsize() {
  return totalItems;
}

template <class T>
int QueueArray<T>::Qsize(int index) {
  if (index < size && index >= 0) {
    return array[index].size();
  }
  return -1;
}

template <class T>
T *QueueArray<T>::Qstate(int index) {
  if (index < size && index >= 0) {
    queue<T> copy_queue = array[index];
    T *temp_array = new T[copy_queue.size()];
    for (int i = 0; i < array[index].size(); i++) {
      temp_array[i] = copy_queue.front();
      copy_queue.pop();
    }
    return temp_array;
  }
  return nullptr;
}
#endif
