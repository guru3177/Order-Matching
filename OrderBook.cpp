#include "OrderBook.h"
#include <iostream>
#include <cmath>
#include <limits>
#include <sstream>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>

OrderBook::OrderBook(double tick_size, std::shared_ptr<boost::lockfree::spsc_queue<std::string>> commands_queue):
	              tick_size_(tick_size), commands_queue_(commands_queue)  {
}

OrderBook::OrderBook(double tick_size):tick_size_(tick_size) {
}

void OrderBook::remove_amended_up_order(std::string const& order_id, double order_price, PriceOrders& price_orders)  {
	for(auto it = std::begin(price_orders[order_price]); it != std::end(price_orders[order_price]);) {
		if ((*it)->get_order_id() == order_id)  {
			it = price_orders[order_price].erase(it);
		}
		else  {
			++it;
		}
	}
}

void OrderBook::remove_fully_filled_orders(std::set<double>& prices, PriceOrders& price_orders )  {
	for(auto itr = prices.begin(); itr != prices.end();) {
		for(auto it = std::begin(price_orders[*itr]); it != std::end(price_orders[*itr]);) {
			if ((*it)->get_order_status() == OrderStatus::FULLY_FILLED)  {
				it = price_orders[*itr].erase(it);

			}
			else  {
				++it;
			}
		}
		if (price_orders[*itr].size() == 0)  {
			price_orders.erase(*itr);
			itr = prices.erase(itr);
		}
		else  {
			++itr;
		}
	}
}

void OrderBook::run_match()  {
	std::for_each(buy_prices_.rbegin(), buy_prices_.rend(), [&](double const buy_price)  {
		std::for_each(sell_prices_.begin(), sell_prices_.end(), [&](double const sell_price)  {
			if (buy_price >= sell_price)  {
				std::for_each(buy_orders_[buy_price].begin(), buy_orders_[buy_price].end(),
					         	[&](std::shared_ptr<Order>& buy_order)  {
					std::for_each(sell_orders_[sell_price].begin(), sell_orders_[sell_price].end(),
						         	[&](std::shared_ptr<Order>& sell_order)  {
						std::size_t sell_leaves_qty = sell_order->get_order_leaves_qty();
						std::size_t buy_leaves_qty = buy_order->get_order_leaves_qty();
						std::size_t sell_executed_qty = sell_order->get_order_leaves_qty();
						std::size_t buy_executed_qty = buy_order->get_order_executed_qty();

						if (buy_order->get_order_status() != OrderStatus::FULLY_FILLED &&  
							 sell_order->get_order_status() != OrderStatus::FULLY_FILLED) {
							if (buy_leaves_qty >= sell_leaves_qty)  {
								buy_executed_qty += sell_leaves_qty;
								buy_order->set_order_executed_qty(buy_executed_qty);
								buy_leaves_qty -= sell_leaves_qty;
								buy_order->set_order_leaves_qty(buy_leaves_qty);
								if (buy_leaves_qty == 0)
									buy_order->set_order_status(OrderStatus::FULLY_FILLED);
								else
									buy_order->set_order_status(OrderStatus::PARTIALLY_FILLED);
								sell_order->set_order_status(OrderStatus::FULLY_FILLED);
								sell_executed_qty += sell_leaves_qty;
								sell_order->set_order_executed_qty(sell_executed_qty);
								sell_order->set_order_leaves_qty(0);
							}
							else  {
								// Buy has less than the sell side
								sell_executed_qty += buy_leaves_qty;
								sell_order->set_order_executed_qty(sell_executed_qty);
								sell_leaves_qty -= buy_leaves_qty;
								sell_order->set_order_leaves_qty(sell_leaves_qty);
								sell_order->set_order_status(OrderStatus::PARTIALLY_FILLED);
								buy_order->set_order_status(OrderStatus::FULLY_FILLED);
								buy_executed_qty += buy_leaves_qty;
								buy_order->set_order_executed_qty(buy_executed_qty);
								buy_order->set_order_leaves_qty(0);
							}
						}
					});
				});
			}
		});
	});

	// Remove fully filled buy orders
	remove_fully_filled_orders(buy_prices_, buy_orders_);

	// Remove fully filled sell orders
	remove_fully_filled_orders(sell_prices_, sell_orders_);
}

