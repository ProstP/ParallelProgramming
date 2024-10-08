#include <Windows.h>
#include <bit>
#include <ctime>
#include <fstream>
#include <iostream>
#include <stdint.h>
#include <string>
#include <vector>

#pragma pack(push, 1)

struct BMPFileHeader
{
	uint16_t fileType;
	uint32_t fileSize;
	uint16_t resorved1;
	uint16_t resorved2;
	uint32_t offsetData;
};

struct BMPInfoHeader
{
	uint32_t size;
	int32_t width;
	int32_t height;

	uint16_t planes;
	uint16_t bitCount;
	uint32_t compression;
	uint32_t sizeImage;
	int32_t xPixelsPerMeter;
	int32_t yPixelsPerMeter;
	uint32_t colorsUsed;
	uint32_t colorsImportant;
};

struct BMPColorHeader
{
	uint32_t redMask;
	uint32_t greenMask;
	uint32_t blueMask;
	uint32_t alphaMask;
	uint32_t colorSpaceType;
	uint32_t unused[16];
};

#pragma pack(pop)

struct FileHeader
{
	BMPFileHeader file;
	BMPInfoHeader info;
	BMPColorHeader color;
};

struct Pixel
{
	int r;
	int g;
	int b;
};

FileHeader ReadHeadersFromFile(std::ifstream& inFile)
{
	FileHeader header;

	inFile.read(reinterpret_cast<char*>(&header.file), sizeof(BMPFileHeader));
	inFile.read(reinterpret_cast<char*>(&header.info), sizeof(BMPInfoHeader));
	inFile.read(reinterpret_cast<char*>(&header.color), sizeof(BMPColorHeader));

	return header;
}

void WriteHeadersToFile(FileHeader& headers, std::ofstream& outFile)
{
	outFile.write(reinterpret_cast<char*>(&headers.file), sizeof(BMPFileHeader));
	outFile.write(reinterpret_cast<char*>(&headers.info), sizeof(BMPInfoHeader));
	outFile.write(reinterpret_cast<char*>(&headers.color), sizeof(BMPColorHeader));
}

std::vector<std::vector<Pixel>> CreatePixelImageFromData(int width, int height, const std::vector<uint8_t>& data)
{
	int currentByte = 0;

	std::vector<std::vector<Pixel>> pixels;

	for (int i = 0; i < height; i++)
	{
		std::vector<Pixel> line;
		for (int j = 0; j < width; j++)
		{
			int r = static_cast<int>(data[currentByte + 1]);
			int g = static_cast<int>(data[currentByte + 2]);
			int b = static_cast<int>(data[currentByte + 3]);
			currentByte += 4;

			line.push_back(Pixel{ r, g, b });
		}

		pixels.push_back(line);
	}

	return pixels;
}

void WritePixelToData(const std::vector<std::vector<Pixel>>& pixels, std::vector<uint8_t>& data)
{
	int currentByte = 0;

	for (const auto& line : pixels)
	{
		for (const auto& pixel : line)
		{
			data[currentByte + 1] = static_cast<uint8_t>(pixel.r);
			data[currentByte + 2] = static_cast<uint8_t>(pixel.g);
			data[currentByte + 3] = static_cast<uint8_t>(pixel.b);

			currentByte += 4;
		}
	}
}

struct InfoForThread
{
	int n;
	int startY;
	int endY;
	std::vector<std::vector<Pixel>>* pixels;
};

