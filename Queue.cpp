#include <iostream>
#include <string>
using namespace std;

/**
 * The List Node class used for the Queue class.
 * Each list node contains a string for info and a pointer to the next list node.
 */
class ListNode
{
public:

	string info;
	ListNode * next;

	/**
	 * List Node constructor
	 * @param i - The string that will be used as the info for the list node.
	 * @param n - The list node pointer that will be used as the next pointer for the list node.
	 */
	ListNode(string i, ListNode * n) :info(i), next(n)
	{};

};

/**
 * The Queue class used for the Wikipedia Pathfinder to store URLs that need to be explored.
 * The queue contains a head pointer for deQueueing items.
 * The queue contains a tail pointer for putting objects into the queue.
 * The queue contains a size integer telling how m any items are in it.
 */
class Queue
{

private:

	ListNode * head;
	ListNode * tail;
	int size;

public:

	/**
	 * The constructor for a queue. Sets the head and tail pointer to null and sets the size to 0.
	 */
	Queue() :head(nullptr), tail(nullptr), size(0)
	{};

	/**
	 * The constructor for a queue that places a single item in the queue.
	 * Used for returning a queue or only the end url of the pathfinder.
	 * @param s - the string used for the info of the first item in the queue.
	 */
	Queue(string s) :size(1)
	{
		head = new ListNode(s, nullptr);
		tail = head;
	}

	/**
	 * Returns a boolean value depending on whether or not the queue is empty.
	 * @return - true if the queue size is 0, false otherwise.
	 */
	bool isEmpty()
	{
		return size == 0;
	}

	/**
	 * Places an item into the queue and increments the size.
	 * @param i - The string for the info of the list node that will be added to the end of the queue.
	 */
	void enQueue(string i)
	{
		if (i != "")
		{
			if (size == 0)
			{
				head = new ListNode(i, nullptr);
				tail = head;
			} else
			{
				tail->next = new ListNode(i, nullptr);
				tail = tail->next;
			}
			++size;
		}
	}

	/**
	 * Removes a list node from the queue, saves its info, deletes the list node, and returns the info.
	 * @return - The info of the list node removed from the queue.
	 */
	string deQueue()
	{
		string ret;
		if (size > 0)
		{
			ListNode *temp = head;
			ret = temp->info;
			head = head->next;
			delete temp;
			--size;
			return ret;
		}
		else
			return string();
	}

	string front()
	{
		return head->info;
	}

	/**
	 * The destructor for the queue class.
	 */
	~Queue()
	{
		ListNode * temp;
		for (head; head != nullptr;)
		{
			temp = head;
			head = head->next;
			delete temp;
		}
	}

};
