#include <iostream> //used for user prompt
#include <string> //used to store URLs
#include <regex> //used to extract links from page contents
#include <map> //used to store previous nodes and distances
#include <queue> //used to queue URLs
#include <stack> // used to find the path
#include "HTTPDownloader.h" //used to download web pages
#include <ctime>
#include <chrono>
using namespace std;

enum Prompt {start, finish}; //used to control user prompts;

/**
 * Prompts the user for the starting URL or finishing URL.
 * @param p - If the user provides the "start" prompt parameter, we will prompt the user for a starting URL. Otherwise, prompt for a ending URL.
 * @return - Returns the user's response as a string.
 */
string promptUser(Prompt p)
{
	string response;
	switch (p)
	{
		case (start):

			cout << "Enter the starting Wikipedia URL: ";
			cin >> response;
			return response;
		case (finish):
			cout << "Enter the ending Wikipedia URL: ";
			cin >> response;
			return response;
	}
}

/**
 * Builds a stack of URLs taken to from the starting URL to the endURL.
 * @param startURL - The URL that will be considered the start.
 * @param url - The URL that will have its path gathered.
 * @param prevMap - The map previous URLs that will be used to build the path stack.
 * @return - Returns a stack of strings, where each string is a URL used to get from startURL to URL.
 */
stack<string> getPath(const string &startURL, const string &url, map<string, string> &prevMap)
{
	stack<string> pathStack; //The stack used for returning
	pathStack.push(url); // put our first URL on the stack
	string tempURL = url; //make the first URL our temporary URL
	while (tempURL != startURL)
	{
		pathStack.push(prevMap[tempURL]); //put the temporary URLs previous page in the stack
		tempURL = prevMap[tempURL]; //make the temporary URL's previous page our current page
	}
	return pathStack;
}

/**
 * Extracts the URLs from the provided page using regex and places them into a queue.
 * If the page contains the ending URL, a queue with only that link is returned.
 * This function filters out junk Wikipedia links.
 * @param pc - The html content of the page that is being searched for URLs.
 * @return - Returns a queue of URLs found on the page.
 */
queue<string> extractLinks(string pc, const string &endURL)
{
	string refURL = "https://en.wikipedia.org";
	string match;
	queue<string> links;
	smatch matched;
	regex linkExp("(<a href=\"/wiki/[^\"]+\" title=\")"); //matches an <a href tag followed by a wiki link with a title
	while (regex_search(pc, matched, linkExp))
	{
		match = matched.str(); //get the matched <a href=> tag in string form
		string fullURL = refURL + match.substr(9, match.length() - 18); //take only the link portion of the match and add it to the base Wikipedia URL
		pc = matched.suffix().str(); //update our string position to be past our last match
		/**
		 * This section fixes broken URLs that contain 1 or more newline characters as a result of cURL HTML content being put into a string.
		 */
		size_t occ = fullURL.find('\n'); //find a newline
		while (occ != string::npos)  //if this newline did not occur at the end of string, we must remove it
		{
			fullURL.erase(occ, 1); //remove the character
			occ = fullURL.find('\n'); //try to find the next newline character
		}
		/**
		 * This section checks to see if URL we are currently looking at on this page is our goal URL.
		 * If it is, return a queue with only this link in it, we can ignore any other URL on this page.
		 */
		if(fullURL == endURL)
		{
			queue<string> q; //create a queue with only the end URL if it can be found on this page
			q.push(fullURL);
			return q;
		}
		/**
		 * This section puts all the URLs found on the page into the returned queue unless they are an invalid Wikipedia link
		 */
		else
		{
			if ("https://en.wikipedia.org/wiki/Wikipedia:" != fullURL.substr(0, 40) &&
			   "https://en.wikipedia.org/wiki/Talk:" != fullURL.substr(0, 35) &&
			   "https://en.wikipedia.org/wiki/Template:" != fullURL.substr(0, 40) &&
			   "https://en.wikipedia.org/wiki/Template_talk:" != fullURL.substr(0, 45) &&
			   "https://en.wikipedia.org/wiki/Help:" != fullURL.substr(0, 35) &&
			   "https://en.wikipedia.org/wiki/Special:" != fullURL.substr(0, 38) &&
			   "https://en.wikipedia.org/wiki/Portal:" != fullURL.substr(0, 37) &&
			   "https://en.wikipedia.org/wiki/Category:" != fullURL.substr(0, 39) &&
			   "https://en.wikipedia.org/wiki/Main_Page" != fullURL)
			{
				links.push(fullURL);
			}
		}
	}
	return links;
}

