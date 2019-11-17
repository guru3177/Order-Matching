#ifndef ORDERBOOK_H
#define ORDERBOOK_H
#include <unordered_map>
#include <vector>
#include <atomic>
#include <memory>
#include "Order.h"
#include <deque>
#include <boost/lockfree/spsc_queue.hpp>
#include <set>
#include <list>
#include <algorithm>

struct CustomException : public std::exception
{
   std::string s;
   CustomException(std::string ss) : s(ss) {}
   ~CustomException() throw () {} 
   const char* what() const throw() { return s.c_str(); }
};

class OrderBook  {
	public:
		using Orders = std::list<std::shared_ptr<Order>>;
		using PriceOrders = std::unordered_map<double, Orders>;
		OrderBook(double tick_size);
		OrderBook(double tick_size, std::shared_ptr<boost::lockfree::spsc_queue<std::string>> commands_queue);
		void add_order(std::string const& order_id, std::size_t order_qty, char order_side, double order_price);
		void amend_order(std::string const& order_id, std::size_t new_qty);
		void cancel_order(std::string const& order_id);
		std::string query_by_level(std::string const& querylevelbidask, std::size_t querylevel) const; 
		Order const& get_order(std::string const& order_id) const;
		void run();

	private:
		void remove_amended_up_order(std::string const& order_id, double order_price, PriceOrders& price_orders);
		void update_order_positions(PriceOrders& order_book, double order_price, std::size_t from_position);
		void remove_fully_filled_orders(std::set<double>& prices, PriceOrders& price_orders );
		void run_match();
		bool validate_price(double const order_price) const;
		std::string query_level(PriceOrders const& price_orders, 
				std::set<double> const& prices, std::size_t level, std::string const& bidask) const; 
		PriceOrders sell_orders_;
		PriceOrders buy_orders_;
		using OrderByOrderId = std::unordered_map<std::string, std::shared_ptr<Order>>;
		OrderByOrderId orders_orderid_;
		double tick_size_;
		std::shared_ptr<boost::lockfree::spsc_queue<std::string>> commands_queue_;
		std::set<double> buy_prices_;
		std::set<double> sell_prices_;
};

#endif

