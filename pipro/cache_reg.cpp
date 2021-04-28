#include <iostream>
#include <fstream>
#include <cassert>
#include <iterator>
#include <set>
#include <cstring>

typedef unsigned int uint;
typedef unsigned short int usint;

int log2(uint x)
{
    int r = 0;
    while(x > 0)
    {
        x = x >> 1;
        r++;
    }

    return r - 1;
}

int pow2(uint n)
{
    int r = 1;
    while(n > 0)
    {
        r = r << 1;
        n--;
    }

    return r;
}
class Cache
{
	uint cacheSize = 256;
	uint blockSize = 4;
	std::fstream srcFile;
	std::vector<uint> sets;
	int num_of_reads = 0;
	int num_of_writes = 0;
	//Little Endian
public:
	Cache(std::fstream fp);
	~Cache();
	uint readBlock(usint address);
	usint readByte(usint address);
	void writeBlock(usint address, uint data);
	void writeByte(usint address, uint data);
	void resetAccesses();
	bool readBusy();
	bool writeBusy();
	void updateSrcFile();
};
Cache::Cache(std::fstream fp){
	this->srcFile = fp;
	sets = vector<uint> (cacheSize,0);
	std::string hexCode;
	uint value;
	int blockNum = 0;
	int offset = 0;
	while(fp >> hexCode){
		if(offset == 4)
		{
			offset = 0;
			blockNum++;
		}
		value = std::stoi(hexCode,0,16);
		for(int i = 0; i < offset; i++)
		{
			value << 8;
		}
		sets[blockNum] += value;
		offset++;
	}
}
uint Cache::readBlock(usint address){
	num_of_reads++;
	return sets[address >> 2];
}
usint Cache::readByte(usint address){
	uint data = readBlock(address);
	uint offset = address & (blockSize-1);
	for(int i = offset; i < 3; i++)
	{
		data << 8;
	}
	for(int i = 0; i < 3; i++)
	{
		data >> 8;
	}
	return (usint)data;
}
void Cache::writeBlock(usint address, uint data){
	sets[address >> log2(blockSize)] = data;
}
void Cache::writeByte(usint address, uint data){
	num_of_writes++;
	uint blockNum = address >> 2;
	uint offset = address & (blockSize-1);
	uint mask = cacheSize-1;
	for(int i = 0; i < offset; i++)
	{
		mask << 8;
		data << 8;
	}
	mask = UINT_MAX - mask;
	sets[blockNum] = (sets[blockNum] & mask) + data;
}
void Cache::resetAccesses(){
	num_of_reads = 0;
	num_of_writes = 0;
}
 bool Cache::readBusy(){
 	if(num_of_reads == 1)
 	{
 		return true;
 	}
 	return false;
 }
 bool Cache::writeBusy(){
 	if(num_of_writes == 1)
 	{
 		return true;
 	}
 	return false;
 }






class RegFile
{
	int regSize = 16;
	std::fstream srcFile;
	std::vector<usint> RF;
	int num_of_reads = 0;
	int num_of_writes = 0;
public:
	RegFile(std::fstream fp);
	~RegFile();
	usint read(usint index);
	void write(usint index, usint data);
	void updateSrcFile();
	void resetAccesses();
	bool readBusy();
	bool writeBusy();
};
RegFile::RegFile(std::fstream fp){
	this->srcFile = fp;
	RF = vector<uint> (regSize);
	std::string hexCode;
	usint value;
	int regNum = 0;
	while(fp >> hexCode){
		value = std::stoi(hexCode,0,16);
		RF[regNum] = value;
		regNum++;
	}
}
usint RegFile::read(usint index){
	num_of_reads++;
	return RF[index];
}
void RegFile::write(usint index,usint data){
	num_of_writes++;
	RF[index] = data;
}
void RegFile::resetAccesses(){
	num_of_reads = 0;
	num_of_writes = 0;
}
 bool RegFile::readBusy(){
 	if(num_of_reads == 2)
 	{
 		return true;
 	}
 	return false;
 }
 bool RegFile::writeBusy(){
 	if(num_of_writes == 1)
 	{
 		return true;
 	}
 	return false;
 }
