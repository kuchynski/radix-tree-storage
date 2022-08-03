//
// Associated memory
// kuchynskiandrei@gmail.com
// 2020 - 2022
//

#ifndef H_AMEMORYCPP
#define H_AMEMORYCPP

#include <iostream>
#include <array>
#include <string.h>

unsigned long memory_used = 0; // for the debug purpose only

template <typename T, size_t footpace>
class AMemory
{
	static_assert(footpace == 1 || footpace == 2 || footpace == 4 || footpace == 8, "\'footpace\' can only be 1, 2, 4 or 8");
private:
	static constexpr auto m_n_elements = 1 << footpace;
	static constexpr auto m_n_steps = 8 / footpace;
	static constexpr auto m_steps_mask = m_n_steps - 1;
	static constexpr unsigned char m_mask = m_n_elements - 1;
	
	std::array<class AMemory<T, footpace>*, m_n_elements> m_elements;

	const size_t m_step;
	
	unsigned char *m_key = nullptr;
	size_t m_key_size = 0;
	T *m_value = nullptr;
	
	T *m_end_point_value = nullptr;
	
	size_t m_size = 0;
	static T end_element;

	bool compare_key(const unsigned char *key, const size_t size) const;
	unsigned char get_symbol(const unsigned char key) const;

protected:
	bool insertInternal(const unsigned char *key, const size_t size, const T &value);
	size_t eraseInternal(const unsigned char *key, const size_t size);
	T& findInternal(const unsigned char *key, const size_t size) const;

public:
	AMemory(size_t step = 0);
	~AMemory();

	virtual bool insert(const std::string& key, const T &value);
	virtual bool insert(const std::pair<std::string, T> &pair);
	virtual size_t erase(const std::string& key);
	void clear();
	size_t size() const {return m_size;}
	T& end() const {return end_element;}
	virtual T& find(const std::string& key) const;
};

template <typename T, size_t footpace>
T AMemory<T, footpace>::end_element;

template <typename T, size_t footpace>
AMemory<T, footpace>::AMemory(size_t step): m_step(step)
{
	for (auto &element: m_elements)
		element = nullptr;
}

template <typename T, size_t footpace>
AMemory<T, footpace>::~AMemory()
{
	clear();
}

template <typename T, size_t footpace>
void AMemory<T, footpace>::clear()
{
	for (auto &element: m_elements) {
		if(element) {
			delete element;
			element = nullptr;
		}
	}
	if (m_value) {
		delete m_value;
		m_value = nullptr;
	}
	if (m_end_point_value) {
		delete m_end_point_value;
		m_end_point_value = nullptr;
	}
	m_size = 0;
}

template <typename T, size_t footpace>
bool AMemory<T, footpace>::compare_key(const unsigned char *key, const size_t size) const
{
	if (m_key_size == size)
		return memcmp(m_key, key, size) == 0;
	return false;
}

template <typename T, size_t footpace>
unsigned char inline AMemory<T, footpace>::get_symbol(const unsigned char key) const
{
	return (key >> m_step * footpace) & m_mask;
}

// ----------------------------------------------------- Add ------------------------------------
template <typename T, size_t footpace>
bool AMemory<T, footpace>::insert(const std::string& key, const T &value)
{
	const unsigned char *ptr = (const unsigned char *)key.c_str();
	return insertInternal(ptr, key.size(), value);
}

template <typename T, size_t footpace>
bool AMemory<T, footpace>::insert(const std::pair<std::string, T> &pair)
{
	const unsigned char *ptr = (const unsigned char *)pair.first.c_str();
	return insertInternal(ptr, pair.first.size(), pair.second);
}

