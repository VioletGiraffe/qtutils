#pragma once
#include "assert/advanced_assert.h"

struct CR {
	inline CR& operator=(bool result) noexcept {
		assert_message_r(result, "connect() failed!");
		return *this;
	}
};
