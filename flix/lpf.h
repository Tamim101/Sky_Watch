


// Low pass filter implementation

#pragma once

template <typename T> // Using template to make the filter usable for scalar and vector values
class LowPassFilter {
public:
	float alpha; // smoothing constant, 1 means filter disabled
	T output;

	LowPassFilter(float alpha): alpha(alpha) {};

	T update(const T input) {
		return output += alpha * (input - output);
	}

	void setCutOffFrequency(float cutOffFreq, float dt) {
		alpha = 1 - exp(-2 * PI * cutOffFreq * dt);
	}

	void reset() {
		output = T(); // set to zero
	}
};