template <typename T, size_t footpace>
bool AMemory<T, footpace>::insertInternal(const unsigned char *key, const size_t size, const T &value)
{
	if(size == 0) {
		if(m_end_point_value == nullptr) {
			//std::cout << "Add2 " << value << std::endl;
			m_end_point_value = new T(value); // set successfully
			m_size++;
			return true;
		}
		//std::cout << "Add duplicate 2 " << value << std::endl;
		return false;
	}

	if(m_size == 0) {
		//std::cout << "Add1 " << value << std::endl;
		m_value = new T(value);
		m_key = new unsigned char[size];
		memcpy(m_key, key, size); // set successfully
		m_key_size = size;
		m_size = 1;
		return true;
	}

	const auto next_step = (m_step + 1) & m_steps_mask;
	if(m_value) {
		if (compare_key(key, size)) {
			//std::cout << "Add duplicate 1 " << key << " " << value << std::endl;
			return false;
		}
		const unsigned char symbol = get_symbol(m_key[0]);
		if(m_elements[symbol] == nullptr) {
			m_elements[symbol] = new AMemory<T, footpace>(next_step);
			memory_used += sizeof(AMemory<T, footpace>);
		}
		const auto next_size = next_step? m_key_size : m_key_size - 1;
		const auto next_key = next_step? m_key : m_key + 1;
		if (m_elements[symbol]->insertInternal(next_key, next_size, *m_value) == false) {
			return false;
		}
		delete m_value;
		m_value = nullptr;
		delete [] m_key;
		m_key = nullptr;
	}

	const unsigned char symbol = get_symbol(key[0]);
	if(m_elements[symbol] == nullptr) {
		m_elements[symbol] = new AMemory<T, footpace>(next_step);
		memory_used += sizeof(AMemory<T, footpace>);
	}
	const auto next_size = next_step? size : size - 1;
	const auto next_key = next_step? key : key + 1;
	const auto ret = m_elements[symbol]->insertInternal(next_key, next_size, value);
	if (ret)
		m_size++;

	return ret;
}

// ----------------------------------------------------- Remove ------------------------------------

template <typename T, size_t footpace>
size_t AMemory<T, footpace>::erase(const std::string& key)
{
	const unsigned char *ptr = (const unsigned char *)key.c_str();
	return eraseInternal(ptr, key.size());
}

template <typename T, size_t footpace>
size_t AMemory<T, footpace>::eraseInternal(const unsigned char *key, const size_t size)
{
	size_t ret = 0;

	if(size == 0) {
		if (m_end_point_value) {
			delete m_end_point_value;
			m_end_point_value = nullptr;
			ret = 1;
		}
	}
	else if (m_value == nullptr) {
		const auto symbol = get_symbol(key[0]);
		auto element = m_elements[symbol];
		if (element) {
			const auto next_step = (m_step + 1) & m_steps_mask;
			const auto next_size = next_step? size : size - 1;
			const auto next_key = next_step? key : key + 1;
			ret = element->eraseInternal(next_key, next_size);
			if (element->size() == 0) {
				delete element;
				m_elements[symbol] = nullptr;
			}
		}
	}
	else if (compare_key(key, size)) {
		delete m_value;
		m_value = nullptr;
		delete [] m_key;
		ret = 1;
	}

	if (ret)
		m_size--;

	return ret;
}

// ----------------------------------------------------- Get ------------------------------------
template <typename T, size_t footpace>
T& AMemory<T, footpace>::find(const std::string& key) const
{
	const unsigned char *ptr = (const unsigned char *)key.c_str();
	return findInternal(ptr, key.size());
}

template <typename T, size_t footpace>
T& AMemory<T, footpace>::findInternal(const unsigned char *key, const size_t size) const
{
	if(size == 0) {
		return m_end_point_value? *m_end_point_value : end_element;
	}

	if (m_value == nullptr) {
		const auto symbol = get_symbol(key[0]);
		auto element = m_elements[symbol];
		const auto next_step = (m_step + 1) & m_steps_mask;
		const auto next_size = next_step? size : size - 1;
		const auto next_key = next_step? key : key + 1;
		return element? element->findInternal(next_key, next_size) : end_element;
	}

	return compare_key(key, size)? *m_value : end_element;
}


template <typename B, typename T, size_t footpace>
class MMemory : public AMemory<T, footpace>
{
public:

	bool insert(const B& key, T &value)
	{
		const unsigned char *ptr = (const unsigned char *)&key;
		return AMemory<T, footpace>::insertInternal(ptr, sizeof(B), value);
	}

	bool insert(const std::pair<B, T> &pair)
	{
		const unsigned char *ptr = (const unsigned char *)&pair.first;
		return AMemory<T, footpace>::insertInternal(ptr, sizeof(B), pair.second);
	}

	size_t erase(const B& key)
	{
		const unsigned char *ptr = (const unsigned char *)&key;
		return AMemory<T, footpace>::eraseInternal(ptr, sizeof(B));
	}

	T& find(const B& key) const
	{
		const unsigned char *ptr = (const unsigned char *)&key;
		return AMemory<T, footpace>::findInternal(ptr, sizeof(B));
	}
};


#endif
