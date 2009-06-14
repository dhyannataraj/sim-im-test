#ifndef GALAXYSINGLETON_H
#define GALAXYSINGLETON_H

#include <cassert>

namespace SIM
{
template <typename Type> class Singleton
{
	static Type* m_instance;
	Singleton(const Singleton<Type>&) {};
	Singleton<Type>& operator=(const Singleton<Type>&) {};
public:
	Singleton()
	{
		assert(!m_instance);
		m_instance = static_cast<Type*>(this);
	}
	virtual ~Singleton()
	{
		m_instance = 0;
	};
	static Type* instance()
	{
		assert(m_instance);
		return m_instance;
	}

};
template <typename Type> Type* Singleton<Type>::m_instance = 0;
}
#endif
