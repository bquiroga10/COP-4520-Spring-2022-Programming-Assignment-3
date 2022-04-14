#include <bits/stdc++.h>

using namespace std;

using ll = long long;

const int N = 500000;
const int NUM_THREADS = 12;
const int MIN = numeric_limits<int>::min();
const int MAX = numeric_limits<int>::max();

// these are for the ease of giving out random gifts to
// insert and delete from the linked list
vector<thread> servants;
atomic<int> counter = 0;

// create some ordering of the operations so that it makes the simulating easier
vector<int> order(N);
atomic<int> orderFront, orderBack;

// linked list node with value, next, and a mutex
struct node {
  int value;
  node *next;
  mutex m;

  node(int value): value(value) {}
};


class LockOrderedLinkedList {
private:
  node *head;

public:
  LockOrderedLinkedList() {
    // make dummy nodes so that there are always 2 nodes to insert the new node in between
    head = new node(MIN);
    head->next = new node(MAX);
  }

  void insert(int value) {
    // create the new node
    node *curr = new node(value);

    // get the head and lock it
    node *prev = head;
    unique_lock<mutex> prevLock(prev->m);

    // get the next node and lock it
    node *next = prev->next;
    unique_lock<mutex> nextLock(next->m);

    // while the next node is smaller than what we are adding continue going along
    while(value > next->value) {
      prev = next;
      prevLock.swap(nextLock);
      next = next->next;
      nextLock = unique_lock<mutex>(next->m);
    }

    // add the node into the linked list
    prev->next = curr;
    curr->next = next;
  }

  void remove(int value) {
    // get the head and lock it
    node *prev = head;
    unique_lock<mutex> prevLock(prev->m);

    // get the next node and lock it
    node *curr = prev->next;
    unique_lock<mutex> nodeLock(curr->m);

    // get the next node and lock it
    node *next = curr->next;
    unique_lock<mutex> nextLock(next->m);

    // while the curr node is smaller than what we are trying to remove continue going along
    while(value > curr->value) {
      prev = curr;
      prevLock.swap(nodeLock);
      curr = next;
      nodeLock.swap(nextLock);
      // if the curr node is MAX, node doesn't exist, return
      if(curr->value == MAX) {
        return;
      }
      next = next->next;
      nextLock = unique_lock<mutex>(next->m);
    }

    // if the current node is the value, remove it, otherwise, do nothing
    if(curr->value == value) {
      prev->next = next;
    }
  }

  bool search(int value) {
    //get the head and lock it
    node *node = head;
    unique_lock<mutex> nodeLock(node->m);

    // while the value is smaller than what we are trying to search for continue going along
    while(value < node->value) {
      node = node->next;
      auto nextLock = unique_lock<mutex>(node->m);
      nodeLock.swap(nextLock);
    }

    // return whether the node we got is the correct value
    return node->value == value;
  }
};

LockOrderedLinkedList linkedList;

void servant(int id) {

  int operation = 0;
  // while there are nodes to remove, continue going along
  while(orderFront < N) {

    // insert
    if(operation == 0) {
      if(orderBack == N) continue;
      int pos = orderBack++;
      int value = order[pos];
      linkedList.insert(value);
      counter++;
      // flip which operation this servant will do
      operation ^= 1;
      continue;
    }

    // remove
    if(operation == 1) {
      if(orderFront == orderBack) continue;
      int pos = orderFront++;
      int value = order[pos];
      linkedList.remove(value);

      // flip which operation this servant will do
      operation ^= 1;
      continue;
    }
  }
}


int main() {

  // seed our random number generator
  srand(time(0));

  for(int i = 0; i < N; i++) {
    order[i] = i + 1;
  }

  for(int i = 0; i < 10000000; i++) {
    int a = rand() % N;
    int b = rand() % N;
    swap(order[a], order[b]);
  }

  linkedList = LockOrderedLinkedList();

  for(int i = 0; i < NUM_THREADS; i++) {
    servants.push_back(thread(servant, i));
  }

  for(int i = 5000; i <= 500000; i += 5000) {
    while(counter < i);
    cout << ((i * 100) / 500000) << "%" << endl;
  }

  for(int i = 0; i < NUM_THREADS; i++) {
    servants[i].join();
  }
  
  cout << "All gifts have been put on the list and removed." << endl;

  return 0;
}