Order const& OrderBook::get_order(std::string const& order_id) const  {
	auto const& itr = orders_orderid_.find(order_id);
	if (itr != orders_orderid_.end())  {
		return *(itr->second);
	}
}

std::string OrderBook::query_level(PriceOrders const& price_orders, std::set<double> const& prices, std::size_t level, 
		                             std::string const& bid_ask) const  {

	std::ostringstream levelinfo;
	levelinfo << std::fixed << std::setprecision(2);

	if (prices.size() - 1 < level)  {
		levelinfo <<  "No entries found for bid_ask " << bid_ask << " level " 
			<< level << " Max size " <<  prices.size();

		return (levelinfo.str());
	}

	double lvl_price = 0;
        if ("Ask" == bid_ask)  {
       		lvl_price = *std::next(prices.begin(), level);
	}
	else if ("Bid" == bid_ask)  {
       		lvl_price = *std::next(prices.rbegin(), level);
	}

	PriceOrders::const_iterator it = price_orders.find(lvl_price);
	if (it != price_orders.end())  {
		auto const& orders(it->second);
		std::size_t total_orders = 0;
		std::for_each(orders.begin(), orders.end(), [&](auto const& order)  {
			total_orders += order->get_order_leaves_qty();
		});

		//std::cout << "From function " <<  bid_ask << ", " << level 
		//                 << ", " << lvl_price << ", " << total_orders << std::endl;
		levelinfo <<  std::string(bid_ask) << ", " << level << ", " << lvl_price << ", " << total_orders;
		return levelinfo.str();
	}
}

std::string OrderBook::query_by_level(std::string const& querylevelbidask, std::size_t querylevel) const {
	if ("bid" == querylevelbidask)
		return query_level(buy_orders_, buy_prices_, querylevel, "Bid");
	else if ("ask" == querylevelbidask)
		return query_level(sell_orders_, sell_prices_, querylevel, "Ask");
}

void OrderBook::run() { 
	std::string cmdvalue;
	while (true)  {
		while(commands_queue_->pop(cmdvalue))  {
			// Split by space
			std::istringstream iss(cmdvalue);
			std::vector<std::string> cmd((std::istream_iterator<std::string>(iss)), 
					              std::istream_iterator<std::string>());

			std::string action = cmd[0];
			if (action == "quit")
				return;

			if (action == "order")  {
				std::string orderno = cmd[1];
				std::size_t qty = boost::lexical_cast<std::size_t>(cmd[3]);
				double price = boost::lexical_cast<double>(cmd[4]);
				char buysell = 'B';
				if (cmd[2] == "sell")
				{
					buysell = 'S';
				}
				add_order(orderno, qty, buysell, price);  
			}
			else if(action == "cancel")  {
				std::string orderno = cmd[1];
				cancel_order(orderno);
			}
			else if(action == "amend")  {
				std::string orderno = cmd[1];
				std::size_t qty = boost::lexical_cast<std::size_t>(cmd[2]);
				amend_order(orderno, qty);
			}
			else if(action == "q")  {
				std::string queryby = cmd[1];
				if (queryby == "level")
				{
					std::string querylevelbidask = cmd[2];
					std::size_t querylevel = boost::lexical_cast<std::size_t>(cmd[3]);
					std::cout << query_by_level(querylevelbidask, querylevel) << std::endl;
				}
				else if (queryby == "order")
				{
					std::string orderid = cmd[2];
					std::cout << "--------------------------------------" << std::endl;
					std::cout << "Order ID " << orderid << std::endl;
					std::cout << "Order status = " 
						  << orders_orderid_[orderid]->get_order_status_str() << std::endl;
					std::cout << "Order position = " 
						  << orders_orderid_[orderid]->get_order_position() << std::endl;
					std::cout << "Order leaves qty = " 
						  << orders_orderid_[orderid]->get_order_leaves_qty() << std::endl;
					std::cout << "--------------------------------------" << std::endl;
				}
			}
		}
	}
}

