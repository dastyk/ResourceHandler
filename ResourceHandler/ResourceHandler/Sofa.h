#ifndef _RESOURCE_HANDLER_SOFA_H_
#define _RESOURCE_HANDLER_SOFA_H_
#include <tuple>
#include <unordered_map>
#include <optional>

namespace Utilz
{
	namespace TupleOperations
	{
		template<std::size_t I = 0, typename... Types>
		inline typename std::enable_if<I == sizeof...(Types), void>::type
			setValue(std::tuple<std::vector<Types>...>& tuple, const std::tuple<Types...>& t, std::size_t index)
		{ }

		template<std::size_t I = 0, class... Types>
		inline typename std::enable_if < I < sizeof...(Types), void>::type
			setValue(std::tuple<std::vector<Types>...>& tuple, const std::tuple<Types...>& t, std::size_t index)
		{
			std::get<I>(tuple)[index] = std::get<I>(t);
			setValue<I + 1, Types...>(tuple, t, index);
		}

		template<std::size_t I = 0, typename... Types>
		inline typename std::enable_if<I == sizeof...(Types), void>::type
			copyValue(std::tuple<std::vector<Types>...>& tuple, std::size_t to, std::size_t from)
		{ }

		template<std::size_t I = 0, class... Types>
		inline typename std::enable_if < I < sizeof...(Types), void>::type
			copyValue(std::tuple<std::vector<Types>...>& tuple, std::size_t to, std::size_t from)
		{
			std::get<I>(tuple)[to] = std::get<I>(tuple)[from];
			copyValue<I + 1, Types...>(tuple, to, from);
		}

		template<std::size_t I = 0, typename... Types>
		inline typename std::enable_if<I == sizeof...(Types), void>::type
			resizeVectorsInTuple(std::tuple<std::vector<Types>...>& tuple, std::size_t newSize)
		{ }

		template<std::size_t I = 0, class... Types>
		inline typename std::enable_if < I < sizeof...(Types), void>::type
			resizeVectorsInTuple(std::tuple<std::vector<Types>...>& tuple, std::size_t newSize)
		{
			std::get<I>(tuple).resize(newSize);
			resizeVectorsInTuple<I + 1, Types...>(tuple, newSize);
		}
	}



	template<class Key, class KeyHash, class... Types>
	class Sofa
	{
	public:
		Sofa(std::size_t size = 64) : used(0), allocated(size)
		{
			Allocate(allocated);
		}
		~Sofa()
		{

		}
		void clear()
		{
			used = 0;
		}

		inline void shrink_to_fit()
		{
			Allocate(allocated);
		}
		inline std::optional<std::pair<Key, std::size_t>> find(const Key key)const
		{
			if (auto const find = map.find(key); find != map.end())
				return { { find->first, find->second } };
			else
				return std::nullopt;
		}

		inline std::size_t size()const { return used; };

		void add(const Key key, const Types... args)
		{
			if (used + 1 > allocated)
				Allocate(allocated * 2);
			auto index = map[key] = used++;
			const auto tpl = std::make_tuple(key, args...);
			TupleOperations::setValue<0, Key, Types...>(tvec, tpl, index);
		}

		std::size_t add(const Key key)
		{
			if (used + 1 > allocated)
				Allocate(allocated * 2);
			return map[key] = used++;
		}

		template<std::size_t N, class type>
		inline void set(std::size_t index, const type&& t)
		{
			std::get<N>(typePointers)[index] = t;
		}

		template<std::size_t N>
		inline auto& get(std::size_t index)
		{
			return std::get<N>(typePointers)[index];
		}

		template<std::size_t N>
		inline auto get()
		{
			return std::get<N>(typePointers).data();
		}


		bool erase(const Key key)
		{
			if (const auto find = map.find(key); find != map.end())
			{
				destroy(find->second);
				return true;
			}
			return false;
		}


		void destroy(std::size_t at)
		{
			auto last = --used;

			auto at_key = std::get<0>(tvec)[at];
			auto last_key = std::get<0>(tvec)[last];

			TupleOperations::copyValue<0, Key, Types...>(tvec, at, last);

			map[last_key] = at;
			map.erase(at_key);
		}


	private:
		std::size_t used;
		std::size_t allocated;

		std::tuple<std::vector<Key>, std::vector<Types>...> tvec;
		std::unordered_map<Key, std::size_t, KeyHash> map;

		void Allocate(std::size_t newSize)
		{
			TupleOperations::resizeVectorsInTuple(tvec, newSize);
			allocated = newSize;
		}
	};
}
#endif //_RESOURCE_HANDLER_SOFA_H_