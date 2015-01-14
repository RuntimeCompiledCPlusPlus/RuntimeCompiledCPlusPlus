//
// Copyright (c) 2010-2011 Matthew Jack and Doug Binks
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#pragma once
#ifndef AUARRAY_DEFINED
#define AUARRAY_DEFINED

#include <stdlib.h>
#include <vector>


// Here we define a semi-virtualised wrapper for a vector-like object so that it can
// be used to pass results across DLL boundaries including resizing without memory
// manager problems.
// Ideally it should not use std::vector as this may not work if one side was compiled
// using debug and the other optimised.

template <class T> class IAUDynArray
{
protected:
	IAUDynArray<T>() {}
	~IAUDynArray<T>() {}

public:
	virtual void Resize(size_t size) = 0;

	virtual void Add(const T& item) = 0;

	size_t Size() const
	{
		return m_vec.size();
	}

	void Clear()
	{
		Resize(0);
	}

	T& operator[] (size_t i)
	{
		return m_vec[i];
	}

	const T& operator[] (size_t i) const
	{
		return m_vec[i];
	}

protected:
	std::vector<T> m_vec;
};


template <class T> class AUDynArray : public IAUDynArray<T>
{
public:
	AUDynArray<T>(size_t size = 0)
	{
		this->m_vec.resize(size);
	}

	~AUDynArray<T>()
	{
		// Ensure this code is created, despite the templates
		Resize(0);
	}

	void Resize(size_t size) 
	{
		this->m_vec.resize(size);
	}

	void Add(const T& item)
	{
		this->m_vec.push_back(item);
	}
};

#endif //AUARRAY_DEFINED