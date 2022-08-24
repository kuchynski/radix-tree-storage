//
// the radix tree
// kuchynskiandrei@gmail.com
// 2022
//

#ifndef H_radix_treeCPP
#define H_radix_treeCPP

#include <iostream>
#include <array>
#include <string.h>

int rter = 0;
template <typename T>
class radix_element
{
protected:
	static constexpr auto m_key_width_bits = 4;
	static constexpr auto m_elements_number = 1 << m_key_width_bits;
	static constexpr unsigned char m_key_mask = m_elements_number - 1;

	radix_element() {}
	bool insertNext(T *value);
	void move(radix_element<T> *element);
	static T& end() {return end_element;}

private:

	unsigned char *m_key = nullptr;
	size_t m_key_size = -1;
	T *m_value = nullptr;
	static T end_element;

public:
	radix_element(const unsigned char *key, const size_t size, T *value);
	virtual ~radix_element();
	virtual size_t eraseInternal(const unsigned char *key, const size_t size);
	virtual T& findInternal(const unsigned char *key, const size_t size) const;
	virtual size_t size() const {return m_value == nullptr? 0 : 1;}
	unsigned char decrease(const bool odd_step);
	void clear();
};

template <typename T>
class radix_tree: public radix_element<T>
{
private:
	std::array<class radix_element<T>*, radix_element<T>::m_elements_number> m_elements;
	size_t m_size = 0;
	const bool m_odd_step;

	bool insertNext(const unsigned char *key, const size_t size, T *value);

protected:
	bool insertFirst(const unsigned char *key, const size_t size, const T &value);
	size_t eraseInternal(const unsigned char *key, const size_t size) override;
	T& findInternal(const unsigned char *key, const size_t size) const override;

public:
	radix_tree(bool odd_step = false);
	radix_tree(radix_element<T> *element, bool odd_step = false);
	~radix_tree();

	virtual bool insert(const std::string& key, const T &value);
	virtual bool insert(const std::pair<std::string, T> &pair);
	virtual size_t erase(const std::string& key);
	virtual T& find(const std::string& key) const;
	void clear();
	size_t size() const override {return m_size;}
	static T& end() {return radix_element<T>::end();}
};

template <typename T>
T radix_element<T>::end_element;

template <typename T>
radix_tree<T>::radix_tree(bool odd_step): m_odd_step(odd_step)
{
	for (auto &element: m_elements)
		element = nullptr;
}

template <typename T>
unsigned char radix_element<T>::decrease(const bool odd_step)
{
	if (odd_step == true) {
		const unsigned char *old_key = m_key;
		unsigned char ret = old_key[0] >> 4;

		if (--m_key_size) {
			m_key = new unsigned char[m_key_size];
			memcpy(m_key, old_key + 1, m_key_size);
		}
		else
			m_key = nullptr;

		delete [] old_key;
		return ret;
	}

	return (m_key_size == 0)? m_elements_number : m_key[0] & m_key_mask;
}

template <typename T>
void radix_element<T>::move(radix_element<T> *element)
{
	radix_element<T>::insertNext(element->m_value);
	element->m_value = nullptr;
	delete element;
}

template <typename T>
radix_tree<T>::radix_tree(radix_element<T> *element, bool odd_step): m_odd_step(odd_step), m_size(1)
{
	for (auto &element: m_elements)
		element = nullptr;

	const unsigned char symbol = element->decrease(m_odd_step);
	if (symbol == radix_element<T>::m_elements_number) {
		radix_element<T>::move(element);
	}
	else {
		m_elements[symbol] = element;
	}
}

template <typename T>
radix_element<T>::radix_element(const unsigned char *key, const size_t size, T *value): m_key_size(size), m_value(value)
{
	m_key = new unsigned char[m_key_size];
	memcpy(m_key, key, m_key_size);
}

template <typename T>
radix_element<T>::~radix_element()
{
	clear();
}

template <typename T>
radix_tree<T>::~radix_tree()
{
	clear();
}

template <typename T>
void radix_element<T>::clear()
{
	if (m_value) {
		delete m_value;
		m_value = nullptr;
	}
	if (m_key) {
		delete m_key;
		m_key = nullptr;
	}
	m_key_size = -1;
}

template <typename T>
void radix_tree<T>::clear()
{
	for (auto &element: m_elements) {
		if(element) {
			delete element;
			element = nullptr;
		}
	}
	m_size = 0;
}

// ----------------------------------------------------- Add ------------------------------------
template <typename T>
bool radix_tree<T>::insert(const std::string& key, const T &value)
{
	return insertFirst((const unsigned char *)key.c_str(), key.size(), value);
}

template <typename T>
bool radix_tree<T>::insert(const std::pair<std::string, T> &pair)
{
	return insertFirst((const unsigned char *)pair.first.c_str(), pair.first.size(), pair.second);
}

