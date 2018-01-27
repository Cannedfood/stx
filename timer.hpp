// Copyright (c) 2017 Benno Straub, licensed under the MIT license. (A copy can be found at the end of this file)

#pragma once

#include <chrono>

namespace stx {

using namespace std;
using namespace std::chrono;
using namespace std::chrono_literals;

class timer {
	high_resolution_clock::time_point m_begin;

	static inline
	high_resolution_clock::time_point now() noexcept { return high_resolution_clock::now(); }

public:
	timer() { reset(); }

	void reset() noexcept { m_begin = now(); }

	double days() const noexcept {
		return duration_cast<duration<double, ratio<86400>>>(now() - m_begin).count();
	}
	double hours() const noexcept {
		return duration_cast<duration<double, ratio<3600>>>(now() - m_begin).count();
	}
	double minutes() const noexcept {
		return duration_cast<duration<double, ratio<60>>>(now() - m_begin).count();
	}
	double seconds() const noexcept {
		return duration_cast<duration<double, ratio<1>>>(now() - m_begin).count();
	}
	double millis() const noexcept {
		return duration_cast<duration<double, milli>>(now() - m_begin).count();
	}
	double micros() const noexcept {
		return duration_cast<duration<double, micro>>(now() - m_begin).count();
	}
	double nanos() const noexcept {
		return duration_cast<duration<double, nano>>(now() - m_begin).count();
	}

	double poll_days() noexcept {
		double result = days(); reset(); return result;
	}
	double poll_hours() noexcept {
		double result = hours(); reset(); return result;
	}
	double poll_minutes() noexcept {
		double result = minutes(); reset(); return result;
	}
	double poll_seconds() noexcept {
		double result = seconds(); reset(); return result;
	}
	double poll_millis() noexcept {
		double result = millis(); reset(); return result;
	}
	double poll_micros() noexcept {
		double result = micros(); reset(); return result;
	}
	double poll_nanos() noexcept {
		double result = nanos(); reset(); return result;
	}

	static double days_now() noexcept {
		return duration_cast<duration<double, ratio<86400>>>(now().time_since_epoch()).count();
	}
	static double hours_now() noexcept {
		return duration_cast<duration<double, ratio<3600>>>(now().time_since_epoch()).count();
	}
	static double minutes_now() noexcept {
		return duration_cast<duration<double, ratio<60>>>(now().time_since_epoch()).count();
	}
	static double seconds_now() noexcept {
		return duration_cast<duration<double, ratio<1>>>(now().time_since_epoch()).count();
	}
	static double millis_now() noexcept {
		return duration_cast<duration<double, milli>>(now().time_since_epoch()).count();
	}
	static double micros_now() noexcept {
		return duration_cast<duration<double, micro>>(now().time_since_epoch()).count();
	}
	static double nanos_now() noexcept {
		return duration_cast<duration<double, nano>>(now().time_since_epoch()).count();
	}
};

} // namespace stx

/*
 Copyright (c) 2017 Benno Straub

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
