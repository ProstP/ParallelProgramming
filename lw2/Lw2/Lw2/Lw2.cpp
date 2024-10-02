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

struct Pixel
{
	uint32_t data;
	int r;
	int g;
	int b;
};

using PixelMatrix = std::vector<std::vector<std::vector<std::vector<uint32_t>>>>;

PixelMatrix ReadDataFromFile(int n, std::ifstream& inFile, const BMPFileHeader& bmpFileHeader, const BMPInfoHeader& bmpInfoHeader, const BMPColorHeader& bmpColorHeader)
{
	int pixelCountWidth = bmpInfoHeader.width / (n * n);
	int pixelCountHeight = bmpInfoHeader.height / (n * n);
	int remainderWidth = bmpInfoHeader.width % (n * n); // остатки обозначают сколько групп будут содержать на пиксель больше, чем указано,
	int remainderHeight = bmpInfoHeader.height % (n * n); // это надо чтобы у нас не осталось не прочитанных файлов

	PixelMatrix pixels;
	for (int i = 0; i < n; i++)
	{
		int countInHeight = pixelCountHeight + (i < remainderHeight ? 1 : 0);
		std::vector<std::vector<std::vector<uint32_t>>> row(n, std::vector<std::vector<uint32_t>>(countInHeight));

		for (int y = 0; y < countInHeight; y++)
		{
			for (int j = 0; j < n; j++)
			{
				std::vector<uint32_t> line;
				int countInWidth = pixelCountWidth + (j < remainderWidth ? 1 : 0);
				std::vector<uint8_t> lineInByte(countInWidth * 4);

				inFile.read(reinterpret_cast<char*>(lineInByte.data()), lineInByte.size());

				for (int x = 0; x < lineInByte.size(); x += 4)
				{
					//uint32_t data;
					uint32_t pixel;
					std::memcpy(&pixel, &lineInByte[x], sizeof(uint32_t));
					//Pixel pixel;
					//pixel.data = data;
					line.push_back(pixel);
				}
				row[j][y] = line;
			}
		}

		pixels.push_back(row);
	}

	return pixels;
}

void WriteDataToFile(PixelMatrix pixels, std::ofstream& outFile)
{
	for (int i = 0; i < pixels.size(); i++)
	{
		for (int y = 0; y < pixels[i][0].size(); y++)
		{
			for (int j = 0; j < pixels[i].size(); j++)
			{
				for (int x = 0; x < pixels[i][j][y].size(); x++)
				{
					uint32_t data = pixels[i][j][y][x];

					std::vector<uint8_t> dataInByte(4);

					std::memcpy(dataInByte.data(), &data, sizeof(uint32_t));
					outFile.write(reinterpret_cast<char*>(dataInByte.data()), dataInByte.size());
				}
			}
		}
	}
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

	auto pixels = ReadDataFromFile(n, inFile, bmpFileHeader, bmpInfoHeader, bmpColorHeader);

	inFile.close();

	std::ofstream outFile(argv[2]);

	outFile.write(reinterpret_cast<char*>(&bmpFileHeader), sizeof(BMPFileHeader));
	outFile.write(reinterpret_cast<char*>(&bmpInfoHeader), sizeof(BMPInfoHeader));
	outFile.write(reinterpret_cast<char*>(&bmpColorHeader), sizeof(BMPColorHeader));
	WriteDataToFile(pixels, outFile);

	outFile.close();
	std::cout << "Time: " << (clock() - start) / (float)CLOCKS_PER_SEC << " sec" << std::endl;
}
