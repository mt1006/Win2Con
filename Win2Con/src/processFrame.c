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
static uint8_t procColor(uint8_t* r, uint8_t* g, uint8_t* b, bool withColors);
static void procRand(uint8_t* val);
static uint8_t findNearestColor16(uint8_t r, uint8_t g, uint8_t b);

void refreshScaling(void)
{
	if (conW == 0 || conH == 0 ||
		wndW == 0 || wndH == 0 ||
		fontRatio == 0.0)
	{
		return;
	}

	switch (settings.scalingMode)
	{
	case SM_NO_SCALING:
		if (settings.scaleWithRatio)
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
		break;

	case SM_SOFT_FILL:
		scaleXMul = 1;
		scaleYMul = 1;
		scaleXDiv = 1;
		scaleYDiv = 1;
		break;
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

	if (settings.colorMode == CM_WINAPI_GRAY ||
		settings.colorMode == CM_WINAPI_16)
	{
		processForWinAPI(frame);
		return;
	}

	uint8_t* output = (uint8_t*)frame->output;
	frame->outputLineOffsets[0] = 0;

	int bmpW, bmpH;
	if (settings.scalingMode == SM_SOFT_FILL)
	{
		bmpW = conW;
		bmpH = conH;
	}
	else
	{
		bmpW = wndW;
		bmpH = wndH;
	}

	int bmpI = bmpH - 1, bmpIMul = 0;
	for (int i = 0; i < imgH; i++)
	{
		uint8_t oldColor = -1;
		uint8_t oldR = -1, oldG = -1, oldB = -1;
		bool isFirstChar = true;

		int offset = frame->outputLineOffsets[i];

		int bmpJ = 0, bmpJMul = 0;
		for (int j = 0; j < imgW; j++)
		{
			if (bmpJ >= bmpW || bmpI < 0)
			{
				output[offset] = ' ';
				offset++;
				continue;
			}

			uint8_t valR = frame->bitmapArray[((bmpI * bmpW) + bmpJ) * 4 + 2];
			uint8_t valG = frame->bitmapArray[((bmpI * bmpW) + bmpJ) * 4 + 1];
			uint8_t valB = frame->bitmapArray[((bmpI * bmpW) + bmpJ) * 4];

			uint8_t val, color;

			if (settings.colorMode == CM_CSTD_GRAY)
			{
				val = procColor(&valR, &valG, &valB, 0);
			}
			else
			{
				if (settings.colorProcMode == CPM_NONE) { val = 255; }
				else { val = procColor(&valR, &valG, &valB, 1); }
			}

			if (settings.brightnessRand) { procRand(&val); }

			switch (settings.colorMode)
			{
			case CM_CSTD_GRAY:
				output[offset] = settings.charset[(val * settings.charsetSize) / 256];
				offset++;
				break;

			case CM_CSTD_16:
				color = findNearestColor16(valR, valG, valB);
				color = (color & 0b1010) | ((color & 4) >> 2) | ((color & 1) << 2);
				if (color > 7) { color += 82; }
				else { color += 30; }

				if (color == oldColor && !isFirstChar)
				{
					output[offset] = settings.charset[(val * settings.charsetSize) / 256];
					offset++;
					break;
				}
				oldColor = color;

				output[offset] = '\x1B';
				output[offset + 1] = '[';
				output[offset + 2] = (char)(((color / 10) % 10) + 0x30);
				output[offset + 3] = (char)((color % 10) + 0x30);
				output[offset + 4] = 'm';
				output[offset + 5] = settings.charset[(val * settings.charsetSize) / 256];

				offset += 6;
				break;

			case CM_CSTD_256:
				color = rgbToAnsi256(valR, valG, valB);

				if (color == oldColor && !isFirstChar)
				{
					output[offset] = settings.charset[(val * settings.charsetSize) / 256];
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
				output[offset + 11] = settings.charset[(val * settings.charsetSize) / 256];

				offset += 12;
				break;

			case CM_CSTD_RGB:
				if (valR == oldR && valG == oldG && valB == oldB && !isFirstChar)
				{
					output[offset] = settings.charset[(val * settings.charsetSize) / 256];
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
				output[offset + 19] = settings.charset[(val * settings.charsetSize) / 256];

				offset += 20;
				break;
			}

			isFirstChar = false;

			if (settings.scalingMode == SM_FILL)
			{
				bmpJ = j * bmpW / imgW;
			}
			else
			{
				bmpJMul++;
				if (bmpJMul == scaleXMul)
				{
					bmpJ += scaleXDiv;
					bmpJMul = 0;
				}
			}
		}

		output[offset] = '\n';
		frame->outputLineOffsets[i + 1] = offset + 1;

		if (settings.scalingMode == SM_FILL)
		{
			bmpI = bmpH - (i * bmpH / imgH) - 1;
		}
		else
		{
			bmpIMul++;
			if (bmpIMul == scaleYMul)
			{
				bmpI -= scaleYDiv;
				bmpIMul = 0;
			}
		}
	}

	if (!settings.disableCLS)
	{
		output[frame->outputLineOffsets[imgH] - 1] = '\0';
		frame->outputLineOffsets[imgH]--;
	}
}

static void processForWinAPI(Frame* frame)
{
	CHAR_INFO* output = (CHAR_INFO*)frame->output;

	int bmpW, bmpH;
	if (settings.scalingMode == SM_SOFT_FILL)
	{
		bmpW = conW;
		bmpH = conH;
	}
	else
	{
		bmpW = wndW;
		bmpH = wndH;
	}

	int bmpI = bmpH - 1, bmpIMul = 0;
	for (int i = 0; i < imgH; i++)
	{
		int bmpJ = 0, bmpJMul = 0;
		for (int j = 0; j < imgW; j++)
		{
			if (j >= bmpW || i >= bmpH)
			{
				output[(i * imgW) + j].Char.AsciiChar = ' ';
				output[(i * imgW) + j].Attributes = 0;
				continue;
			}

			uint8_t valR = frame->bitmapArray[((bmpI * bmpW) + bmpJ) * 4 + 2];
			uint8_t valG = frame->bitmapArray[((bmpI * bmpW) + bmpJ) * 4 + 1];
			uint8_t valB = frame->bitmapArray[((bmpI * bmpW) + bmpJ) * 4];

			uint8_t val;

			if (settings.colorMode == CM_WINAPI_16)
			{
				if (settings.colorProcMode == CPM_NONE) { val = 255; }
				else { val = procColor(&valR, &valG, &valB, true); }

				output[(i * imgW) + j].Attributes = findNearestColor16(valR, valG, valB);
			}
			else
			{
				val = procColor(&valR, &valG, &valB, false);

				if (settings.setColorMode == SCM_WINAPI)
				{
					output[(i * imgW) + j].Attributes = settings.setColorVal;
				}
				else
				{
					output[(i * imgW) + j].Attributes =
						FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
				}
			}

			if (settings.brightnessRand) { procRand(&val); }

			output[(i * imgW) + j].Char.AsciiChar =
				settings.charset[(val * settings.charsetSize) / 256];

			if (settings.scalingMode == SM_FILL)
			{
				bmpJ = j * bmpW / imgW;
			}
			else
			{
				bmpJMul++;
				if (bmpJMul == scaleXMul)
				{
					bmpJ += scaleXDiv;
					bmpJMul = 0;
				}
			}
		}

		if (settings.scalingMode == SM_FILL)
		{
			bmpI = bmpH - (i * bmpH / imgH) - 1;
		}
		else
		{
			bmpIMul++;
			if (bmpIMul == scaleYMul)
			{
				bmpI -= scaleYDiv;
				bmpIMul = 0;
			}
		}
	}
}

static uint8_t procColor(uint8_t* r, uint8_t* g, uint8_t* b, bool withColors)
{
	uint8_t valR = *r, valG = *g, valB = *b;
	uint8_t retVal = (uint8_t)((double)valR * 0.299 + (double)valG * 0.587 + (double)valB * 0.114);

	if (withColors && settings.colorProcMode == CPM_BOTH)
	{
		if (!valR) { valR = 1; }
		if (!valG) { valG = 1; }
		if (!valB) { valB = 1; }

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

	return retVal;
}

static void procRand(uint8_t* val)
{
	if (settings.colorProcMode == CPM_NONE)
	{
		*val -= rand() % (settings.brightnessRand + 1);
	}
	else
	{
		if (settings.brightnessRand > 0)
		{
			int tempVal = (int)(*val) +
				(rand() % (settings.brightnessRand + 1)) -
				(settings.brightnessRand / 2);

			if (tempVal >= 255) { *val = 255; }
			else if (tempVal <= 0) { *val = 0; }
			else { *val = (uint8_t)tempVal; }
		}
		else
		{
			int tempVal = (int)(*val) -
				(rand() % (-settings.brightnessRand + 1));

			if (tempVal <= 0) { *val = 0; }
			else { *val = (uint8_t)tempVal; }
		}
	}
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