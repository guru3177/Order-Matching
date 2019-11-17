#ifndef ORDER_H
#define ORDER_H

#include <cstddef>
#include <string>

enum OrderStatus
{
	OPEN,
	PARTIALLY_FILLED,
	FULLY_FILLED,
	CANCELLED
};

class Order  {
	private:
		std::string order_id_;
		std::size_t order_qty_;
		std::size_t order_executed_qty_;
		std::size_t order_leaves_qty_;
		double order_price_;
		char order_buy_sell_;
		std::size_t order_position_;
		OrderStatus order_status_;
	public:
		Order(std::string const& order_id, std::size_t order_qty, double order_price,
				char buy_sell);

		std::string const& get_order_id() const;
		std::size_t get_order_qty() const;
		void set_order_qty(std::size_t order_qty);
		double get_order_price() const;
		char get_order_side() const;
		std::size_t get_order_executed_qty() const;
		std::size_t get_order_leaves_qty() const;
		std::size_t set_order_executed_qty(std::size_t const exec_qty);
		std::size_t set_order_leaves_qty(std::size_t const lvs_qty);
		void set_order_position(std::size_t position);
		std::size_t get_order_position() const;
		std::string get_order_status_str() const;
		OrderStatus get_order_status() const;
		void set_order_status(OrderStatus const status);

		Order(Order const&) = delete;
		Order& operator=(Order const&) = delete;
};

#endif
