#include "win2con.h"

// https://devblogs.microsoft.com/commandline/updating-the-windows-console-colors/
static const uint8_t CMD_COLORS_16[16][3] =
{
	{12,12,12},{0,55,218},{19,161,14},{58,150,221},
	{197,15,31},{136,23,152},{193,156,0},{204,204,204},
	{118,118,118},{59,120,255},{22,198,12},{97,214,214},
	{231,72,86},{180,0,158},{249,241,165},{242,242,242}
};

static void processForWinAPI(Frame* frame);
static uint8_t procColor(uint8_t* r, uint8_t* g, uint8_t* b, int withColors);
static uint8_t findNearestColor16(uint8_t r, uint8_t g, uint8_t b);

void initProcessFrame(void)
{
	return;
}

void refreshScaling(void)
{
	if (conW == 0 || conH == 0 ||
		wndW == 0 || wndH == 0 ||
		fontRatio == 0.0)
	{
		return;
	}

	switch (scalingMode)
	{
	case SM_INT:
		if (conW >= wndW) { scaleXMul = conW / wndW; }
		else { scaleXMul = 1; }

		if (conH >= wndH) { scaleYMul = conH / wndH; }
		else { scaleYMul = 1; }

		if (scaleWithRatio)
		{
			double wndRatio = (double)wndW / (double)wndH;
			double imgRatio = (wndRatio * fontRatio) *
				((double)scaleXMul / (double)scaleYMul);

			if (imgRatio > wndRatio)
			{
				scaleXMul = (int)round((double)scaleXMul * (wndRatio / imgRatio));
				if (scaleXMul < 1) { scaleXMul = 1; }
			}
			else
			{
				scaleYMul = (int)round((double)scaleYMul * (imgRatio / wndRatio));
				if (scaleYMul < 1) { scaleYMul = 1; }
			}
		}

		scaleXDiv = 1;
		scaleYDiv = 1;
		break;

	case SM_INT_FRACTION:
		if (conW >= wndW)
		{
			scaleXMul = conW / wndW;
			scaleXDiv = 1;
		}
		else
		{
			scaleXMul = 1;
			scaleXDiv = (wndW / conW) + 1;
		}

		if (conH >= wndH)
		{
			scaleYMul = conH / wndH;
			scaleYDiv = 1;
		}
		else
		{
			scaleYMul = 1;
			scaleYDiv = (wndH / conH) + 1;
		}
		break;

	case SM_NO_SCALING:
		if (scaleWithRatio)
		{
			if (fontRatio > 1.0)
			{
				scaleXMul = 1;

				scaleYMul = (int)round(fontRatio);
				if (scaleYMul > conH / wndH) { scaleYMul = conH / wndH; }
				if (scaleYMul < 1) { scaleYMul = 1; }
			}
			else
			{
				scaleYMul = 1;

				scaleXMul = (int)round(1.0 / fontRatio);
				if (scaleXMul > conW / wndW) { scaleXMul = conW / wndW; }
				if (scaleXMul < 1) { scaleXMul = 1; }
			}
		}
		else
		{
			scaleXMul = 1;
			scaleYMul = 1;
		}

		scaleXDiv = 1;
		scaleYDiv = 1;
	}
}

