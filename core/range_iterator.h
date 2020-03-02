/*************************************************************************/
/*  range_iterator.h                                                     */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                    http://www.godotengine.org                         */
/*************************************************************************/
/* Copyright (c) 2007-2016 Juan Linietsky, Ariel Manzur.                 */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/
#ifndef RANGE_ITERATOR_H
#define RANGE_ITERATOR_H

#include "array.h"
#include "reference.h"
#include "variant.h"

class RangeIterator : public Reference {
protected:
	OBJ_TYPE(RangeIterator, Reference);

	static void _bind_methods();

private:
	int current;
	int stop;
	int step;

	_ALWAYS_INLINE_ bool _iter_init(Variant arg) const {
		return !is_finished();
	}

	_ALWAYS_INLINE_ bool _iter_next(Variant arg) {
		current += step;
		return !is_finished();
	}

	_ALWAYS_INLINE_ int _iter_get(Variant arg) const {
		return current;
	}

public:
	_ALWAYS_INLINE_ bool use_native_iterator() const {
		return true;
	}

	_ALWAYS_INLINE_ bool is_finished() const {
		return (step > 0) ? current >= stop : current <= stop;
	}

	_ALWAYS_INLINE_ int get_current() const {
		return current;
	}

	_ALWAYS_INLINE_ int get_stop() const {
		return stop;
	}

	_ALWAYS_INLINE_ int get_step() const {
		return step;
	}

	Array to_array();

	void set_range(int stop);
	void set_range(int start, int stop);
	void set_range(int start, int stop, int step);

	Ref<RangeIterator> _set_range(Variant arg1, Variant arg2 = Variant(), Variant arg3 = Variant());

	void _init(Variant arg1, Variant arg2, Variant arg3);

	RangeIterator();
	RangeIterator(int stop);
	RangeIterator(int start, int stop);
	RangeIterator(int start, int stop, int step);
};

#endif // RANGE_ITERATOR_H
