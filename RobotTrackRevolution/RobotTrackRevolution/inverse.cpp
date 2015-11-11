float MINOR(float m[16], int r0, int r1, int r2, int c0, int c1, int c2)
{
	return m[4 * r0 + c0] * (m[4 * r1 + c1] * m[4 * r2 + c2] - m[4 * r2 + c1] * m[4 * r1 + c2]) -
		m[4 * r0 + c1] * (m[4 * r1 + c0] * m[4 * r2 + c2] - m[4 * r2 + c0] * m[4 * r1 + c2]) +
		m[4 * r0 + c2] * (m[4 * r1 + c0] * m[4 * r2 + c1] - m[4 * r2 + c0] * m[4 * r1 + c1]);
}


void adjoint(float m[16], float adjOut[16])
{
	adjOut[0] = MINOR(m, 1, 2, 3, 1, 2, 3); adjOut[1] = -MINOR(m, 0, 2, 3, 1, 2, 3); adjOut[2] = MINOR(m, 0, 1, 3, 1, 2, 3); adjOut[3] = -MINOR(m, 0, 1, 2, 1, 2, 3);
	adjOut[4] = -MINOR(m, 1, 2, 3, 0, 2, 3); adjOut[5] = MINOR(m, 0, 2, 3, 0, 2, 3); adjOut[6] = -MINOR(m, 0, 1, 3, 0, 2, 3); adjOut[7] = MINOR(m, 0, 1, 2, 0, 2, 3);
	adjOut[8] = MINOR(m, 1, 2, 3, 0, 1, 3); adjOut[9] = -MINOR(m, 0, 2, 3, 0, 1, 3); adjOut[10] = MINOR(m, 0, 1, 3, 0, 1, 3); adjOut[11] = -MINOR(m, 0, 1, 2, 0, 1, 3);
	adjOut[12] = -MINOR(m, 1, 2, 3, 0, 1, 2); adjOut[13] = MINOR(m, 0, 2, 3, 0, 1, 2); adjOut[14] = -MINOR(m, 0, 1, 3, 0, 1, 2); adjOut[15] = MINOR(m, 0, 1, 2, 0, 1, 2);
}

float det(float m[16])
{
	return m[0] * MINOR(m, 1, 2, 3, 1, 2, 3) -
		m[1] * MINOR(m, 1, 2, 3, 0, 2, 3) +
		m[2] * MINOR(m, 1, 2, 3, 0, 1, 3) -
		m[3] * MINOR(m, 1, 2, 3, 0, 1, 2);
}


void invertRowMajor(float m[16], float invOut[16])
{
	adjoint(m, invOut);

	float inv_det = 1.0f / det(m);
	for (int i = 0; i < 16; ++i)
		invOut[i] = invOut[i] * inv_det;
}

bool invertColumnMajor(double m[16], double invOut[16])
{
	double inv[16], det;
	int i;

	inv[0] = m[5] * m[10] * m[15] - m[5] * m[11] * m[14] - m[9] * m[6] * m[15] + m[9] * m[7] * m[14] + m[13] * m[6] * m[11] - m[13] * m[7] * m[10];
	inv[4] = -m[4] * m[10] * m[15] + m[4] * m[11] * m[14] + m[8] * m[6] * m[15] - m[8] * m[7] * m[14] - m[12] * m[6] * m[11] + m[12] * m[7] * m[10];
	inv[8] = m[4] * m[9] * m[15] - m[4] * m[11] * m[13] - m[8] * m[5] * m[15] + m[8] * m[7] * m[13] + m[12] * m[5] * m[11] - m[12] * m[7] * m[9];
	inv[12] = -m[4] * m[9] * m[14] + m[4] * m[10] * m[13] + m[8] * m[5] * m[14] - m[8] * m[6] * m[13] - m[12] * m[5] * m[10] + m[12] * m[6] * m[9];
	inv[1] = -m[1] * m[10] * m[15] + m[1] * m[11] * m[14] + m[9] * m[2] * m[15] - m[9] * m[3] * m[14] - m[13] * m[2] * m[11] + m[13] * m[3] * m[10];
	inv[5] = m[0] * m[10] * m[15] - m[0] * m[11] * m[14] - m[8] * m[2] * m[15] + m[8] * m[3] * m[14] + m[12] * m[2] * m[11] - m[12] * m[3] * m[10];
	inv[9] = -m[0] * m[9] * m[15] + m[0] * m[11] * m[13] + m[8] * m[1] * m[15] - m[8] * m[3] * m[13] - m[12] * m[1] * m[11] + m[12] * m[3] * m[9];
	inv[13] = m[0] * m[9] * m[14] - m[0] * m[10] * m[13] - m[8] * m[1] * m[14] + m[8] * m[2] * m[13] + m[12] * m[1] * m[10] - m[12] * m[2] * m[9];
	inv[2] = m[1] * m[6] * m[15] - m[1] * m[7] * m[14] - m[5] * m[2] * m[15] + m[5] * m[3] * m[14] + m[13] * m[2] * m[7] - m[13] * m[3] * m[6];
	inv[6] = -m[0] * m[6] * m[15] + m[0] * m[7] * m[14] + m[4] * m[2] * m[15] - m[4] * m[3] * m[14] - m[12] * m[2] * m[7] + m[12] * m[3] * m[6];
	inv[10] = m[0] * m[5] * m[15] - m[0] * m[7] * m[13] - m[4] * m[1] * m[15] + m[4] * m[3] * m[13] + m[12] * m[1] * m[7] - m[12] * m[3] * m[5];
	inv[14] = -m[0] * m[5] * m[14] + m[0] * m[6] * m[13] + m[4] * m[1] * m[14] - m[4] * m[2] * m[13] - m[12] * m[1] * m[6] + m[12] * m[2] * m[5];
	inv[3] = -m[1] * m[6] * m[11] + m[1] * m[7] * m[10] + m[5] * m[2] * m[11] - m[5] * m[3] * m[10] - m[9] * m[2] * m[7] + m[9] * m[3] * m[6];
	inv[7] = m[0] * m[6] * m[11] - m[0] * m[7] * m[10] - m[4] * m[2] * m[11] + m[4] * m[3] * m[10] + m[8] * m[2] * m[7] - m[8] * m[3] * m[6];
	inv[11] = -m[0] * m[5] * m[11] + m[0] * m[7] * m[9] + m[4] * m[1] * m[11] - m[4] * m[3] * m[9] - m[8] * m[1] * m[7] + m[8] * m[3] * m[5];
	inv[15] = m[0] * m[5] * m[10] - m[0] * m[6] * m[9] - m[4] * m[1] * m[10] + m[4] * m[2] * m[9] + m[8] * m[1] * m[6] - m[8] * m[2] * m[5];

	det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

	if (det == 0)
		return false;

	det = 1.f / det;

	for (i = 0; i < 16; i++)
		invOut[i] = inv[i] * det;

	return true;
}