template <typename T>
bool radix_tree<T>::insertFirst(const unsigned char *key, const size_t size, const T &value)
{
	T *value_ptr = new T(value);
	const auto res = insertNext(key, size, value_ptr);
	
	if (res == false)
		delete value_ptr;
	return res;
}

template <typename T>
bool radix_element<T>::insertNext(T *value)
{
	if (m_value) {
		return false;
	}

	m_value = value;
	m_key_size = 0;
	return true;
}

template <typename T>
bool radix_tree<T>::insertNext(const unsigned char *key, const size_t size, T *value)
{	
	bool ret = false;
	if(size == 0) {
		ret = radix_element<T>::insertNext(value);
	}
	else {
		const unsigned char symbol = m_odd_step? key[0] >> 4 : key[0] & radix_element<T>::m_key_mask;
		auto element = m_elements[symbol];

		if(element == nullptr) {
			const unsigned char *next_key = m_odd_step? key + 1: key;
			const size_t next_size = m_odd_step? size - 1 : size;
			m_elements[symbol] = new radix_element<T>(next_key, next_size, value);
			ret = true;
		}
		else {	
			radix_tree<T>* a_element = dynamic_cast<radix_tree<T>*>(element);
			if(a_element == nullptr) {
				a_element = new radix_tree<T>(element, !m_odd_step);
				m_elements[symbol] = a_element;
			}

			ret = a_element->insertNext(m_odd_step? key + 1: key, m_odd_step? size - 1 : size, value);
		}
	}

	if (ret)
		m_size++;

	return ret;
}

// ----------------------------------------------------- Remove ------------------------------------

template <typename T>
size_t radix_tree<T>::erase(const std::string& key)
{
	const unsigned char *ptr = (const unsigned char *)key.c_str();
	return eraseInternal(ptr, key.size());
}

template <typename T>
size_t radix_element<T>::eraseInternal(const unsigned char *key, const size_t size)
{
	if (m_key_size == size) {
		if (memcmp(m_key, key, size) == 0) {
			clear();
			return 1;
		}
	}
	return 0;
}

template <typename T>
size_t radix_tree<T>::eraseInternal(const unsigned char *key, const size_t size)
{
	size_t ret = 0;

	if (size) {
		const auto symbol = m_odd_step? key[0] >> 4 : key[0] & radix_element<T>::m_key_mask;
		auto element = m_elements[symbol];
		if (element) {
			ret = element->eraseInternal(m_odd_step? key + 1 : key, m_odd_step? size - 1 : size);
			if (element->size() == 0) {
				delete element;
				m_elements[symbol] = nullptr;
			}
		}
	}
	else {
		ret = radix_element<T>::eraseInternal(key, size);
	}

	if (ret)
		m_size--;

	return ret;
}

// ----------------------------------------------------- Find ------------------------------------
template <typename T>
T& radix_tree<T>::find(const std::string& key) const
{
	const unsigned char *ptr = (const unsigned char *)key.c_str();
	return findInternal(ptr, key.size());
}

template <typename T>
T& radix_element<T>::findInternal(const unsigned char *key, const size_t size) const
{
	if (m_key_size == size) {
		if (memcmp(m_key, key, size) == 0)
			return *m_value;
	}
	return end_element;
}

template <typename T>
T& radix_tree<T>::findInternal(const unsigned char *key, const size_t size) const
{
	if (size) {
		const auto symbol = m_odd_step? key[0] >> 4 : key[0] & radix_element<T>::m_key_mask;
		const auto element = m_elements[symbol];

		if (element) {
			return element->findInternal(m_odd_step? key + 1 : key, m_odd_step? size - 1 : size);
		}
		return radix_element<T>::end();
	}
	return radix_element<T>::findInternal(key, size);
}

// Other key types -----------------------------------------------------------------------------------------

template <typename B, typename T>
class radix_tree_multi : public radix_tree<T>
{
public:

	bool insert(const B& key, const T &value)
	{
		const unsigned char *ptr = (const unsigned char *)&key;
		return radix_tree<T>::insertFirst(ptr, sizeof(B), value);
	}

	bool insert(const std::pair<B, T> &pair)
	{
		const unsigned char *ptr = (const unsigned char *)&pair.first;
		return radix_tree<T>::insertFirst(ptr, sizeof(B), pair.second);
	}

	size_t erase(const B& key)
	{
		const unsigned char *ptr = (const unsigned char *)&key;
		return radix_tree<T>::eraseInternal(ptr, sizeof(B));
	}

	T& find(const B& key) const
	{
		const unsigned char *ptr = (const unsigned char *)&key;
		return radix_tree<T>::findInternal(ptr, sizeof(B));
	}
};

#endif
