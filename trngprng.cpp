#include <iostream>
#include <chrono>
#include <vector>
#include <cmath>
#include <random>

using namespace std;
using namespace std::chrono;

vector<int> generateRawBits(int n) {
	vector<int> raw_bits;
	while ((int)raw_bits.size() < n) {
    	auto t1 = high_resolution_clock::now();
    	for (volatile int i = 0; i < 100; i++); //creating jitter
    	auto t2 = high_resolution_clock::now();

    	auto duration = duration_cast<nanoseconds>(t2 - t1).count();
    	int bit = duration & 1; //Extract LSB
    	raw_bits.push_back(bit);
	}
	return raw_bits;
}

vector<int> vonNeumannExtractor(const vector<int>& bits) { // we are doing this to remove bias
	vector<int> extracted;
	for (size_t i = 0; i + 1 < bits.size(); i += 2) {
    	if (bits[i] == 0 && bits[i + 1] == 1) {
        	extracted.push_back(0);
    	}
    	else if (bits[i] == 1 && bits[i + 1] == 0) {
        	extracted.push_back(1);
    	}
    	// discard 00 and 11 pairs
	}
	return extracted;
}

double shannonEntropy(const vector<int>& bits) {
	int count0 = 0, count1 = 0;
	for (int b : bits) {
    	if (b == 0) count0++;
    	else count1++;
	}
	double p0 = (double)count0 / bits.size();
	double p1 = (double)count1 / bits.size();

	auto log2_safe = [](double x) { return x > 0 ? log2(x) : 0; };

	double H = - (p0 * log2_safe(p0) + p1 * log2_safe(p1));
	return H;
}

double chiSquareTest(const vector<int>& bits) {
	int n = bits.size();
	int O0 = 0, O1 = 0;
	for (int b : bits) {
    	if (b == 0) O0++;
    	else O1++;
	}
	double E0 = n / 2.0;
	double E1 = n / 2.0;

	double chi2 = ((O0 - E0) * (O0 - E0)) / E0 + ((O1 - E1) * (O1 - E1)) / E1;
	return chi2;
}


vector<int> generatePRNGBits(int n) {
	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<> dis(0, 1);
	vector<int> bits;
	for (int i = 0; i < n; i++) {
    	bits.push_back(dis(gen));
	}
	return bits;
}

int main() {
	const int raw_bits_needed = 1024;

	// Generate raw TRNG bits
	cout << "Generating raw TRNG bits using timing jitter..." << endl;
	vector<int> raw_bits = generateRawBits(raw_bits_needed);
	cout << "Raw bits generated: " << raw_bits.size() << endl;

	// Apply Von Neumann extractor
	cout << "Applying Von Neumann extractor to remove bias..." << endl;
	vector<int> processed_bits = vonNeumannExtractor(raw_bits);
	cout << "Bits after Von Neumann extraction: " << processed_bits.size() << endl;

	// Calculate statistics for TRNG bitstream
	double entropy_TRNG = shannonEntropy(processed_bits);
	double chi2_TRNG = chiSquareTest(processed_bits);

	cout << "\nTRNG Post-Processed Bitstream Statistics:" << endl;
	cout << "Shannon Entropy: " << entropy_TRNG << " bits per bit" << endl;
	cout << "Chi-Square Statistic: " << chi2_TRNG << endl;

	// Generate PRNG bits of same length as processed TRNG bits
	cout << "\nGenerating PRNG bits for comparison..." << endl;
	vector<int> prng_bits = generatePRNGBits((int)processed_bits.size());

	// Calculate statistics for PRNG bitstream
	double entropy_PRNG = shannonEntropy(prng_bits);
	double chi2_PRNG = chiSquareTest(prng_bits);

	cout << "\nPRNG Bitstream Statistics:" << endl;
	cout << "Shannon Entropy: " << entropy_PRNG << " bits per bit" << endl;
	cout << "Chi-Square Statistic: " << chi2_PRNG << endl;

	// Optional: Compare which bitstream looks more random based on stats
	cout << "\nSummary:" << endl;
	if (entropy_TRNG > entropy_PRNG) {
    	cout << "TRNG bitstream has higher entropy and is more random." << endl;
	} else if (entropy_TRNG < entropy_PRNG) {
    	cout << "PRNG bitstream has higher entropy." << endl;
	} else {
    	cout << "Both bitstreams have similar entropy." << endl;
	}

	if (chi2_TRNG < chi2_PRNG) {
    	cout << "TRNG bitstream has chi-square closer to expected, indicating better balance." << endl;
	} else if (chi2_TRNG > chi2_PRNG) {
    	cout << "PRNG bitstream has better chi-square statistic." << endl;
	} else {
    	cout << "Both bitstreams have similar chi-square statistics." << endl;
	}

	return 0;
}