void processFrame(Frame* frame)
{
	static int lastW = 0, lastH = 0;
	if (lastW != imgW || lastH != imgH)
	{
		lastW = imgW;
		lastH = imgH;
		if (frame->output) { free(frame->output); }
		if (frame->outputLineOffsets) { free(frame->outputLineOffsets); }
		frame->output = malloc(getOutputArraySize());
		frame->outputLineOffsets = malloc((imgH + 1) * sizeof(int));
	}

	if (colorMode == CM_WINAPI_GRAY || colorMode == CM_WINAPI_16)
	{
		processForWinAPI(frame);
		return;
	}

	uint8_t* output = (uint8_t*)frame->output;
	frame->outputLineOffsets[0] = 0;

	int wndI = wndH - 1, wndIMul = 0;
	for (int i = 0; i < imgH; i++)
	{
		uint8_t oldColor = -1;
		uint8_t oldR = -1, oldG = -1, oldB = -1;
		int isFirstChar = 1;

		int offset = frame->outputLineOffsets[i];

		int wndJ = 0, wndJMul = 0;
		for (int j = 0; j < imgW; j++)
		{
			if (wndJ >= wndW || wndI < 0)
			{
				output[offset] = ' ';
				offset++;
				continue;
			}

			uint8_t valR = frame->bitmapArray[((wndI * wndW) + wndJ) * 4 + 2];
			uint8_t valG = frame->bitmapArray[((wndI * wndW) + wndJ) * 4 + 1];
			uint8_t valB = frame->bitmapArray[((wndI * wndW) + wndJ) * 4];

			uint8_t val, color;

			if (colorMode == CM_CSTD_GRAY) { val = procColor(&valR, &valG, &valB, 0); }
			else { val = procColor(&valR, &valG, &valB, 1); }

			switch (colorMode)
			{
			case CM_CSTD_GRAY:
				output[offset] = charset[(val * charsetSize) / 256];
				offset++;
				break;

			case CM_CSTD_16:
				color = findNearestColor16(valR, valG, valB);
				color = (color & 0b1010) | ((color & 4) >> 2) | ((color & 1) << 2);
				if (color > 7) { color += 82; }
				else { color += 30; }

				if (color == oldColor && !isFirstChar)
				{
					output[offset] = charset[(val * charsetSize) / 256];
					offset++;
					break;
				}
				oldColor = color;

				output[offset] = '\x1B';
				output[offset + 1] = '[';
				output[offset + 2] = (char)(((color / 10) % 10) + 0x30);
				output[offset + 3] = (char)((color % 10) + 0x30);
				output[offset + 4] = 'm';
				output[offset + 5] = charset[(val * charsetSize) / 256];

				offset += 6;
				break;

			case CM_CSTD_256:
				color = rgbToAnsi256(valR, valG, valB);

				if (color == oldColor && !isFirstChar)
				{
					output[offset] = charset[(val * charsetSize) / 256];
					offset++;
					break;
				}
				oldColor = color;

				output[offset] = '\x1B';
				output[offset + 1] = '[';
				output[offset + 2] = '3';
				output[offset + 3] = '8';
				output[offset + 4] = ';';
				output[offset + 5] = '5';
				output[offset + 6] = ';';
				output[offset + 7] = (char)(((color / 100) % 10) + 0x30);
				output[offset + 8] = (char)(((color / 10) % 10) + 0x30);
				output[offset + 9] = (char)((color % 10) + 0x30);
				output[offset + 10] = 'm';
				output[offset + 11] = charset[(val * charsetSize) / 256];

				offset += 12;
				break;

			case CM_CSTD_RGB:
				if (valR == oldR && valG == oldG && valB == oldB && !isFirstChar)
				{
					output[offset] = charset[(val * charsetSize) / 256];
					offset++;
					break;
				}
				oldR = valR;
				oldG = valG;
				oldB = valB;

				output[offset] = '\x1B';
				output[offset + 1] = '[';
				output[offset + 2] = '3';
				output[offset + 3] = '8';
				output[offset + 4] = ';';
				output[offset + 5] = '2';
				output[offset + 6] = ';';
				output[offset + 7] = (char)(((valR / 100) % 10) + 0x30);
				output[offset + 8] = (char)(((valR / 10) % 10) + 0x30);
				output[offset + 9] = (char)((valR % 10) + 0x30);
				output[offset + 10] = ';';
				output[offset + 11] = (char)(((valG / 100) % 10) + 0x30);
				output[offset + 12] = (char)(((valG / 10) % 10) + 0x30);
				output[offset + 13] = (char)((valG % 10) + 0x30);
				output[offset + 14] = ';';
				output[offset + 15] = (char)(((valB / 100) % 10) + 0x30);
				output[offset + 16] = (char)(((valB / 10) % 10) + 0x30);
				output[offset + 17] = (char)((valB % 10) + 0x30);
				output[offset + 18] = 'm';
				output[offset + 19] = charset[(val * charsetSize) / 256];

				offset += 20;
				break;
			}

			isFirstChar = 0;

			if (scalingMode == SM_FILL)
			{
				wndJ = j * wndW / imgW;
			}
			else
			{
				wndJMul++;
				if (wndJMul == scaleXMul)
				{
					wndJ += scaleXDiv;
					wndJMul = 0;
				}
			}
		}

		output[offset] = '\n';
		frame->outputLineOffsets[i + 1] = offset + 1;

		if (scalingMode == SM_FILL)
		{
			wndI = wndH - (i * wndH / imgH) - 1;
		}
		else
		{
			wndIMul++;
			if (wndIMul == scaleYMul)
			{
				wndI -= scaleYDiv;
				wndIMul = 0;
			}
		}
	}

	if (!disableCLS)
	{
		output[frame->outputLineOffsets[imgH] - 1] = '\0';
		frame->outputLineOffsets[imgH]--;
	}
}

