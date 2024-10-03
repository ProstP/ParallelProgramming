#include <Windows.h>
#include <bit>
#include <ctime>
#include <fstream>
#include <iostream>
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

using PixelMatrix = std::vector<std::vector<std::vector<std::vector<uint32_t>>>>;

PixelMatrix ReadDataFromFile(int n, std::ifstream& inFile, int width, int height, BMPColorHeader& color)
{
	auto getColor = [](uint32_t& data, uint32_t& mask) {
		uint32_t color = 0;

		color |= (data & mask); // >> std::popcount(mask);

		return static_cast<int>(color);
	};
	std::vector<uint8_t> image(width * height * 4);
	int currentByte = 0;
	inFile.read(reinterpret_cast<char*>(image.data()), image.size());

	int pixelCountWidth = width / n;
	int pixelCountHeight = height / n;
	int remainderWidth = width % n; // остатки обозначают сколько групп будут содержать на пиксель больше, чем указано,
	int remainderHeight = height % n; // это надо чтобы у нас не осталось не прочитанных файлов

	PixelMatrix pixels;
	for (int i = 0; i < n; i++)
	{
		int countInHeight = pixelCountHeight + (i < remainderHeight ? 1 : 0);
		std::vector<std::vector<std::vector<uint32_t>>> row(n);

		for (int y = 0; y < countInHeight; y++)
		{
			for (int j = 0; j < n; j++)
			{
				int countInWidth = pixelCountWidth + (j < remainderWidth ? 1 : 0);
				std::vector<uint32_t> line(countInWidth, 0);

				for (int x = 0; x < countInWidth; x++)
				{
					std::memcpy(&line[x], &image[currentByte], sizeof(uint32_t));
					currentByte += sizeof(uint32_t);

					std::cout << getColor(line[x], color.blueMask) << std::endl;
				}
				row[j].push_back(line);
			}
		}

		pixels.push_back(row);
	}

	return pixels;
}

void WriteDataToFile(const PixelMatrix& pixels, std::ofstream& outFile, int width, int height, int n)
{
	std::vector<uint8_t> image(height * width * 4);
	int currentByte = 0;
	//int padding = (4 - (width * sizeof(uint32_t)) % 4) % 4;

	for (int i = 0; i < n; i++)
	{
		for (int y = 0; y < pixels[i][0].size(); y++)
		{
			std::vector<uint8_t> row;
			for (int j = 0; j < n; j++)
			{
				for (int x = 0; x < pixels[i][j][y].size(); x++)
				{
					std::memcpy(&image[currentByte], &pixels[i][j][y][x], sizeof(uint32_t));
					currentByte += sizeof(uint32_t);
				}
			}
			//currentByte += padding;
		}
	}

	outFile.write(reinterpret_cast<char*>(image.data()), image.size());
}

void Blur(std::vector<uint8_t>& pixels, int startX, int startY, int width, int height)
{
	std::vector<uint8_t> blurred(pixels.size());

	for (int y = startY; y < height - 1; y++)
	{
		for (int x = startX; x < width - 1; x++)
		{
			int sumR = 0, sumG = 0, sumB = 0;
			for (int ky = -1; ky <= 1; ky++)
			{
				for (int kx = -1; kx <= 1; kx++)
				{
					int idx = ((y + ky) * width + (x + kx)) * 3;
					sumB += pixels[idx];
					sumG += pixels[idx + 1];
					sumR += pixels[idx + 2];
				}
			}
			int avgIdx = (y * width + x) * 3;
			blurred[avgIdx] = sumB / 9;
			blurred[avgIdx + 1] = sumG / 9;
			blurred[avgIdx + 2] = sumR / 9;
		}
	}

	pixels.swap(blurred);
}

int main(int argc, char* argv[])
{
	std::cout << "Enter count of threads:" << std::endl;
	int n = 10;
	//std::cin >> n;
	auto start = clock();

	std::ifstream inFile(argv[1], std::ios::binary);
	if (!inFile.is_open())
	{
		std::cout << "Failed to open input file" << std::endl;
		return 1;
	}

	BMPFileHeader bmpFileHeader;
	BMPInfoHeader bmpInfoHeader;
	BMPColorHeader bmpColorHeader;

	inFile.read(reinterpret_cast<char*>(&bmpFileHeader), sizeof(BMPFileHeader));
	inFile.read(reinterpret_cast<char*>(&bmpInfoHeader), sizeof(BMPInfoHeader));
	inFile.read(reinterpret_cast<char*>(&bmpColorHeader), sizeof(BMPColorHeader));

	//std::cout << std::popcount(bmpColorHeader.blueMask) << std::endl;

	if (bmpFileHeader.fileType != 'MB')
	{
		std::cout << "Input file not .bmp" << std::endl;
		return 1;
	}
	if (bmpInfoHeader.bitCount != 32)
	{
		std::cout << "Bit count must be 32" << std::endl;
		return 1;
	}

	auto pixels = ReadDataFromFile(n, inFile, bmpInfoHeader.width, bmpInfoHeader.height, bmpColorHeader);
	inFile.close();

	std::ofstream outFile(argv[2]);

	outFile.write(reinterpret_cast<char*>(&bmpFileHeader), sizeof(BMPFileHeader));
	outFile.write(reinterpret_cast<char*>(&bmpInfoHeader), sizeof(BMPInfoHeader));
	outFile.write(reinterpret_cast<char*>(&bmpColorHeader), sizeof(BMPColorHeader));
	WriteDataToFile(pixels, outFile, bmpInfoHeader.width, bmpInfoHeader.height, n);

	outFile.close();
	std::cout << "Time: " << (clock() - start) / (float)CLOCKS_PER_SEC << " sec" << std::endl;
}
