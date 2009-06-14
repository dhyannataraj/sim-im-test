#ifndef GALAXYSINGLETON_H
#define GALAXYSINGLETON_H

#include <cassert>

namespace SIM
{
template <typename Type> class Singleton
{
	static Type* m_instance;
        // no need to implement them!
	Singleton(const Singleton<Type>&);
	Singleton<Type>& operator=(const Singleton<Type>& t);
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
                //fprintf(stderr, "instance: %p\n", instance);
		assert(m_instance);
		return m_instance;
	}

};
// this can not work. the compiler will generate a static variable
// in every object file which includes this header which results in
// one instance per plugin... see the fprintf() above
//template <typename Type> Type* Singleton<Type>::m_instance = 0;
}
#endif
