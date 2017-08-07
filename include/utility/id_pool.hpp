
#ifndef UTILITY_ID_POOL_HPP_
#define UTILITY_ID_POOL_HPP_

#include <deque>
#include <limits>
#include <algorithm>

namespace utility {

template <class T = unsigned int, T Min = std::numeric_limits<T>::min(), T Max = std::numeric_limits<T>::max()>
class id_pool {
public:
	using id_type = T;
	using free_id_container = std::deque<id_type>;

	static constexpr id_type min_id = Min;
	static constexpr id_type max_id = Max;

	constexpr id_pool() = default;

	id_type allocate() {
		id_type id = max_id;

		if (!_free_ids.empty()) {
			id = _free_ids.back();
			_free_ids.pop_back();

		} else if (_current_id == max_id) {
			id = max_id;

		} else {
			id = _current_id;
			_current_id++;
		}

		return id;
	}

	void free(const id_type &id) {
		_free_ids.push_back(id);
	}

	void free(id_type &&id) {
		_free_ids.push_back(id);
	}

	void clear() {
		_free_ids.clear();
		_current_id = min_id;
	}

private:
	id_type _current_id = min_id;
	free_id_container _free_ids;
};

} // namespace utility

#endif // UTILITY_ID_POOL_HPP_
