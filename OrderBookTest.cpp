#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE OrderBookTest
#include <boost/test/unit_test.hpp>
#include "OrderBook.h"
#include <iostream>

BOOST_AUTO_TEST_CASE( order_book_test )
{
    OrderBook orderbook(0.2);

    orderbook.add_order("1001", 100, 'B', 12.20);

    BOOST_CHECK( orderbook.get_order("1001").get_order_position() == 0 );

    orderbook.add_order("1002", 10, 'B', 12.20);
    BOOST_CHECK( orderbook.get_order("1002").get_order_status() == OrderStatus::OPEN );

    orderbook.add_order("1003", 200, 'B', 12.80);
    orderbook.add_order("1004", 250, 'B', 12.80);

    BOOST_CHECK(orderbook.query_by_level("bid", 0) == "Bid, 0, 12.80, 450");
    BOOST_CHECK(orderbook.query_by_level("bid", 1) == "Bid, 1, 12.20, 110");

    orderbook.add_order("1005", 150, 'S', 12.00);
    orderbook.add_order("1006", 150, 'S', 11.80);
    BOOST_CHECK( orderbook.get_order("1005").get_order_status() == OrderStatus::FULLY_FILLED );
    BOOST_CHECK( orderbook.get_order("1005").get_order_leaves_qty() == 0 );
    BOOST_CHECK( orderbook.get_order("1003").get_order_status() == OrderStatus::FULLY_FILLED );
    BOOST_CHECK( orderbook.get_order("1003").get_order_leaves_qty() == 0 );
    BOOST_CHECK( orderbook.get_order("1006").get_order_status() == OrderStatus::FULLY_FILLED );
    BOOST_CHECK( orderbook.get_order("1003").get_order_leaves_qty() == 0 );
    BOOST_CHECK( orderbook.get_order("1004").get_order_status() == OrderStatus::PARTIALLY_FILLED );
    BOOST_CHECK( orderbook.get_order("1004").get_order_leaves_qty() == 150 );

    BOOST_CHECK(orderbook.query_by_level("bid", 0) == "Bid, 0, 12.80, 150");
    BOOST_CHECK(orderbook.query_by_level("bid", 1) == "Bid, 1, 12.20, 110");

    orderbook.add_order("1007", 250, 'S', 12.20);

    BOOST_CHECK(orderbook.query_by_level("bid", 0) == "Bid, 0, 12.20, 10");
    BOOST_CHECK( orderbook.get_order("1001").get_order_status() == OrderStatus::FULLY_FILLED );
    BOOST_CHECK( orderbook.get_order("1002").get_order_status() == OrderStatus::OPEN );

    orderbook.add_order("1008", 100, 'B', 11.60);
    orderbook.add_order("1009", 200, 'B', 11.60);
    orderbook.add_order("1010", 300, 'B', 11.60);
    orderbook.add_order("1011", 20, 'S', 11.60);
    BOOST_CHECK( orderbook.get_order("1002").get_order_status() == OrderStatus::FULLY_FILLED );
    BOOST_CHECK( orderbook.get_order("1011").get_order_status() == OrderStatus::FULLY_FILLED );
    BOOST_CHECK(orderbook.query_by_level("bid", 0) == "Bid, 0, 11.60, 590");
    
    orderbook.amend_order("1009", 1000);
    BOOST_CHECK( orderbook.get_order("1009").get_order_position() == 2 );
    orderbook.amend_order("1010", 150);
    BOOST_CHECK( orderbook.get_order("1010").get_order_position() == 1 );
    BOOST_CHECK(orderbook.query_by_level("bid", 0) == "Bid, 0, 11.60, 1240");

    BOOST_CHECK( orderbook.get_order("1008").get_order_status() == OrderStatus::PARTIALLY_FILLED );
    BOOST_CHECK( orderbook.get_order("1008").get_order_leaves_qty() == 90 );
    orderbook.cancel_order("1010");
    BOOST_CHECK( orderbook.get_order("1010").get_order_status() == OrderStatus::CANCELLED );
    BOOST_CHECK(orderbook.query_by_level("bid", 0) == "Bid, 0, 11.60, 1090");
    BOOST_CHECK( orderbook.get_order("1009").get_order_position() == 1 );
}



