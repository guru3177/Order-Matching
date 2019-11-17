#include <iostream>
#include <memory>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include <deque>
#include "OrderBook.h"

int main(int argc, char *argv[]) {
	try  {

		/*
		if (2 != argc) {
			std::cout << "Usage: BlockOne <tick size> " << std::endl; 
			exit(0);
		}
		*/

		//std::deque<std::string> commands;
		std::shared_ptr<boost::lockfree::spsc_queue<std::string>> commands = 
			std::make_shared<boost::lockfree::spsc_queue<std::string>>(100);

		// Initialise OrderBook
		std::shared_ptr<OrderBook> ob = std::make_shared<OrderBook>(0.2, commands);
		boost::thread obThread(boost::bind(&OrderBook::run, ob));
		
		std::string inp_string;
		while(true)  {
			std::getline(std::cin, inp_string);
			if (boost::iequals("quit", inp_string))  {
			        commands->push(inp_string);
				obThread.join();
				exit(0);
			}

			commands->push(inp_string);
		}

	}
	
	catch(...)  {
		std::cout << "Exception caught " << std::endl;
	}

}
