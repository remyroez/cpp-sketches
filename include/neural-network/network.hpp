
#ifndef NEURAL_NETWORK_NETWORK_HPP_
#define NEURAL_NETWORK_NETWORK_HPP_

#include <memory>
#include <vector>
#include <functional>

#include "neuron.hpp"
#include "connection.hpp"

namespace neural_network {

template <class T = neuron, class U = connection>
class base_network {
public:
	using node_type = T;
	using node_pointer = std::shared_ptr<node_type>;
	using node_handle = std::weak_ptr<node_type>;
	using node_value_type = typename node_type::value_type;
	using node_list_type = std::vector<node_pointer>;
	using node_listing_type = std::vector<node_handle>;
	using node_index_type = typename node_list_type::size_type;

	using connection_type = U;
	using connection_list_type = std::vector<connection_type>;
	using connection_listing_type = std::vector<connection_type *>;
	using connection_index_type = typename connection_list_type::size_type;

	using activation_function_type = std::function<node_value_type(node_pointer)>;

public:
	base_network() {}

	const node_list_type &node_list() const { return _node_list; }
	node_list_type &node_list() { return const_cast<node_list_type &>(static_cast<const base_network *>(this)->node_list()); }

	const node_listing_type &input_nodes() const { return _input_nodes; }
	node_listing_type &input_nodes() { return const_cast<node_listing_type &>(static_cast<const base_network *>(this)->input_nodes()); }

	const node_listing_type &hidden_nodes() const { return _hidden_nodes; }
	node_listing_type &hidden_nodes() { return const_cast<node_listing_type &>(static_cast<const base_network *>(this)->hidden_nodes()); }

	const node_listing_type &output_nodes() const { return _output_nodes; }
	node_listing_type &output_nodes() { return const_cast<node_listing_type &>(static_cast<const base_network *>(this)->output_nodes()); }

	const connection_list_type &connection_list() const { return _connection_list; }
	connection_list_type &connection_list() { return const_cast<connection_list_type &>(static_cast<const base_network *>(this)->connection_list()); }

	void set_activation_function(activation_function_type function) { _activation_function = function; }

	const node_pointer &node(node_index_type index) const { return node_list().at(index); }
	node_pointer &node(node_index_type index) { return const_cast<node_pointer &>(static_cast<const base_network *>(this)->node(index)); }

	template <class... Args>
	node_index_type push_input(Args&&... args) {
		node_index_type index = push_node(std::forward<Args>(args)...);
		input_nodes().push_back(node(index));
		return index;
	}

	template <class... Args>
	node_index_type push_hidden(Args&&... args) {
		node_index_type index = push_node(std::forward<Args>(args)...);
		hidden_nodes().push_back(node(index));
		return index;
	}

	template <class... Args>
	node_index_type push_output(Args&&... args) {
		node_index_type index = push_node(std::forward<Args>(args)...);
		output_nodes().push_back(node(index));
		return index;
	}

	template <class... Args>
	void push_connection(Args&&... args) {
		connection_list().emplace_back(std::forward<Args>(args)...);
	}

	void process() {
		auto list = listing_connections();
		auto it = list.begin();
		while (!list.empty()) {
			auto out_node = node((*it)->out());
			if (!out_node) break;

			for (auto pout_connection : listing_connections_out((*it)->out())) {
				if (pout_connection == nullptr) continue;
				process_connection(*pout_connection);

				list.erase(std::remove(list.begin(), list.end(), pout_connection), list.end());
			}
			out_node->set_value(activation(out_node));
			it = list.begin();
		}
	}

	void learn_connections(std::function<void(connection_type*)> fn) {
		if (!fn) return;

		auto list = listing_connections();
		auto it = list.begin();
		while (!list.empty()) {
			auto out_node = node((*it)->out());
			if (!out_node) break;

			for (auto pout_connection : listing_connections_out((*it)->out())) {
				if (pout_connection == nullptr) continue;
				fn(pout_connection);

				list.erase(std::remove(list.begin(), list.end(), pout_connection), list.end());
			}
			out_node->set_value(activation(out_node));
			it = list.begin();
		}
	}

	void reset(node_value_type value = 0) {
		for (auto node : node_list()) {
			node->set_value(value);
		}
	}

	connection_listing_type listing_connections() {
		connection_listing_type list;

		for (auto &connection : connection_list()) {
			list.push_back(&connection);
		}

		return list;
	}

	connection_listing_type listing_connections(std::function<bool(connection_type)> fn) {
		connection_listing_type list;

		if (fn) {
			for (auto &connection : connection_list()) {
				if (fn(connection)) {
					list.push_back(&connection);
				}
			}
		}

		return list;
	}

	connection_listing_type listing_connections_in(connection_index_type index) {
		return listing_connections([index](auto cn) { return (cn.in() == index); });
	}

	connection_listing_type listing_connections_out(connection_index_type index) {
		return listing_connections([index](auto cn) { return (cn.out() == index); });
	}

protected:
	template <class... Args>
	node_index_type push_node(Args&&... args) {
		node_list().push_back(std::make_shared<node_type>(std::forward<Args>(args)...));
		return (node_list().size() - 1);
	}

	void process_connection(const connection_type *connection) {
		if (connection == nullptr) return;
		auto in_node = node(connection->in());
		auto out_node = node(connection->out());
		out_node->set_value(out_node->value() + in_node->value() * connection->weight());
	}

	node_value_type activation(node_pointer p) {
		node_value_type value = p->value();

		if (_activation_function) {
			value = _activation_function(p);
		}

		return value;
	}

private:
	node_list_type _node_list;
	node_listing_type _input_nodes;
	node_listing_type _hidden_nodes;
	node_listing_type _output_nodes;
	connection_list_type _connection_list;
	activation_function_type _activation_function;
};

using network = base_network<>;

} // namespace neural_network

#endif // NEURAL_NETWORK_NETWORK_HPP_
