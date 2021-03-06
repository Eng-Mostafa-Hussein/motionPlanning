#pragma once

/* This shuffles around pointers to class T with the assumption that class Ops defines:

unsigned int getHeapIndex(const T*)
void setHeapIndex(T*, unsigned int i)
bool pred(const T*, const T*)
*/


#include <limits>
#include <cassert>
#include <vector>

template <class T, class Ops>
class InPlaceBinaryHeap {
public:
	InPlaceBinaryHeap(int size=100) : heap(size), fill(0) {}

	~InPlaceBinaryHeap() {}

	// initalize the heap from a vector, overwriting existing heap
	void createFromVector(const std::vector<T *> &vec) {
		if(heap.size() < vec.size()) {
			unsigned int size = heap.size() * 2;
			while(size < vec.size()) {
				size *= 2;
			}
			heap.resize(size);
		}
		fill = 0;
		for(auto item = vec.begin(); item != vec.end(); ++item) {
			heap[1 + fill++] = *item;
		}
		for(int i = fill / 2; i > 0; i--) {
			siftUp(i);
		}
	}

	void clear() {
		fill = 0;
	}

	// push an item into the heap
	void push(T *data) {
		fill++;
		if(fill >= heap.size()) {
			heap.resize(heap.size() * 2);
		}
		heap[fill] = data;

		Ops::setHeapIndex(data, fill);

		siftUp(fill);
	}

	// peek into the heap
	T *peek() {
		assert(fill > 0);
		return heap[1];
	}

	// pop the front of the heap
	T *pop() {
		assert(fill > 0);
		T *ret_T = heap[1];
		swap(1,fill);
		fill--;
		siftDown(1);
		Ops::setHeapIndex(ret_T, std::numeric_limits<unsigned int>::max());
		return ret_T;
	}

	bool isEmpty() const {
		return fill <= 0;
	}

	int getFill() const {
		return fill;
	}

	// is the data item in the heap currently
	bool inHeap(const T *data) const {
		unsigned int index = Ops::getHeapIndex(data);
		return index >= 1 && index <= fill;
	}

	// the data item may have been updated while in the heap and requires fixing
	void siftFromItem(const T *data) {

		unsigned int index = Ops::getHeapIndex(data);
		if(index > 0 && index <= fill) {
			int parent_index = parent(index);
			if(index > 1 && Ops::pred(heap[index], heap[parent_index])) {
				siftUp(index);
			} else {
				siftDown(index);
			}
		}
	}

	// remove an item from the heap
	void remove(const T *data) {
		unsigned int index = Ops::getHeapIndex(data);
		swap(index,fill);
		fill--;
		if(index <= fill) {
			int parent_index = parent(index);
			if(index > 1 && Ops::pred(heap[index], heap[parent_index])) {
				siftUp(index);
			} else {
				siftDown(index);
			}
		}
	}

	bool checkInvariant() const {
		for(unsigned int i = 1; i < fill; i++) {
			auto l = left(i);
			auto r = right(i);
			if(l < fill && !Ops::pred(heap[i], heap[l])) return false;
			if(r < fill && !Ops::pred(heap[i], heap[r])) return false;
		}
		return true;
	}

protected:
	bool checkIndices() const {
		for(unsigned int i = 1; i <= fill; ++i) {
			if(Ops::getHeapIndex(heap[i]) != i) {
				fprintf(stderr, "%u ?= %u\n", i, Ops::getHeapIndex(heap[i]));
				return false;
			}
		}
		return true;
	}

private:
	inline unsigned int left(unsigned int i) const {
		return (2 * i);
	}

	inline unsigned int right(unsigned int i) const {
		return (2 * i + 1);
	}

	inline unsigned int parent(unsigned int i) const {
		return (i / 2);
	}

	void swap(unsigned int i, unsigned int j) {
		T *temp = heap[i];
		heap[i] = heap[j];
		heap[j] = temp;

		Ops::setHeapIndex(heap[i], i);
		Ops::setHeapIndex(heap[j], j);
	}

	unsigned int childToSwap(unsigned int index) {
		unsigned int left_index = left(index);
		unsigned int right_index = right(index);
		if(left_index <= fill && right_index <= fill) {
			if(Ops::pred(heap[left_index], heap[right_index])) {
				return left_index;
			} else {
				return right_index;
			}
		} else if(left_index <= fill) {
			return left_index;
		} else if(right_index <= fill) {
			return right_index;
		} else {
			return index;
		}
	}

	void siftDown(unsigned int index) {
		unsigned int child_index = childToSwap(index);
		if(Ops::pred(heap[child_index], heap[index])) {
			swap(index, child_index);
			siftDown(child_index);
		}
	}

	void siftUp(unsigned int index) {
		if(index <= 1) return;

		unsigned int parent_index = parent(index);
		if(Ops::pred(heap[index], heap[parent_index])) {
			swap(index, parent_index);
			siftUp(parent_index);
		}
	}

	std::vector<T *> heap;
	unsigned int fill;
};
