
#ifndef NEURAL_NETWORK_NEURON_HPP_
#define NEURAL_NETWORK_NEURON_HPP_

namespace neural_network {

template <class T = float>
class base_neuron {
public:
	using value_type = T;

public:
	base_neuron() : _value(0) {}
	base_neuron(const value_type &value) : _value(value) {}

	const value_type value() const { return _value; }

	void set_value(const value_type &value) { _value = value; }

private:
	value_type _value;
};

using neuron = base_neuron<>;

} // namespace neural_network

#endif // NEURAL_NETWORK_NEURON_HPP_
