#include "Order.h"

Order::Order(std::string const& order_id, std::size_t order_qty, double order_price,
		char buy_sell):order_id_(order_id),
	                                 order_qty_(order_qty),
	                                 order_leaves_qty_(order_qty),
	     			         order_price_(order_price),
					 order_buy_sell_(buy_sell),
				         order_status_(OrderStatus::OPEN)	 
{
}

void Order::set_order_qty(std::size_t order_qty)  {
	order_qty_ = order_qty;
	set_order_leaves_qty(order_qty);
}

std::string const& Order::get_order_id() const  {
	return order_id_;
}

std::size_t Order::get_order_qty() const  {
	return order_qty_;
}

double Order::get_order_price() const  {
	return order_price_;
}

char Order::get_order_side() const  {
	return order_buy_sell_;
}

std::size_t Order::get_order_executed_qty() const  {
	return order_executed_qty_;
}

std::size_t Order::get_order_leaves_qty() const  {
	return order_leaves_qty_;
}

std::size_t Order::set_order_executed_qty(std::size_t const exec_qty) {
	order_executed_qty_ = exec_qty;
}

std::size_t Order::set_order_leaves_qty(std::size_t lvs_qty) {
	order_leaves_qty_ = lvs_qty;
}

std::size_t Order::get_order_position() const  {
	return order_position_;
}

void Order::set_order_position(std::size_t order_position)  {
	order_position_ = order_position;
}

std::string Order::get_order_status_str() const  {
	if (order_status_ == OrderStatus::OPEN)  
	        return "OPEN" ;
	else if (order_status_ == OrderStatus::PARTIALLY_FILLED) 
		return "PARTIALLY FILLED";
	else if (order_status_ == OrderStatus::FULLY_FILLED) 
		return "FULLY FILLED";
	else if (order_status_ == OrderStatus::CANCELLED) 
		return "CANCELLED";
}

OrderStatus Order::get_order_status() const  {
	return order_status_;
}

void Order::set_order_status(OrderStatus const status) {
	order_status_ = status;
}



