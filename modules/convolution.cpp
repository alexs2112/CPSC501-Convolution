/* Using a slightly modified version of the input-side algorithm presented in lecture */
void linear_convolution(float* x, int N, float* h, int M, float* y, int P) {
    int n, m;

	// Clear output buffer y[]
	for (n = 0; n < P; n++) {
		y[n] = 0.0;
	}

	// Outer Loop: Process each input value x[n] in turn
	for (n = 0; n < N; n++) {
		// Inner Loop: process x[n] with each sample of h[n]
		for (m = 0; m < M; m++) {
			y[n+m] += (float)(x[n] * h[m]);
		}
	}
}