DWORD WINAPI BlurThread(const LPVOID lpParams)
{
	int count = 0;
	InfoForThread* info = (InfoForThread*)lpParams;

	int widthCount = info->pixels->at(info->startY).size() / info->n;
	int remainderWidth = info->pixels->at(info->startY).size() % info->n;

	int lastEnd = 0;
	for (int i = 0; i < info->n; i++)
	{
		int startX = lastEnd;
		lastEnd = lastEnd + widthCount;
		int endX = lastEnd;

		if (i < remainderWidth)
		{
			lastEnd++;
		}
		else
		{
			endX--;
		}

		for (int y = info->startY; y <= info->endY; y++)
		{
			for (int x = startX; x <= endX; x++)
			{
				int sumR = 0;
				int sumG = 0;
				int sumB = 0;

				for (int dy = y - 1; dy <= y + 1; dy++)
				{
					if (dy < info->startY || info->endY < dy)
					{
						sumR += (3 * info->pixels->at(y)[x].r);
						sumG += (3 * info->pixels->at(y)[x].g);
						sumB += (3 * info->pixels->at(y)[x].b);

						continue;
					}

					for (int dx = x - 1; dx <= x + 1; dx++)
					{
						if (dx < startX || endX < dx)
						{
							sumR += info->pixels->at(y)[x].r;
							sumG += info->pixels->at(y)[x].g;
							sumB += info->pixels->at(y)[x].b;

							continue;
						}

						sumR += info->pixels->at(dy)[dx].r;
						sumG += info->pixels->at(dy)[dx].g;
						sumB += info->pixels->at(dy)[dx].b;
					}
				}

				const float BlurValue = 1.11111;
				sumR *= BlurValue;
				sumG *= BlurValue;
				sumB *= BlurValue;

				info->pixels->at(y)[x].r = sumR / 9;
				info->pixels->at(y)[x].g = sumG / 9;
				info->pixels->at(y)[x].b = sumB / 9;
			}
		}
	}

	ExitThread(0);
}

void BlurInThreads(int n, std::vector<std::vector<Pixel>>& pixels)
{
	int heightCount = pixels.size() / n; // остатки обозначают сколько групп будут содержать на пиксель больше, чем указано,
	int remainderHeight = pixels.size() % n; // это надо чтобы у нас не осталось не прочитанных файлов

	HANDLE* handles = new HANDLE[n];
	InfoForThread* infos = new InfoForThread[n];

	int lastEnd = 0;
	for (int i = 0; i < n; i++)
	{
		InfoForThread info;
		info.n = n;
		info.pixels = &pixels;

		info.startY = lastEnd;
		lastEnd = lastEnd + heightCount;
		info.endY = lastEnd;
		if (i < remainderHeight)
		{
			lastEnd++;
		}
		else
		{
			info.endY--;
		}
		infos[i] = info;
		handles[i] = CreateThread(NULL, 0, &BlurThread, &infos[i], CREATE_SUSPENDED, NULL);
	}

	for (int i = 0; i < n; i++)
	{
		ResumeThread(handles[i]);
	}

	WaitForMultipleObjects(n, handles, true, INFINITE);
}

void BlurImage(int n, const std::string inFileName, const std::string& outFileName)
{
	std::ifstream inFile(inFileName, std::ios::binary);
	if (!inFile.is_open())
	{
		throw std::invalid_argument("Failed to open input file");
	}

	auto headers = ReadHeadersFromFile(inFile);

	if (headers.file.fileType != 'MB')
	{
		throw std::invalid_argument("Input file not .bmp");
	}
	if (headers.info.bitCount != 32)
	{
		throw std::invalid_argument("Bit count must be 32");
	}

	std::vector<uint8_t> data(headers.info.width * headers.info.height * 4);
	inFile.read(reinterpret_cast<char*>(data.data()), data.size());
	inFile.close();

	auto pixels = CreatePixelImageFromData(headers.info.width, headers.info.height, data);

	BlurInThreads(n, pixels);

	WritePixelToData(pixels, data);

	std::ofstream outFile(outFileName);

	WriteHeadersToFile(headers, outFile);
	outFile.write(reinterpret_cast<char*>(data.data()), data.size());

	outFile.close();
}

void Test(int n)
{
	std::vector<std::vector<Pixel>> pixels;

	for (int i = 0; i < 6; i++)
	{
		std::vector<Pixel> line;
		for (int j = 0; j < 8; j++)
		{
			line.push_back(Pixel{ i + j, 2 * (i + j), 3 * (i + j) });
		}
		pixels.push_back(line);
	}

	BlurInThreads(n, pixels);
}

int main(int argc, char* argv[])
{
	std::cout << "Enter count of threads:" << std::endl;
	int n;
	std::cin >> n;
	if (n < 0)
	{
		std::cout << "Num of thread is positive value" << std::endl;
		return 1;
	}
	auto start = clock();

	try
	{
		//Test(n);
		BlurImage(n, argv[1], argv[2]);
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}

	std::cout << "Time: " << (clock() - start) / (float)CLOCKS_PER_SEC << " sec" << std::endl;
}
