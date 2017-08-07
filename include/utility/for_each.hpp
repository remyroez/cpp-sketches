
#ifndef UTILITY_FOR_EACH_HPP_
#define UTILITY_FOR_EACH_HPP_

#include <tuple>
#include <type_traits>

namespace utility {

namespace detail {

template<class F, class...Ts, std::size_t...Is>
void for_each_in_tuple(const std::tuple<Ts...> & tuple, F func, std::index_sequence<Is...>) {
	using expander = int[];
	(void)expander {
		0, ((void)func(std::get<Is>(tuple)), 0)...
	};
}

template<class F, class...Ts, std::size_t...Is>
void for_each_in_tuple(std::tuple<Ts...> & tuple, F func, std::index_sequence<Is...>) {
	using expander = int[];
	(void)expander {
		0, ((void)func(std::get<Is>(tuple)), 0)...
	};
}

} // namespace detail

template<class F, class...Ts>
void for_each_in_tuple(const std::tuple<Ts...> & tuple, F func) {
	detail::for_each_in_tuple(tuple, func, std::make_index_sequence<sizeof...(Ts)>());
}

template<class F, class...Ts>
void for_each_in_tuple(std::tuple<Ts...> & tuple, F func) {
	detail::for_each_in_tuple(tuple, func, std::make_index_sequence<sizeof...(Ts)>());
}

} // namespace utility

#endif // UTILITY_FOR_EACH_HPP_