static void processForWinAPI(Frame* frame)
{
	#ifdef _WIN32
	CHAR_INFO* output = (CHAR_INFO*)frame->output;

	int wndI = wndH - 1, wndIMul = 0;
	for (int i = 0; i < imgH; i++)
	{
		int wndJ = 0, wndJMul = 0;
		for (int j = 0; j < imgW; j++)
		{
			if (j >= wndW || i >= wndH)
			{
				output[(i * imgW) + j].Char.AsciiChar = ' ';
				output[(i * imgW) + j].Attributes = 0;
				continue;
			}

			uint8_t valR = frame->bitmapArray[((wndI * wndW) + wndJ) * 4 + 2];
			uint8_t valG = frame->bitmapArray[((wndI * wndW) + wndJ) * 4 + 1];
			uint8_t valB = frame->bitmapArray[((wndI * wndW) + wndJ) * 4];

			uint8_t val;

			if (colorMode == CM_WINAPI_16)
			{
				val = procColor(&valR, &valG, &valB, 1);
				output[(i * imgW) + j].Attributes = findNearestColor16(valR, valG, valB);
			}
			else
			{
				val = procColor(&valR, &valG, &valB, 0);
				output[(i * imgW) + j].Attributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
			}

			output[(i * imgW) + j].Char.AsciiChar = charset[(val * charsetSize) / 256];

			if (scalingMode == SM_FILL)
			{
				wndJ = j * wndW / imgW;
			}
			else
			{
				wndJMul++;
				if (wndJMul == scaleXMul)
				{
					wndJ += scaleXDiv;
					wndJMul = 0;
				}
			}
		}

		if (scalingMode == SM_FILL)
		{
			wndI = wndH - (i * wndH / imgH) - 1;
		}
		else
		{
			wndIMul++;
			if (wndIMul == scaleYMul)
			{
				wndI -= scaleYDiv;
				wndIMul = 0;
			}
		}
	}
	#endif
}

static uint8_t procColor(uint8_t* r, uint8_t* g, uint8_t* b, int withColors)
{
	uint8_t valR = *r, valG = *g, valB = *b;

	if (withColors)
	{
		if (valR >= valG && valR >= valB)
		{
			*r = 255;
			*g = (uint8_t)(255.0f * ((float)valG / (float)valR));
			*b = (uint8_t)(255.0f * ((float)valB / (float)valR));
		}
		else if (valG >= valR && valG >= valB)
		{
			*r = (uint8_t)(255.0f * ((float)valR / (float)valG));
			*g = 255;
			*b = (uint8_t)(255.0f * ((float)valB / (float)valG));
		}
		else
		{
			*r = (uint8_t)(255.0f * ((float)valR / (float)valB));
			*g = (uint8_t)(255.0f * ((float)valG / (float)valB));
			*b = 255;
		}
	}

	return (uint8_t)((double)valR * 0.299 + (double)valG * 0.587 + (double)valB * 0.114);
}

static uint8_t findNearestColor16(uint8_t r, uint8_t g, uint8_t b)
{
	int min = INT_MAX;
	int minPos = 0;
	for (int i = 0; i < 16; i++)
	{
		int diff = (r - CMD_COLORS_16[i][0]) * (r - CMD_COLORS_16[i][0]) +
			(g - CMD_COLORS_16[i][1]) * (g - CMD_COLORS_16[i][1]) +
			(b - CMD_COLORS_16[i][2]) * (b - CMD_COLORS_16[i][2]);
		if (diff < min)
		{
			min = diff;
			minPos = i;
		}
	}
	return (uint8_t)minPos;
}