/**
 * Handles retrieving the Queue for a given URL.
 * It doownloads the page, extract the links, and save the queue created before returning.
 * @param pd - The page downloader used to download the HTML for unarchived pages.
 * @param url - The URL of the queue we are trying create.
 * @param endURL - The end URL for the pathfinder used for shortcutting queues.
 * @param pageURLs - The map of page URL queues for each URL.
 * @return - Returns the queue of URLs found on the page of a given URL.
 */
queue<string> retrieveURLQueue(HTTPDownloader & pd, const string &url, const string &endURL, map<string, queue<string>> &pageURLs)
{
	string pageContent = pd.download(url); //download the page content from that URL
	queue<string> newURLs =  extractLinks(pageContent, endURL); //get all the valid and formatted URLs from that page
	return newURLs;
}

/**
 * Uses a breadth-first search to find the fastest path from the start URL to the end URL.
 * Stops search as soon as the end URL.
 * @param pd - The HTTP downloader object used to retrieve page info.
 * @param startURL - The starting URL of the path.
 * @param endURL - The ending URL owe are trying to build a path to.
 * @param distMap - The depth of each page, might be able to be removed?
 * @param prevMap - The map of strings telling you which node led to the key node.
 */
void findPath(HTTPDownloader &pd, const string &startURL, const string &endURL, map<string, string> &prevMap)
{
	map<string, queue<string>> pastURLQueues;
	queue<string> urlQueue; //set an empty queue of URLs to explore
	urlQueue.push(startURL); //put our starting URL into the queue
	int nodesVisited = 0;
	/**
	 * While our queue of URLS needing to be explored is not empty
	 */
	while (!urlQueue.empty())
	{
		string currentURL = urlQueue.front(); //retrieve and pop the first URL in the queue
		urlQueue.pop();
		queue<string> newURLs = retrieveURLQueue(pd, currentURL, endURL, pastURLQueues);
		++nodesVisited;
		/**
		 * Get every node off of the page we are exploring and check each next node.
		 */
		while (!newURLs.empty()) //until our queue of newly discovered URL is empty
		{
			string tempURL = newURLs.front(); //retrieve and pop the first URL in the queue
			newURLs.pop();
			/**
			 * If we have not visited this page before, or got here faster than before, we must update its info and visit it later.
			 */
			if (prevMap[tempURL] == "" && tempURL != startURL) //if we have not visited this page before
			{
				prevMap[tempURL] = currentURL; //make the previous URL our current
				if (tempURL == endURL) //if we found the endURL, stop searching
				{
					cout << "Path searching " << nodesVisited << " pages!" << endl;
					return;
				}
				else //if this isn't the goal, put it back in to go further.
				{
					urlQueue.push(tempURL); //place the URL back in the queue to update the URLs it can access based off of its new distance
				}
			}
		}
	}
}

/**
 * The main for the Wikipedia Pathfinder. Finds the shortest path between 2 pages.
 * Uses a map to keep track of the shortest path from any page to the finishing page.
 * Uses a queue to keep track of pages needed to be accessed.
 * @return - Returns exit code 0.
 */
int main(int argc, char* argv[])
{
	HTTPDownloader pageDownloader; //object used to download page content and place it into a string
	map<string, string> urlPrevMap; //map of previous URL taken to arrive to each key

	string startURL = promptUser(start);
	string endURL = promptUser(finish);

	time_t time = chrono::system_clock::to_time_t(chrono::system_clock::now());
	cout << "Starting path discovery at: " << ctime(&time);

	findPath(pageDownloader, startURL, endURL, urlPrevMap);

	time = chrono::system_clock::to_time_t(chrono::system_clock::now());
	cout << "Finished path discovery at: " << ctime(&time);

	stack<string> pathStack = getPath(startURL, endURL, urlPrevMap);
	cout << "Path completed by taking " << pathStack.size() - 1  << " Wikipedia Links!" << endl;
	while(!pathStack.empty())
	{
		cout << pathStack.top() << endl;
		pathStack.pop();
	}

	return 0;
}