void OrderBook::cancel_order(std::string const& order_id) {
	auto& order = orders_orderid_[order_id];
	char orderside =  order->get_order_side();
	order->set_order_status(OrderStatus::CANCELLED);
	double order_price = order->get_order_price();
	bool erased = false;
	if ('B' == orderside)  {
		// Buy order
		for(auto it = std::begin(buy_orders_[order_price]); it != std::end(buy_orders_[order_price]);) {
			if ((*it)->get_order_id() == order_id)  {
				it = buy_orders_[order_price].erase(it);
				erased = true;
				if (buy_orders_[order_price].size() == 0)  {
					buy_orders_.erase(order_price);
					buy_prices_.erase(order_price);
					return;
				}
			}
			else  {
				if (erased)  {
					std::size_t newpos = (*it)->get_order_position() - 1;
					(*it)->set_order_position(newpos);
				}
				++it;
			}
		}
	} 
	else {
		for(auto it = std::begin(sell_orders_[order_price]); it != std::end(sell_orders_[order_price]);) {
			if ((*it)->get_order_id() == order_id)  {
				it = sell_orders_[order_price].erase(it);
				erased = true;
				if (sell_orders_[order_price].size() == 0)  {
					sell_orders_.erase(order_price);
					sell_prices_.erase(order_price);
					return;
				}
			}
			else  {
				if (erased)  {
					std::size_t newpos = (*it)->get_order_position() - 1;
					(*it)->set_order_position(newpos);
				}
				++it;
			}
		}
	}
}

bool OrderBook::validate_price(double const order_price) const {
	static const double EPSILON = 1e-8;
	double quo = order_price/tick_size_;
	if (fabs(floor(quo) - quo) < EPSILON || fabs(ceil(quo) - quo) < EPSILON)
		return true;
	else
		return false;
}

void OrderBook::update_order_positions(PriceOrders& order_book, double order_price, std::size_t from_position)  {
	// Update all other order positions by decreasing 1
	auto start_position = order_book[order_price].begin();
	std::advance(start_position, from_position);
	std::for_each(start_position, order_book[order_price].end(), 
         	[&](std::shared_ptr<Order>& order)  {
			std::size_t newposition = order->get_order_position() - 1;
			order->set_order_position(newposition);
	});
}

void OrderBook::amend_order(std::string const& order_id, std::size_t amend_qty)  {
	auto& order = orders_orderid_[order_id];
	if (order->get_order_status() == OrderStatus::OPEN || order->get_order_status() == OrderStatus::PARTIALLY_FILLED) {
		if (order->get_order_leaves_qty() > amend_qty)  {
			// Amended qty less than open qty
			order->set_order_qty(amend_qty);
			order->set_order_leaves_qty(amend_qty);
		}
		else  {
			order->set_order_qty(amend_qty);

			// Order needs to move to the end of the queue
			double order_price = order->get_order_price();
			if (order->get_order_side() == 'B')  {
				update_order_positions(buy_orders_, order_price, order->get_order_position());
                                remove_amended_up_order(order_id, order_price, buy_orders_); 
				order->set_order_position(buy_orders_[order_price].size());
				buy_orders_[order_price].push_back(order);
			}
			else  {
				update_order_positions(sell_orders_, order_price, order->get_order_position());
                                remove_amended_up_order(order_id, order_price, sell_orders_); 
				order->set_order_position(sell_orders_[order_price].size());
				sell_orders_[order_price].push_back(order);
			}
		}
	}
}

void OrderBook::add_order(std::string const& order_id, std::size_t order_qty, char order_side, double order_price)  {
	if (validate_price(order_price))  {
		auto order = std::make_shared<Order>(order_id, order_qty, order_price, order_side);
		orders_orderid_.insert(std::make_pair(order_id, order));
		if ('B' == order_side)  {
			buy_prices_.insert(order_price);
			auto itr = buy_orders_.find(order_price);
			if (buy_orders_.end() == itr)  {
				order->set_order_position(0);
				buy_orders_[order_price] = Orders(1, order);
			}
			else  {
				order->set_order_position(buy_orders_[order_price].size());
				buy_orders_[order_price].push_back(order);
			}
		}
		else if ('S' == order_side)  {
			sell_prices_.insert(order_price);
			auto itr = sell_orders_.find(order_price);
			if (sell_orders_.end() == itr)  {
				order->set_order_position(0);
				sell_orders_[order_price] = Orders(1, order);
			}
			else  {
				order->set_order_position(sell_orders_[order_price].size());
				sell_orders_[order_price].push_back(order);
			}
		}

		run_match();
	}
	else  {
		std::stringstream err_msg;
		err_msg << "Invalid price specified " << order_price;
		throw CustomException(err_msg.str());
	}
}
