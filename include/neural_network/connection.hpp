
#ifndef NEURAL_NETWORK_CONNECTION_HPP_
#define NEURAL_NETWORK_CONNECTION_HPP_

namespace neural_network {

class connection {
public:
	using index_type = size_t;
	using weight_type = float;

public:
	connection() : _in(0), _out(0), _weight(1.0f), _enabled(false) {}

	connection(index_type in, index_type out, weight_type weight, bool enabled = true)
		: _in(in), _out(out), _weight(weight), _enabled(enabled) {}

	connection(const connection &other) = default;

	connection &operator =(const connection &other) = default;

	index_type in() const { return _in; }
	index_type out() const { return _out; }
	weight_type weight() const { return _weight; }
	bool enabled() const { return _enabled; }

	void set_in(index_type index) { _in = index; }
	void set_out(index_type index) { _out = index; }
	void set_weight(weight_type weight) { _weight = weight; }
	void set_enabled(bool enabled) { _enabled = enabled; }

private:
	index_type _in;
	index_type _out;
	weight_type _weight;
	bool _enabled;
};

} // namespace neural_network

#endif // NEURAL_NETWORK_CONNECTION_HPP_
