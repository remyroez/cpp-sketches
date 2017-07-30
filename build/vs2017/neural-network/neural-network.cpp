
#include <iostream>
#include <vector>
#include <array>
#include <algorithm>

#include "neural-network/neural-network.hpp"

namespace nn = neural_network;

namespace {

// ニューロン
class neuron : public nn::base_neuron<> {
public:
	neuron() : base_neuron(), _threshold(0) {}
	neuron(const value_type &value) : base_neuron(value), _threshold(0) {}
	neuron(const value_type &value, value_type threshold) : base_neuron(value), _threshold(threshold) {}

	value_type threshold() const { return _threshold; }

	void set_threshold(value_type threshold) { _threshold = threshold; }

private:
	value_type _threshold;
};

// ネットワーク
using network = nn::base_network<neuron>;

// テストケースクラス
class test_case {
public:
	using value_type = network::node_value_type;
	using value_list_type = std::vector<value_type>;
	using result_type = int;
	using result_list_type = std::vector<result_type>;

public:
	test_case(const value_list_type &&input_list, const value_list_type &&answer_list)
		: _input_list(input_list), _answer_list(answer_list) {}

	// テスト
	result_type test(network &network) {
		network.reset();

		auto &inputs = network.input_nodes();
		for (size_t i = 0; i < _input_list.size(); ++i) {
			auto node = inputs[i];
			if (auto ptr = node.lock()) {
				ptr->set_value(_input_list[i]);
			}
		}

		print_nodes(network.input_nodes());

		std::cout << " -> ";

		network.process();

		print_nodes(network.output_nodes());

		result_type ok = 0;

		const auto &outputs = network.output_nodes();
		for (size_t i = 0; i < _answer_list.size(); ++i) {
			if (auto ptr = outputs[i].lock()) {
				if (ptr->value() != _answer_list[i]) {
					ok = (ptr->value() > _answer_list[i]) ? 1 : -1;
					break;
				}
			}
		}

		std::cout << " ... ";

		print_values(_answer_list);

		std::cout << " " << ((ok == 0) ? "OK!" : "NG") << std::endl;

		return ok;
	}

	const value_list_type &input_list() const { return _input_list; }
	const value_list_type &answer_list() const { return _answer_list; }

protected:
	// ノードリストの出力
	void print_nodes(network::node_listing_type nodes) {
		std::cout << "[ ";

		bool first = true;
		for (auto node : nodes) {
			if (auto ptr = node.lock()) {
				if (!first) std::cout << ", ";
				std::cout << ptr->value();
				first = false;
			}
		}

		std::cout << " ]";
	}

	// 値リストの出力
	void print_values(value_list_type values) {
		std::cout << "[ ";

		bool first = true;
		for (auto value : values) {
			if (!first) std::cout << ", ";
			std::cout << value;
			first = false;
		}

		std::cout << " ]";
	}

private:
	value_list_type _input_list;
	value_list_type _answer_list;
};

// テストケースリスト
using test_case_list_type = std::vector<test_case>;

// テスト
test_case::result_list_type test(network &network, test_case_list_type &test_cases) {
	::test_case::result_list_type results;

	for (auto testcase : test_cases) {
		results.push_back(testcase.test(network));
	}

	return std::move(results);
}

// 学習
void learn(network &network, const test_case_list_type &test_cases, const test_case::result_list_type &results) {
	for (size_t i = 0; i < results.size(); ++i) {
		if (results[i] == 0) continue;

		// ノード（買ったお菓子）からの接続のウェイトを調整する
		network.learn_connections(
			[&](auto *p) {
				auto in_value = test_cases[i].input_list()[p->in()];
				if ((in_value > 0) || (in_value < 0)) {
					p->set_weight((float)((int)p->weight() + ((results[i] > 0) ? -1 : 1)));
				}
			}
		);

		// 出力ノードのしきい値を調整する
		for (auto node : network.output_nodes()) {
			if (auto ptr = node.lock()) {
				ptr->set_threshold((float)((int)ptr->threshold() + ((results[i] > 0) ? 1 : -1)));
			}
		}
	}
}

// ノードの出力
void print_node(const network::node_pointer node) {
	std::cout
		<< "node { value = " << node->value()
		<< ", threshold = " << node->threshold()
		<< " }"
		<< std::endl;
}

// 接続の出力
void print_connection(const network::connection_type &connection) {
	std::cout
		<< "connection { in = " << connection.in()
		<< ", out = " << connection.out()
		<< ", weight = " << connection.weight()
		<< ", enabled = " << connection.enabled()
		<< " }"
		<< std::endl;
}

// ネットワークの出力
void print_network(const network &network) {
	std::cout << "---------- NETWORK BEGIN" << std::endl;
	for (auto node : network.node_list()) {
		print_node(node);
	}
	for (auto connection : network.connection_list()) {
		print_connection(connection);
	}
	std::cout << "---------- NETWORK END" << std::endl;
}

} // namespace

int main()
{
	::network network;

	// マッチ箱の脳
	{
		// ステップ関数（形式ニューロン）
		network.set_activation_function(
			[](auto x) {
				return static_cast<float>((x->value() < x->threshold()) ? 0 : 1);
			}
		);

		// 入力ノード（お菓子）
		network.push_input(); // 0: 310 yen
		network.push_input(); // 1: 220 yen
		network.push_input(); // 2:  70 yen

		// 出力ノード（マッチ箱）
		// 0 = 買える
		// 1 = 買えない
		network.push_output(0.0f, 6.f);// 3: total / 6 match

		// 接続（マッチ箱）
		network.push_connection(0, 3, 1.f); // 1 match
		network.push_connection(1, 3, 3.f); // 3 match
		network.push_connection(2, 3, 8.f); // 8 match
	}

	print_network(network);

	// 学習
	{
		::test_case_list_type test_cases = {
			{ { 1, 1, 1 },{ 1 } },
			{ { 1, 1, 0 },{ 1 } },
			{ { 1, 0, 1 },{ 0 } },
			{ { 0, 1, 1 },{ 0 } },
			{ { 1, 0, 0 },{ 0 } },
			{ { 0, 1, 0 },{ 0 } },
			{ { 0, 0, 1 },{ 0 } },
			{ { 0, 0, 0 },{ 0 } },
		};

		::test_case::result_list_type results;
		
		std::cout << "---------- LEARN START! " << std::endl;

		for (int i = 0; i < 10; ++i) {
			results = ::test(network, test_cases);
			::learn(network, test_cases, results);
			std::cout << "----------" << std::endl;
		}

		test(network, test_cases);

		std::cout << "---------- LEARN END! " << std::endl;

	}

	// テスト
	{
		::test_case_list_type test_cases = {
			{ { 1, 1, 1 },{ 1 } },
			{ { 1, 1, 0 },{ 1 } },
			{ { 1, 0, 1 },{ 0 } },
			{ { 0, 1, 1 },{ 0 } },
			{ { 0, 0, 0 },{ 0 } },
		};

		test(network, test_cases);
	}

	print_network(network);

#if _DEBUG
	system("pause");
#endif

    return 0;
}

