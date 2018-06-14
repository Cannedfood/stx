// Copyright (c) 2017 Benno Straub, licensed under the MIT license. (A copy can be found at the end of this file)

#pragma once

#include <chrono>

namespace stx {

using namespace std;
using namespace std::chrono;
using namespace std::chrono_literals;

template<class Clock>
class basic_timer {
	using clock          = Clock;
	using time_point     = typename clock::time_point;
	using clock_duration = typename clock::duration;

	time_point m_begin;
public:
	basic_timer() { reset(); }

	void reset() noexcept { m_begin = now(); }

	static inline
	time_point now() noexcept { return clock::now(); }

	template<class Duration = clock_duration>
	Duration time() const noexcept { return duration_cast<Duration>(now() - m_begin); }

	std::string to_string() {
		auto dif = time<>();
		if(dif >= 1min) {
			auto hours = duration_cast<duration<unsigned, ratio<3600>>>(dif);
			dif -= hours;
			auto minutes = duration_cast<duration<unsigned, ratio<60>>>(dif);
			dif -= minutes;
			auto seconds = duration_cast<duration<double>>(dif);
			std::string result;
			if(hours.count() > 0)
				result += std::to_string(hours.count()) + "h ";
			result += std::to_string(minutes.count()) + "min " + std::to_string(seconds.count()) + "s";
			return result;
		}
		else if(dif > 500ms)
			return std::to_string(duration_cast<duration<double, ratio<1>>>(dif).count()) + "s";
		else if(dif > 3ms)
			return std::to_string(duration_cast<duration<double, milli>>(dif).count()) + "ms";
		else if(dif > 3us)
			return std::to_string(duration_cast<duration<double, micro>>(dif).count()) + "us";
		else
			return std::to_string(duration_cast<duration<double, nano>>(dif).count()) + "ns";
	}

	// Get time passe since timer construction or the last timer reset
	double days() const noexcept {
		return time<duration<double, ratio<86400>>>().count();
	}
	double hours() const noexcept {
		return time<duration<double, ratio<3600>>>().count();
	}
	double minutes() const noexcept {
		return time<duration<double, ratio<60>>>().count();
	}
	double seconds() const noexcept {
		return time<duration<double, ratio<1>>>().count();
	}
	double millis() const noexcept {
		return time<duration<double, milli>>().count();
	}
	double micros() const noexcept {
		return time<duration<double, micro>>().count();
	}
	double nanos() const noexcept {
		return time<duration<double, nano>>().count();
	}

	// Get time passed and reset the timer
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

	// Returns whether d time has passed, if it returns true also resets the timer
	// For use in something like void update() { if(mTimer.passed_seconds(1)) update_once_a_seconds(); }
	bool passed_days(double d) noexcept {
		bool b = days() >= d; if(b) reset(); return b;
	}
	bool passed_hours(double d) noexcept {
		bool b = hours() >= d; if(b) reset(); return b;
	}
	bool passed_minutes(double d) noexcept {
		bool b = minutes() >= d; if(b) reset(); return b;
	}
	bool passed_seconds(double d) noexcept {
		bool b = seconds() >= d; if(b) reset(); return b;
	}
	bool passed_millis(double d) noexcept {
		bool b = millis() >= d; if(b) reset(); return b;
	}
	bool passed_micros(double d) noexcept {
		bool b = micros() >= d; if(b) reset(); return b;
	}
	bool passed_nanos(double d) noexcept {
		bool b = nanos() >= d; if(b) reset(); return b;
	}

	// Static functions to get the time since epoch
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

using highres_timer = basic_timer<std::chrono::high_resolution_clock>;
using steady_timer  = basic_timer<std::chrono::steady_clock>;
using timer         = highres_timer;

} // namespace stx

/*
 Copyright (c) 2017 Benno Straub

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
