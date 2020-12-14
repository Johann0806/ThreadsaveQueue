#include <queue>
#include <memory>
#include <mutex>
#include <exception>



	/*
	Remarks:
	(1) The pop() methods avoid constructing a new object. Among the reasos is that the constructor may throw an exception -
	for example bad_alloc exception if the allocation of memory for the newly constructed object fails.
	This may lead to cases when the element is poped from the queue but never processed - because returning the element to the
	calling code fails (due to the exception in the constructor
	We implemented two alternatives:
	(a) The element returned by pop(...) is constructed in the calling code and passed as a reference.
	(b) Pop returns a (managed) pointer to the existing element from the queue. If the queue has its own memory management
	we can take advantage of this optimization.
	We are using a managed pointer. This way the memory is freed automatically as soon as we don"t need it anymore.

	(2) The std::queue<>front() call and the std::queue<>pop() call are merged into a single method: pop()
	Otherwise there is a risk race-condition in interface. Imagine the scenario:
	thread 1 calls front and getsitem 8
	thread 2 calls front and gets item 8 as well
	thread 1 calls pop and removes item 8
	thread 2  calls pop and removes item 9
	thread 1 processes item 8 (which it got from pop()
	thread 2 processes item 8 too. (which it got from pop as well)
	=> Item 8 gets processed twice. Item 9 does not get processed at all.
	*/





struct empty_queue: public std::exception
{
	const char* what() const throw()
	{
        return "queue empty exception";
	}
};




template<typename T>
class ThreadsafeQueue
{
private:
	mutable std::mutex m;
	std::queue<T> theQueue;
public:
	ThreadsafeQueue(){}

	ThreadsafeQueue(const ThreadsafeQueue& other)
	{
		std::lock_guard<std::mutex> lock(other.m);
		theQueue=other.theQueue;
		// Don't copy in initializer list but in method body. This way we are sure that the entire copying is locked by mutex
	}

	ThreadsafeQueue& operator=(const ThreadsafeQueue&) = delete;
    // don't allow copying the queue as a whole


   	bool empty() const
	{
		std::lock_guard<std::mutex> lock(m);
		return theQueue.empty();
	}


	void pop(T& value)
	{
		std::lock_guard<std::mutex> lock(m);
		if(theQueue.empty()) throw empty_queue();
		value=theQueue.front();
		theQueue.pop();
	}

	std::shared_ptr<T> pop()
	{
		std::lock_guard<std::mutex> lock(m);
		if(theQueue.empty()) throw empty_queue();
		std::shared_ptr<T> const res(std::make_shared<T>(theQueue.front()));
		theQueue.pop();
		return res;
	}

	void push(T new_value)
	{
		std::lock_guard<std::mutex> lock(m);
		theQueue.push(new_value);
	}





};

